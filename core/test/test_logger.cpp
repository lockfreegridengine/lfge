#define CAF_SUITE core_tests

#include "caf/test/dsl.hpp"
#include "core/logger.hpp"
#include "core/core_atoms.hpp"
#include <chrono>

using namespace caf;
using namespace lfge::core;

class logger_fixture : public test_coordinator_fixture<>
{
    public:
    logger_fixture()
    {
        lfge::core::logger::init( sys );
    }

    ~logger_fixture()
    {
        lfge::core::logger::stop();
    }
};


CAF_TEST_FIXTURE_SCOPE( logger_tests, logger_fixture )

CAF_TEST( test_sending_log_message  ){
    auto grp = sys.groups().get_local(lfge::core::logger::log_group_name);
    self->join(grp);

    lfge::core::logger::log( lfge::core::loglevel::error, "big error" );
    sched.run();

    expect( ( lfge::core::loglevel, std::string ), from(_).to(self).with( lfge::core::loglevel::error, std::string("big error")) );
}

CAF_TEST_FIXTURE_SCOPE_END()