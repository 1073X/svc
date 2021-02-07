#include <gtest/gtest.h>

#include <iostream>
#include <meta/info.hpp>

#include "svc/version.hpp"

TEST(ut_version, version) {
    std::cout << miu::svc::version() << std::endl;
    std::cout << miu::svc::build_info() << std::endl;

    std::cout << miu::meta::info() << std::endl;
}
