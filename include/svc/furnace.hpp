#pragma once

#include <cfg/settings.hpp>
#include <job/pool.hpp>

namespace miu::svc {

class furnace {
  public:
    virtual ~furnace() = default;

    void warmup(cfg::settings const& com, cfg::settings const& spec);
    void forge();
    void finish();

  protected:
    template<typename... ARGS>
    auto add_task(ARGS&&... args) {
        _pool.add(std::forward<ARGS>(args)...);
    }

  private:
    virtual void ignite(cfg::settings const&) = 0;
    virtual void quench()                     = 0;

  private:
    uint32_t _heartbeat { 0 };
    job::pool _pool;
};

}    // namespace miu::svc
