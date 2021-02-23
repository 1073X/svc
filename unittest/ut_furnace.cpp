#include <gmock/gmock.h>

#include <cfg/json_source.hpp>
#include <com/thread_id.hpp>
#include <csignal>
#include <filesystem>
#include <log/log.hpp>
#include <nlohmann/json.hpp>
#include <shm/tempfs.hpp>
#include <thread>
#include <time/offset.hpp>
#include <time/time.hpp>

#include "svc/furnace.hpp"

using namespace std::chrono_literals;

struct ut_furnace : public testing::Test {
    void TearDown() override { miu::log::dump(); }

    std::string asp_file { "ut_furnace.asp" };

    struct : public miu::svc::furnace {
        MOCK_METHOD(void, do_ignite, (miu::cfg::settings const&));

        void ignite(miu::cfg::settings const& settings) override {
            add_task("task", 1, 1s, [](auto) {});
            do_ignite(settings);
        }

        MOCK_METHOD(void, quench, (), (override));
    } furnace;
};

TEST_F(ut_furnace, log_file) {
    using namespace std::chrono_literals;

    // hold current time offset for restoring
    auto offset = miu::time::offset::get();

    // remove log file
    miu::time::date date { (miu::time::now() - 4h).time_since_epoch() };
    auto log_file = "ut_furnace_" + miu::com::to_string(date) + ".log";
    std::filesystem::remove(log_file);

    nlohmann::json json;
    json["com"]["meta"]["category"] = "cate";
    json["com"]["meta"]["type"]     = "type";
    json["com"]["meta"]["name"]     = "ut_furnace";
    json["com"]["time_offset"]      = 4 * 3600 * 1000;
    json["com"]["log"]["type"]      = "file";
    json["com"]["log"]["path"]      = ".";
    json["com"]["log"]["severity"]  = "DEBUG";
    json["com"]["log"]["capacity"]  = 4096;
    json["spec"]                    = {};
    miu::cfg::json_source src { "ut_furnace", json };
    miu::cfg::settings settings { &src };

    EXPECT_CALL(furnace, do_ignite(testing::_));
    furnace.warmup(settings);

    // should reset meta info
    EXPECT_STREQ("cate.type.ut_furnace", miu::meta::info());
    // should reset time offset
    EXPECT_EQ(4h, miu::time::offset::get());
    // should create log file
    EXPECT_TRUE(std::filesystem::exists(log_file));
    // should create asp file
    EXPECT_TRUE(miu::shm::tempfs::exists(asp_file));

    EXPECT_CALL(furnace, quench());
    furnace.finish();

    miu::time::offset::set(offset);
}

TEST_F(ut_furnace, log_term) {
    nlohmann::json json;
    json["com"]["meta"]["category"] = "cate";
    json["com"]["meta"]["type"]     = "type";
    json["com"]["meta"]["name"]     = "ut_furnace";
    // json["com"]["log"]["type"]      = "term";
    json["com"]["log"]["severity"] = "DEBUG";
    json["com"]["core"]            = 4;
    json["com"]["interval"]        = 200;
    json["spec"]                   = {};
    miu::cfg::json_source src { "ut_furnace", json };
    miu::cfg::settings settings { &src };

    EXPECT_CALL(furnace, do_ignite(testing::_));
    EXPECT_CALL(furnace, quench());

    furnace.warmup(settings);

    std::thread thrd([&]() {
        furnace.forge();
        miu::com::thread_id::reset();
    });

    std::this_thread::sleep_for(100ms);
    std::raise(SIGTERM);

    thrd.join();
}

TEST_F(ut_furnace, log_sys) {
    nlohmann::json json;
    json["com"]["meta"]["category"] = "cate";
    json["com"]["meta"]["type"]     = "type";
    json["com"]["meta"]["name"]     = "ut_furnace";
    json["com"]["log"]["type"]      = "syslog";
    json["spec"]                    = {};
    miu::cfg::json_source src { "ut_furnace", json };
    miu::cfg::settings settings { &src };

    EXPECT_CALL(furnace, do_ignite(testing::_));
    furnace.warmup(settings);

    EXPECT_CALL(furnace, quench());
    furnace.finish();
}

TEST_F(ut_furnace, invalid_log_type) {
    nlohmann::json json;
    json["com"]["meta"]["category"] = "cate";
    json["com"]["meta"]["type"]     = "type";
    json["com"]["meta"]["name"]     = "ut_furnace";
    json["com"]["log"]["type"]      = "unknown";
    miu::cfg::json_source src { "ut_furnace", json };
    miu::cfg::settings settings { &src };

    EXPECT_ANY_THROW(furnace.warmup(settings));
}
