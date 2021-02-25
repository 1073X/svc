
#include <cfg/cmd_source.hpp>
#include <cfg/json_source.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>    // std::err
#include <log/log.hpp>
#include <memory>    // std::unique_ptr

#include "svc/svc.hpp"

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

static auto warmup(std::string_view name, std::string_view file) {
    if (std::filesystem::exists(file)) {
        nlohmann::json json;
        std::ifstream { file.data() } >> json;
        cfg::json_source jsonsrc { name, json };
        g_svc->warmup({ &jsonsrc });
    } else {
        FATAL_ERROR("Missing config, file[", file, "] doesn't exist.");
    }
}

int32_t main(int32_t argc, const char* argv[]) try {
    if (!g_svc) {
        return error("fake furnace");
    }

    cfg::cmd_source cmdsrc { argc, argv };
    cfg::settings cmdset { &cmdsrc };
    if (cmdset.optional<bool>("version", false)) {
        std::cout << g_svc->version() << std::endl;
    } else {
        warmup(cmdset.name(), cmdset.required<std::string>(0));
        g_svc->forge();
    }

    return 0;
} catch (std::exception const& err) {
    return error(err.what());
} catch (...) {
    return error("unknown error");
}
