#define CAF_SUITE core_tests

#include "caf/test/dsl.hpp"
#include "core/heartbeat.hpp"
#include <chrono>

using namespace caf;
using namespace lfge::core;

struct heartbeat_fixture : test_coordinator_fixture<>
{
    heartbeat_fixture()
    {
        run();
    }
};

CAF_TEST_FIXTURE_SCOPE( heartbeat_reciever_tests, heartbeat_fixture )

CAF_TEST(heartbeat_sender_reciever){
    int rec = 0;
    auto reciever = sys.spawn<heartbeat_receiver>("hb_reciever", [&rec](auto &&){
        ++rec;
        CAF_MESSAGE("HB_RECIEVED ");
    },
    0,
    [](auto&&){
        CAF_FAIL("HB_RECIEVER TIMEOUT");
    } );

    auto ract = caf::actor_cast<actor>(reciever);
    self->send( ract, heartbeat_atom_v, std::size_t(10) );

    expect( (heartbeat_atom, std::size_t), from(self).to(ract).with( heartbeat_atom_v, 10 ) );

    expect( (heartbeat_reply_atom, std::size_t), from(ract).to(self).with( heartbeat_reply_atom_v, 10 ) );

    CAF_CHECK(rec, 1);
}

CAF_TEST_FIXTURE_SCOPE_END()