#pragma once
#include "caf/all.hpp"
#include <functional>
#include <vector>
#include <list>
#include <algorithm>

namespace lfge::resource_manager
{
    namespace task_distribution
    {
        template< typename OutputType, typename ... InputType >
        using typed_split_and_agg_task_distributor = caf::typed_actor<
            caf::result<OutputType>( InputType ... )
        >;

        template< typename OutputType, typename ... InputType >
        using typed_split_and_agg_task_task_runner = caf::typed_actor<
            caf::result<OutputType>( InputType ... )
        >;

        template< typename OutputType, typename SubtaskType, typename SubtaskResponseType, typename ... InputType >
        class split_and_aggregate_distributor : public typed_split_and_agg_task_distributor<OutputType, InputType ...>::base
        {
            public:

            using super = typename typed_split_and_agg_task_distributor<OutputType, InputType ...>::base;
            using splitter_function = std::function< std::vector<SubtaskType>( InputType ... ) >;
            using aggregation_function = std::function< OutputType( const std::vector<SubtaskResponseType>& ) >;
            using on_aggregate_result_recieved = std::function< void (SubtaskResponseType&) >;

            protected:
            
            std::string serviceName;
            lfge::resource_manager::typed_registration_actor registrationActor;
            splitter_function splitterFunction;
            aggregation_function aggregationFunction;
            on_aggregate_result_recieved on_res;
            bool retryOnFailure;
            int maxRetryCount;

            using response_promise_type = typename caf::detail::make_response_promise_helper<OutputType>::type;

            struct task
            {
                task( std::size_t size, const response_promise_type& rp) : size(size),responsePromise(rp)
                {
                    result_vector.resize(size);
                    numberOfRetries.resize(size, 0);
                    hasRes.resize(size, false);
                }
                std::size_t size;
                std::vector<SubtaskResponseType> result_vector;
                std::vector<int> numberOfRetries;
                std::vector<bool> hasRes;
                response_promise_type responsePromise;
            };

            using task_container = std::list<task>;
            task_container tasks_pending;

            public:

            split_and_aggregate_distributor(  caf::actor_config& config,
                                               const std::string& serviceName, 
                                               lfge::resource_manager::typed_registration_actor registrationActor,
                                               splitter_function splitterFunction,
                                               aggregation_function aggregationFunction,
                                               bool canReplyOnFailedSubtask = false,
                                               on_aggregate_result_recieved on_res = nullptr,
                                               bool retryOnFailure = false,
                                               int maxRetryCount = 3
                                            ):
                super(config), 
                serviceName(serviceName), 
                registrationActor(registrationActor), 
                splitterFunction(splitterFunction), 
                aggregationFunction(aggregationFunction),
                on_res(on_res),
                retryOnFailure(retryOnFailure),
                maxRetryCount(maxRetryCount)
            {
            }

            bool hasFinishedTheTreatment( typename task_container::iterator& taskIte )
            {
                for( std::size_t i=0; i < taskIte->size; ++i )
                {
                    if( !taskIte->hasRes[i] && ( !retryOnFailure || taskIte->numberOfRetries[i] < maxRetryCount ) )
                        return false;
                }
                return true;
            }

            void distributeOne( const SubtaskType& subtask, typename task_container::iterator taskIte, const std::size_t dataIndex)
            {
                this->request( registrationActor, caf::infinite, lfge::core::find_service_v, serviceName ).then(
                        [this, subtask,taskIte,dataIndex]( std::string name, caf::actor_addr addr ) mutable
                        {
                            auto casted_addr = caf::actor_cast< typed_split_and_agg_task_task_runner<SubtaskResponseType, SubtaskType> >(addr);
                            this->request(casted_addr, caf::infinite, subtask ).then( 
                                        [this,taskIte,dataIndex](SubtaskResponseType output) mutable
                                        {
                                            taskIte->result_vector[dataIndex] = output;
                                            taskIte->hasRes[dataIndex] = true;
                                            if( hasFinishedTheTreatment(taskIte) )
                                            {
                                                taskIte->responsePromise.deliver( aggregationFunction(taskIte->result_vector) );
                                            }
                                        },
                                        [this,taskIte,dataIndex]( const caf::error e ) mutable
                                        {
                                            taskIte->responsePromise.deliver(e);
                                        }
                                    );
                        },
                        [this, taskIte]( caf::error e )
                        {
                            taskIte->responsePromise.deliver(e);
                        });
            }

            virtual typename super::behavior_type make_behavior() override
            {
                return {
                    [this]( InputType ... inputs ) -> caf::result<OutputType>
                    {
                        auto rp = this->template make_response_promise< OutputType >();
                        
                        std::vector<SubtaskType> splittedTasks = splitterFunction( inputs ... );
                        tasks_pending.push_front( task( splittedTasks.size(), rp ) );
                        typename task_container::iterator for_current_task = tasks_pending.begin();
                        for( auto ite = splittedTasks.begin(); ite != splittedTasks.end(); ++ite )
                        {
                            distributeOne( *ite, for_current_task, std::distance( splittedTasks.begin(), ite ) );
                        }
                        return rp;
                    }
                };
            }
        };
    }
}