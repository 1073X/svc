
#include <cfg/cmd_source.hpp>
#include <cfg/json_source.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>    // std::err
#include <log/log.hpp>
#include <memory>    // std::unique_ptr

#include "source/lib/svc/settings.hpp"
#include "svc/svc.hpp"

namespace miu::svc {
// should be implemented in concrete service
extern std::string_view concrete_version();
extern std::string_view concrete_build_info();
}    // namespace miu::svc

using namespace miu;

std::unique_ptr<svc::furnace> g_svc { svc::create() };

static auto error(std::string_view text) {
    std::cerr << text << std::endl;
    log::error(text);
    if (g_svc) {
        g_svc->finish();
    }
    return -1;
}

int32_t main(int32_t argc, const char* argv[]) try {
    if (!g_svc) {
        return error("fake furnace");
    }

    cfg::cmd_source cmdsrc { argc, argv };
    cfg::settings cmdset { &cmdsrc };
    if (cmdset.optional<bool>("version", false)) {
        std::cout << svc::concrete_version() << std::endl;

    } else if (cmdset.optional<bool>("usage", false)) {
        std::cout << svc::concrete_version() << std::endl;
        std::cout << svc::concrete_build_info() << std::endl;
        std::cout << "\nUsage: " << cmdset.name() << " <path/to/json/config/file>" << std::endl;
        std::cout << "\t--version: show version" << std::endl;
        std::cout << "\t--usage: show this usage" << std::endl;

    } else {
        {
            // release settings after initialization
            auto bin_name  = cmdset.name();
            auto json_file = cmdset.required<std::string>(0);
            svc::settings settings { bin_name, json_file };
            g_svc->warmup(settings.com(), settings.spec());
        }

        g_svc->forge();
    }

    return 0;
} catch (std::exception const& err) {
    return error(err.what());
} catch (...) {
    return error("unknown error");
}
