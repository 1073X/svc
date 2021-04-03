
#include <log/log.hpp>

#include "svc/furnace.hpp"
#include "svc/version.hpp"

namespace miu::sample {

class furnace : public svc::furnace {
  private:
    void proc(job::status* st) {
        while (st->beat()) {
            log::info(+"beating ...");
        }
    }

  private:    // impl furnace
    void ignite(cfg::settings const& settings) override {
        auto core     = settings.required<int32_t>("core");
        auto interval = settings.required<time::delta>("interval");
        add_task("proc0", core, interval, &furnace::proc, this);
        add_task("proc1", std::chrono::seconds(1), &furnace::proc, this);
    }

    void quench() override {}
};

}    // namespace miu::sample
