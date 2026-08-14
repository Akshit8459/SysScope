#include "tests/test_framework.hpp"
#include "sysscope/core/result.hpp"
#include <string>

TEST_CASE("Core::Result Success and Error Cases") {
    auto ok_res = sysscope::core::Result<int>(42);
    REQUIRE(ok_res.is_ok());
    REQUIRE(!ok_res.is_err());
    REQUIRE_EQ(ok_res.value(), 42);
    REQUIRE_EQ(ok_res.value_or(100), 42);

    auto err_res = sysscope::core::Result<int>::error("Sensor offline");
    REQUIRE(!err_res.is_ok());
    REQUIRE(err_res.is_err());
    REQUIRE_EQ(err_res.error_message(), "Sensor offline");
    REQUIRE_EQ(err_res.value_or(100), 100);
}
