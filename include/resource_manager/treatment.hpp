#pragma once
#include <utility>
#include "caf/all.hpp"
#include "caf/response_promise.hpp"

#include "resource_manager/registering_service.hpp"
#include "core/logger.hpp"

namespace lfge::resource_manager
{

    template< typename OutputType, typename ... InputType >
    using typed_simple_task_distributor = caf::typed_actor<
        caf::result<OutputType>( InputType ... )
    >;

    template< typename OutputType, typename ... InputType >
    using typed_simple_task_runner = caf::typed_actor<
        caf::result<OutputType>( InputType ... )
    >;

    using simple_int_replyer = typed_simple_task_distributor<int, std::string>;

    typename simple_int_replyer::behavior_type SimpleTaskDistributor( typename simple_int_replyer::pointer self , const std::string& serviceName, typed_registration_actor registrationActor)
    {
        return 
            {
                [=]( std::string inputs ) -> caf::result<int>
                {
                    auto rp = self->make_response_promise< int >();

                    self->request( registrationActor, caf::infinite, lfge::core::find_service_v, serviceName ).then(
                            [=, intp =  std::make_tuple( inputs ) ]( std::string name, caf::actor_addr addr ) mutable
                            {
                                std::apply( 
                                    [=]( std::string inputs ) mutable
                                    {
                                        auto casted_addr = caf::actor_cast< simple_int_replyer >(addr);
                                        self->request(casted_addr, caf::infinite, inputs).then( 
                                            [=](int output) mutable
                                            {
                                                rp.deliver(output);
                                            },
                                            [=]( const caf::error e ) mutable
                                            {
                                                rp.deliver( std::move(e) );
                                            }
                                        );
                                    }
                                    ,std::move(intp)
                                );
                                
                            },
                            [self, serviceName](caf::error e)
                            {
                                caf::aout(self) << " SimpleTaskDistributor failed to find the service " << serviceName << " error " << to_string(e) << std::endl;
                            }
                        );

                    return rp;
                }
            };
    }

    /*template< typename OutputType, typename ... InputType >
    class SimpleTaskDistributor : public typed_simple_task_distributor<OutputType, InputType ...>
    {
        //Actor which task distributor will reply after treating the task
        std::string serviceName;
        typed_registration_actor_hdl registrationActor;
        public:

        using super = typed_simple_task_distributor<OutputType, InputType ...>;

        SimpleTaskDistributor( caf::actor_config& config, const std::string& serviceName, typed_registration_actor registrationActor ):
            super(config), serviceName(serviceName), registrationActor(registrationActor)
        {

        }

        typename super::behavior_type make_behavior() override
        {
            return 
            {
                [this]( InputType ... inputs ) -> caf::result<OutputType>
                {
                    auto rp = this->make_response_promise< int >();

                    this->request( registrationActor, caf::infinite, lfge::core::find_service_v, serviceName ).then(
                            [this, rp, intp = std::make_tuple( inputs ... )]( std::string name, caf::actor_addr addr )
                            {
                                std::apply( 
                                    [this, rp, addr]( InputType ... inputs )
                                    {
                                        auto casted_addr = caf::actor_cast< typename typed_simple_task_runner<OutputType, InputType ...>::actor_hdl>(addr);
                                        this->request(casted_addr, caf::infinite, std::forward<InputType>(inputs) ...).then( 
                                            [rp](OutputType output)
                                            {
                                                rp.deliver(output);
                                            },
                                            [rp]( const caf::error e )
                                            {
                                                rp.deliver( std::move(e) );
                                            }
                                        );
                                    }
                                    ,std::move(intp)
                                );
                                
                            },
                            [this](caf::error e)
                            {
                                caf::aout(this) << " SimpleTaskDistributor failed to find the service " << serviceName << " error " << to_string(e) << std::endl;
                            }
                        );

                    return rp;
                }
            };
        }
    };*/
}