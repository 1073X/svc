#pragma once

#include <cfg/json_source.hpp>
#include <cfg/settings.hpp>
#include <com/fatal_error.hpp>
#include <filesystem>
#include <log/log.hpp>

namespace miu::svc {

class settings {
  public:
    settings(std::string_view bin_name, std::filesystem::path const& json_file) {
        if (!std::filesystem::exists(json_file)) {
            FATAL_ERROR<std::ios_base::failure>("missing config file[", json_file.string(), "]");
        }

        std::ifstream ss { json_file };
        auto json = nlohmann::json::parse(ss);

        if (!json.contains("com")) {
            json["com"] = {};
        }
        if (!json["com"].contains("meta")) {
            json["com"]["meta"] = {};
        }
        if (!json["com"]["meta"].contains("category")) {
            json["com"]["meta"]["category"] = bin_name;
            log::debug(+"svc apply default category", bin_name);
        }
        if (!json["com"]["meta"].contains("type")) {
            json["com"]["meta"]["type"] = bin_name;
            log::debug(+"svc apply default type", bin_name);
        }
        if (!json["com"]["meta"].contains("name")) {
            json["com"]["meta"]["name"] = json_file.stem().string();
            log::debug(+"svc apply default name", bin_name);
        }

        if (!json.contains("spec")) {
            json["spec"] = {};
            log::debug(+"svc apply default spec");
        }

        _src = new cfg::json_source { bin_name, json };
        _raw = cfg::settings { _src };
    }

    settings(settings const&) = delete;
    auto operator=(settings const&) = delete;

    ~settings() { delete _src; }

    auto com() const { return _raw.required<cfg::settings>("com"); }
    auto spec() const { return _raw.required<cfg::settings>("spec"); }

  private:
    cfg::json_source* _src;
    cfg::settings _raw;
};

}    // namespace miu::svc
