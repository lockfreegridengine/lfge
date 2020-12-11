#pragma once

#include "core_atoms.hpp"
#include <unordered_map>
#include <stdexcept>

enum class DataContainerErrors : uint8_t {
    elementNotFound = 1,
};


CAF_ERROR_CODE_ENUM(DataContainerErrors)

namespace lfge::core{

    template <typename Value, typename Container>
    class DataContainerActor : public caf::stateful_actor<Container>
    {
        public:
        using value_type = Value;
        using container_type = Container;
        
        Value defaultValue = Value();

        DataContainerActor(caf::actor_config& cfg) : caf::stateful_actor< Container >(cfg)
        {
        }

        caf::behavior make_behavior() override
        {
            return {
                [this](read_atom, const std::size_t &index)
                {
                    if( this->state.size() > index )
                    {
                        auto ite = this->state.begin();
                        std::advance( ite, index );
                        return *ite;
                    }
                    return defaultValue;
                },
                [this]( write_atom, const Value& v )
                {
                    this->state.insert(v);
                },
                [this]( contains_atom, const Value& v )
                {
                    return std::find(this->state.begin(), this->state.end(), v) != this->state.end();
                }
            };
        }
    };

    template <typename Value>
    class DataContainerActor<Value, std::vector<Value>> : public caf::stateful_actor<std::vector<Value>>
    {
        public:
        using value_type = Value;
        using container_type = std::vector<Value>;
        
        Value defaultValue = Value();

        DataContainerActor(caf::actor_config& cfg) : caf::stateful_actor< container_type >(cfg)
        {
        }

        caf::behavior make_behavior() override
        {
            return {
                [this](read_atom, const std::size_t &index)
                {
                    if( this->state.size() > index )
                    {
                        return this->state[index];
                    }
                    return defaultValue;
                },
                [this]( write_atom, const Value& v )
                {
                    this->state.push_back(v);
                },
                [this]( contains_atom, const Value& v )
                {
                    return std::find(this->state.begin(), this->state.end(), v) != this->state.end();
                }
            };
        }
    };


    /**
     * @brief An actor which stores a container of type key, value
     *        It provides 3 ways to access the elements in the container
     *          1) Read -> To read the element in the container
     *          2) Write -> To write an element into container
     *          3) Contains -> To check if the given key is in the container
     *  It also contains a default value which is the default value for a given key if it does not exist in the container.
     *  WARN : Read will add a key with a default value if the key does not exists. So read will never reaturn an exception. Please check using contains if the key exists.
     *  Constraints : 
     *          Value should have default constructor,
     *          Key and value should be copyable -> should have copy constructor and equality operators
     * @tparam Key 
     * @tparam Value 
     * @tparam Container -> by default std::unordered_map
     * @tparam Value> 
     */
    template<typename Key, typename Value, typename Container = std::unordered_map<Key, Value>>
    class DataContainerMapActor : public caf::stateful_actor<Container>
    {
        public:
        using value_type = Value;
        using container_type = Container;
        using key_type = Key;

        Value defaultValue = Value();

        DataContainerMapActor(caf::actor_config& cfg) : caf::stateful_actor< Container >(cfg)
        {

        }

        caf::behavior make_behavior() override
        {
            return {
                [this]( read_atom, const Key& key )
                {
                    auto ite = this->state.find( key );
                    if( ite == this->state.end() )
                    {
                        ite = this->state.insert( std::make_pair( key, defaultValue ) ).first;
                    }
                    return caf::make_result(key, this->state[key]);
                },
                [this]( write_atom, const Key& key, const Value& value )
                {
                    this->state[key] = value;
                },
                [this]( contains_atom, const Key& key )
                {
                    return this->state.count(key) > 0;
                }
            };
        }
    };

}