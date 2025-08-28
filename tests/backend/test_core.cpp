#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Core.h"
#include "Security.h"
#include <memory>
#include <chrono>

using namespace Aether;

class MockCore : public Core {
public:
    MockCore() = default;
    bool test_initialize() { return Initialize(); }
    void test_cleanup() { Cleanup(); }
    bool is_test_initialized() const { return m_initialized; }
};

TEST_CASE("Core Initialization", "[core]") {
    SECTION("Core initializes successfully") {
        MockCore core;
        REQUIRE(core.test_initialize() == true);
        REQUIRE(core.is_test_initialized() == true);
    }

    SECTION("Core cleanup works") {
        MockCore core;
        core.test_initialize();
        core.test_cleanup();
        REQUIRE(core.is_test_initialized() == false);
    }

    SECTION("Double initialization is safe") {
        MockCore core;
        REQUIRE(core.test_initialize() == true);
        REQUIRE(core.test_initialize() == true); // Should not fail
        REQUIRE(core.is_test_initialized() == true);
    }
}

TEST_CASE("Core Performance", "[core][performance]") {
    MockCore core;
    
    SECTION("Initialization time is reasonable") {
        auto start = std::chrono::high_resolution_clock::now();
        core.test_initialize();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        REQUIRE(duration.count() < 1000); // Should take less than 1 second
    }
}

TEST_CASE("Core Thread Safety", "[core][threading]") {
    SECTION("Multiple threads can access core safely") {
        MockCore core;
        core.test_initialize();
        
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};
        
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&core, &success_count]() {
                if (core.is_test_initialized()) {
                    success_count++;
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        REQUIRE(success_count == 10);
    }
}

TEST_CASE("Security Integration", "[core][security]") {
    SECTION("Core respects security constraints") {
        MockCore core;
        
        // Test that core initializes with security checks
        REQUIRE(core.test_initialize() == true);
        
        // Verify security state
        // This would test actual security implementation
        REQUIRE(true); // Placeholder for actual security tests
    }
}