#include <string>
#include <iostream>

#include <qb/actor.h>
#include <qb/main.h>

#ifdef NDEBUG
#    define MAX_BENCHMARK_ITERATION 10
#    define SHIFT_NB_EVENT 15
#else
#    define SHIFT_NB_EVENT 4
#    define MAX_BENCHMARK_ITERATION 1
#endif

struct TinyEvent : qb::Event {
    uint64_t _ttl;
    explicit TinyEvent(uint64_t y)
        : _ttl(y) {}
};

struct BigEvent : qb::Event {
    uint64_t _ttl;
    uint64_t padding[127];
    explicit BigEvent(uint64_t y)
        : _ttl(y)
        , padding() {}
};

struct DynamicEvent : qb::Event {
    uint64_t _ttl;
    std::vector<int> vec;
    explicit DynamicEvent(uint64_t y)
        : _ttl(y)
        , vec(512, 8) {}
};

template <typename TestEvent>
class PongActor final : public qb::Actor {
public:
    bool
    onInit() final {
        registerEvent<TestEvent>(*this);
        return true;
    }

    void
    on(TestEvent &event) {
      std::cout << "PONG\n";
        --event._ttl;
        reply(event);
    }
};

template <typename TestEvent>
class PingActor final : public qb::Actor {
    const uint64_t max_sends;
    const qb::ActorId actor_to_send;

public:
    PingActor(uint64_t const max, qb::ActorId const id)
        : max_sends(max)
        , actor_to_send(id) {}
    ~PingActor() final = default;

    bool
    onInit() final {
        registerEvent<TestEvent>(*this);
        send<TestEvent>(actor_to_send, max_sends);
        return true;
    }

    void
    on(TestEvent &event) {
      std::cout << "PING\n";
        if (event._ttl)
            reply(event);
        else {
            kill();
            send<qb::KillEvent>(event.getSource());
        }
    }
};

int main()
{
  std::cout << "Testing qb code" << std::endl;
  qb::Main main;
  const auto max_events = 10;
  const auto nb_actor = 1;
  const int nb_core = 4;
  for (int i = nb_actor; i > 0;) {
      for (auto j = 0u; j < nb_core && i > 0; ++j) {
          main.addActor<PingActor<TinyEvent>>(
              j, max_events,
              main.addActor<PongActor<TinyEvent>>(((j + 1) % nb_core)));
          --i;
      }
  }

  main.start();
  main.join();
  return 0;

}