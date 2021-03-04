#include <gtest/gtest.h>

#include <fstream>

#include "source/lib/svc/settings.hpp"

TEST(ut_settings, load) {
    miu::com::json json;
    json["com"]["meta"]["category"] = "cate";
    json["com"]["meta"]["type"]     = "type";
    json["com"]["meta"]["name"]     = "name";
    json["spec"]                    = {};

    std::ofstream { "ut_settings.json" } << json;

    miu::svc::settings settings { "bin", "ut_settings.json" };

    auto meta = settings.com().required<miu::cfg::settings>("meta");
    EXPECT_EQ("cate", meta.required<std::string>("category"));
    EXPECT_EQ("type", meta.required<std::string>("type"));
    EXPECT_EQ("name", meta.required<std::string>("name"));

    EXPECT_TRUE(settings.spec());
}

TEST(ut_settings, default) {
    miu::com::json json;
    std::ofstream { "name.json" } << json;

    miu::svc::settings settings { "bin", "name.json" };

    auto meta = settings.com().required<miu::cfg::settings>("meta");
    EXPECT_EQ("bin", meta.required<std::string>("category"));
    EXPECT_EQ("bin", meta.required<std::string>("type"));
    EXPECT_EQ("name", meta.required<std::string>("name"));

    EXPECT_TRUE(settings.spec());
}

TEST(ut_settings, invalid_file) {
    EXPECT_THROW(miu::svc::settings("bin", "not_exists.json"), std::ios_base::failure);
}
