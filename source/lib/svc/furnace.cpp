
#include "svc/furnace.hpp"

#include <asp/asp.hpp>
#include <asp/version.hpp>
#include <cfg/version.hpp>
#include <cmd/cmd.hpp>
#include <cmd/version.hpp>
#include <com/version.hpp>
#include <csignal>
#include <job/signal.hpp>
#include <job/utility.hpp>
#include <job/version.hpp>
#include <log/log.hpp>
#include <log/version.hpp>
#include <net/version.hpp>
#include <shm/version.hpp>
#include <time/offset.hpp>
#include <time/time.hpp>

#include "svc/version.hpp"

namespace miu::svc {

extern std::string_view concrete_version();

static auto cmd_kill() {
    std::raise(SIGTERM);
}

static auto init_log(cfg::settings const& settings) {
    auto sev_str  = settings.optional<std::string>("severity", "INFO");
    auto severity = com::val_to_enum<enum log::severity>(sev_str);
    auto capacity = settings.optional<uint32_t>("capacity", 4096);
    auto log_type = settings.optional<std::string>("type", "term");
    if ("file" == log_type) {
        auto log_path = settings.required<std::string>("path");
        log::reset(severity, capacity, log_path, meta::name());
    } else if ("syslog" == log_type) {
        log::reset(severity, capacity, meta::name());
    } else if ("term" == log_type) {
        log::reset(severity, capacity);
    } else {
        FATAL_ERROR("invalid log type");
    }
}

void furnace::warmup(cfg::settings const& com, cfg::settings const& spec) {
    // 1. meta
    auto meta = com.required<cfg::settings>("meta");
    meta::set_category(meta.required<std::string>("category"));
    meta::set_type(meta.required<std::string>("type"));
    meta::set_name(meta.required<std::string>("name"));

    // 2. time offset
    auto offset = com.optional<time::delta>("time_offset", time::offset::get());
    time::offset::set(offset);

    // 3. main thread affinity
    job::set_core(com.optional<int32_t>("core", -1));

    // 4. log
    init_log(com.required<cfg::settings>("log"));

    log::debug(+"meta", meta::info(), +"VER", svc::concrete_version());
    log::debug(+"com VER", com::version());
    log::debug(+"log VER", log::version());
    log::debug(+"cfg VER", cfg::version());
    log::debug(+"shm VER", shm::version());
    log::debug(+"net VER", net::version());
    log::debug(+"asp VER", asp::version());
    log::debug(+"cmd VER", cmd::version());
    log::debug(+"job VER", job::version());
    log::debug(+"svc VER", svc::version());

    // 5. specific
    ignite(spec);

    // 6. reset aspects
    asp::read({ +"_COM_", +"job", +"_main_", +"core" }, []() { return job::get_core(); });
    asp::read({ +"_COM_", +"job", +"_main_", +"lag" }, &cmd::interval);
    asp::read({ +"_COM_", +"job", +"_main_", +"hb" }, [this]() { return _heartbeat; });
    asp::read({ +"_COM_", +"job", +"_main_", +"alive" }, [this]() { return _heartbeat > 0; });

    asp::read({ +"_COM_", +"time", +"start" }, []() {
        static auto start_at = time::now();
        return start_at;
    });
    asp::read({ +"_COM_", +"time", +"offset" }, &time::offset::get);

    asp::read({ +"_COM_", +"meta", +"cate" }, []() -> std::string { return meta::category(); });
    asp::read({ +"_COM_", +"meta", +"type" }, []() -> std::string { return meta::type(); });
    asp::read({ +"_COM_", +"meta", +"name" }, []() -> std::string { return meta::name(); });

    asp::read({ +"_COM_", +"log", +"type" }, &log::type);
    asp::read({ +"_COM_", +"log", +"cap" }, &log::capacity);
    asp::read({ +"_COM_", +"log", +"sev" }, []() -> std::string {
        static auto str = com::enum_to_str<enum log::severity>(log::severity());
        return str;
    });

    asp::read({ +"_COM_", +"cmd", +"type" }, &cmd::svr_type);
    asp::read({ +"_COM_", +"cmd", +"intvl" }, &cmd::interval);

    // NOTE: reset after all asps are added
    asp::reset(meta::name());

    // 7. cmd server
    cmd::insert({ "kill" }, &cmd_kill);

    auto interval = com.optional<time::delta>("interval", time::delta { 5000 });
    cmd::reset(meta::name(), interval);
    log::debug(+"cmd SVR", cmd::svr_type(), cmd::interval());

    log::debug(+"time OFF", time::offset::get());
}

void furnace::forge() {
    auto sig = job::signal::instance();

    log::debug(+"job BEG _main_ CORE", job::get_core(), +"LAG", cmd::interval());
    _pool.start();

    while (!sig->value()) {
        _heartbeat++;
        log::dump();
        cmd::handle();
        asp::dump();
    }
    _heartbeat = 0;

    finish();
    log::debug(+"job END _main_");
    log::dump();
}

void furnace::finish() {
    _pool.stop();

    asp::dump();

    asp::reset();
    _pool.clear();
    cmd::reset();
    quench();
}

}    // namespace miu::svc
