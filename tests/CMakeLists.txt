cmake_minimum_required(VERSION 3.20)

# Find Catch2
find_package(Catch2 3)
if(NOT Catch2_FOUND)
    message(WARNING "Catch2 not found - tests will be disabled")
    return()
endif()

# Backend Tests
set(BACKEND_TEST_SOURCES
    backend/test_core.cpp
    backend/test_ipc.cpp
    backend/test_security.cpp
    backend/test_ai_controller.cpp
    backend/test_memory_patcher.cpp
    backend/test_polymorphic_engine.cpp
    backend/test_network_manager.cpp
)

add_executable(backend_tests ${BACKEND_TEST_SOURCES})

target_link_libraries(backend_tests 
    PRIVATE 
    Catch2::Catch2WithMain
)

if(nlohmann_json_FOUND)
    target_link_libraries(backend_tests PRIVATE nlohmann_json::nlohmann_json)
endif()

if(cryptopp_FOUND)
    target_link_libraries(backend_tests PRIVATE cryptopp::cryptopp)
endif()

if(Boost_FOUND)
    target_link_libraries(backend_tests PRIVATE Boost::system Boost::filesystem Boost::thread)
endif()

if(spdlog_FOUND)
    target_link_libraries(backend_tests PRIVATE spdlog::spdlog)
endif()

target_include_directories(backend_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/tests/include
)

# Add tests to CTest
include(CTest)
include(Catch)
catch_discover_tests(backend_tests)

# Performance Tests
add_executable(performance_tests
    backend/perf_luau_execution.cpp
    backend/perf_ipc_throughput.cpp
    backend/perf_memory_operations.cpp
)

target_link_libraries(performance_tests 
    PRIVATE 
    Catch2::Catch2WithMain
)

if(nlohmann_json_FOUND)
    target_link_libraries(performance_tests PRIVATE nlohmann_json::nlohmann_json)
endif()

if(cryptopp_FOUND)
    target_link_libraries(performance_tests PRIVATE cryptopp::cryptopp)
endif()

if(Boost_FOUND)
    target_link_libraries(performance_tests PRIVATE Boost::system)
endif()

if(spdlog_FOUND)
    target_link_libraries(performance_tests PRIVATE spdlog::spdlog)
endif()

target_include_directories(performance_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/tests/include
)

# Integration Tests
add_executable(integration_tests
    integration/test_full_pipeline.cpp
    integration/test_frontend_backend_comm.cpp
    integration/test_script_execution_flow.cpp
)

target_link_libraries(integration_tests 
    PRIVATE 
    Catch2::Catch2WithMain
)

if(nlohmann_json_FOUND)
    target_link_libraries(integration_tests PRIVATE nlohmann_json::nlohmann_json)
endif()

if(cryptopp_FOUND)
    target_link_libraries(integration_tests PRIVATE cryptopp::cryptopp)
endif()

if(Boost_FOUND)
    target_link_libraries(integration_tests PRIVATE Boost::system)
endif()

if(spdlog_FOUND)
    target_link_libraries(integration_tests PRIVATE spdlog::spdlog)
endif()

target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/tests/include
)

# Test data
file(COPY test_data DESTINATION ${CMAKE_BINARY_DIR}/tests)

# Add custom targets
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel
    DEPENDS backend_tests performance_tests integration_tests
    COMMENT "Running all tests"
)

add_custom_target(run_backend_tests
    COMMAND $<TARGET_FILE:backend_tests>
    DEPENDS backend_tests
    COMMENT "Running backend tests"
)

add_custom_target(run_performance_tests
    COMMAND $<TARGET_FILE:performance_tests>
    DEPENDS performance_tests
    COMMENT "Running performance tests"
)