#pragma once

#include <cfg/settings.hpp>
#include <job/job.hpp>

namespace miu::svc {

class furnace {
  public:
    virtual ~furnace() {}

    void warmup(cfg::settings const&);
    void forge();
    void finish();

  protected:
    template<typename... ARGS>
    auto add_task(ARGS&&... args) {
        job::add(std::forward<ARGS>(args)...);
    }

  private:
    virtual void ignite(cfg::settings const&) = 0;
    virtual void quench()                     = 0;

  private:
    uint32_t _heartbeat { 0 };
};

}    // namespace miu::svc