#pragma once
#include <utility>
#include "caf/all.hpp"
#include "caf/response_promise.hpp"

#include "resource_manager/registering_service.hpp"
#include "core/logger.hpp"
#include "core/lfge_errors.hpp"

namespace lfge::resource_manager
{

    namespace task_distribution
    {

        template< typename OutputType, typename ... InputType >
        using typed_simple_task_distributor = caf::typed_actor<
            caf::result<OutputType>( InputType ... )
        >;

        template< typename OutputType, typename ... InputType >
        using typed_simple_task_runner = caf::typed_actor<
            caf::result<OutputType>( InputType ... )
        >;

        template< typename OutputType, typename ... InputType >
        class simple_distributor : public typed_simple_task_distributor<OutputType, InputType ...>::base
        {
            //Actor which task distributor will reply after treating the task
            std::string serviceName;
            lfge::resource_manager::typed_registration_actor registrationActor;
            bool retryOnFailure;
            int maxNumberOfRetries;
            public:

            using super = typename typed_simple_task_distributor<OutputType, InputType ...>::base;

            simple_distributor(  caf::actor_config& config, 
                                    const std::string& serviceName, 
                                    lfge::resource_manager::typed_registration_actor registrationActor, 
                                    bool retryOnFailure = false, 
                                    const int& maxNumberOfRetries = 3 ):
                super(config), 
                serviceName(serviceName), 
                registrationActor(registrationActor), 
                retryOnFailure(retryOnFailure), 
                maxNumberOfRetries(maxNumberOfRetries)
            {

            }

            caf::result<OutputType> distribute( const int& retryCount, InputType ... inputs )
            {
                auto rp = this->template make_response_promise< OutputType >();

                this->request( registrationActor, caf::infinite, lfge::core::find_service_v, serviceName ).then(
                        [this, rp, intp =  std::make_tuple( inputs ... ) ]( std::string name, caf::actor_addr addr ) mutable
                        {
                            std::apply( 
                                [this, rp, addr]( InputType ... inputs ) mutable
                                {
                                    auto casted_addr = caf::actor_cast< typed_simple_task_runner<OutputType, InputType ... > >(addr);
                                    this->request(casted_addr, caf::infinite, inputs ... ).then( 
                                        [rp](OutputType output) mutable
                                        {
                                            rp.deliver(output);
                                        },
                                        [rp]( const caf::error e ) mutable
                                        {
                                            rp.deliver( std::move(e) );
                                        }
                                    );
                                }
                                ,std::move(intp)
                            );
                            
                        },
                        [this, rp, retryCount, intp =  std::make_tuple( inputs ... )](caf::error e) mutable
                        {
                            //Retry launch
                            if(retryOnFailure && retryCount < maxNumberOfRetries)
                            {
                                lfge::core::logger::log( lfge::core::loglevel::error, " simple_distributor could not send the request with error " + to_string(e) + " will retry " );
                            
                                std::apply(
                                    [this, retryCount]( InputType ... inputs ) mutable
                                    {
                                        return distribute( retryCount+1,  inputs ... );
                                    },
                                    std::move(intp)
                                );
                            }
                            else
                            {
                                lfge::core::logger::log( lfge::core::loglevel::error, " simple_distributor could not send the request with error " + to_string(e) + " will NOT retry " );
                            
                                rp.deliver( lfge_errors::retry_limit_surpassed );
                            }
                            
                        }
                    );

                return rp;
            }

            virtual typename super::behavior_type make_behavior() override
            {
                return 
                {
                    [this]( InputType ... inputs ) -> caf::result<OutputType>
                    {
                        return distribute( 0, inputs ... );
                    }
                };
            }
        };
    }
}