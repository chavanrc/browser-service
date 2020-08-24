#include <catch2/catch.hpp>
#include <execution.hpp>

TEST_CASE("execution test", "[unit]") {
    int32_t     argc{3};
    const char *argv[]{"execution", "-f", "./config/config_execution.json"};
    holding::execution::Start(argc, argv);
}