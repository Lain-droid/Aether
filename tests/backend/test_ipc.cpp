#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "IPC.h"
#include <memory>
#include <chrono>
#include <future>

using namespace Aether;

class TestIPCServer : public IPCServer {
public:
    TestIPCServer() : IPCServer("test_pipe") {}
    
    bool test_start() { return Start(); }
    void test_stop() { Stop(); }
    bool is_test_running() const { return IsRunning(); }
    
    // Override message handler for testing
    void HandleMessage(const std::string& message) override {
        m_last_message = message;
        m_message_count++;
    }
    
    std::string m_last_message;
    int m_message_count = 0;
};

class TestIPCClient : public IPCClient {
public:
    TestIPCClient() : IPCClient("test_pipe") {}
    
    bool test_connect() { return Connect(); }
    void test_disconnect() { Disconnect(); }
    bool test_send(const std::string& msg) { return SendMessage(msg); }
    bool is_test_connected() const { return IsConnected(); }
};

TEST_CASE("IPC Server Lifecycle", "[ipc][server]") {
    TestIPCServer server;
    
    SECTION("Server starts successfully") {
        REQUIRE(server.test_start() == true);
        REQUIRE(server.is_test_running() == true);
        server.test_stop();
    }
    
    SECTION("Server stops successfully") {
        server.test_start();
        server.test_stop();
        REQUIRE(server.is_test_running() == false);
    }
    
    SECTION("Double start is safe") {
        REQUIRE(server.test_start() == true);
        REQUIRE(server.test_start() == true); // Should not fail
        server.test_stop();
    }
}

TEST_CASE("IPC Client Connection", "[ipc][client]") {
    TestIPCServer server;
    TestIPCClient client;
    
    server.test_start();
    
    SECTION("Client connects successfully") {
        REQUIRE(client.test_connect() == true);
        REQUIRE(client.is_test_connected() == true);
        client.test_disconnect();
    }
    
    SECTION("Client disconnects successfully") {
        client.test_connect();
        client.test_disconnect();
        REQUIRE(client.is_test_connected() == false);
    }
    
    server.test_stop();
}

TEST_CASE("IPC Message Passing", "[ipc][messaging]") {
    TestIPCServer server;
    TestIPCClient client;
    
    server.test_start();
    client.test_connect();
    
    SECTION("Simple message transmission") {
        std::string test_message = "Hello, IPC!";
        REQUIRE(client.test_send(test_message) == true);
        
        // Wait a bit for message to be processed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        REQUIRE(server.m_message_count == 1);
        REQUIRE(server.m_last_message == test_message);
    }
    
    SECTION("Multiple messages") {
        std::vector<std::string> messages = {
            "Message 1",
            "Message 2", 
            "Message 3"
        };
        
        for (const auto& msg : messages) {
            REQUIRE(client.test_send(msg) == true);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        REQUIRE(server.m_message_count == 3);
        REQUIRE(server.m_last_message == messages.back());
    }
    
    SECTION("Large message handling") {
        std::string large_message(65536, 'A'); // 64KB message
        REQUIRE(client.test_send(large_message) == true);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        REQUIRE(server.m_message_count == 1);
        REQUIRE(server.m_last_message == large_message);
    }
    
    client.test_disconnect();
    server.test_stop();
}

TEST_CASE("IPC Performance", "[ipc][performance]") {
    TestIPCServer server;
    TestIPCClient client;
    
    server.test_start();
    client.test_connect();
    
    SECTION("Message throughput test") {
        const int message_count = 1000;
        const std::string test_message = "Performance test message";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < message_count; ++i) {
            REQUIRE(client.test_send(test_message) == true);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should handle 1000 messages in reasonable time
        REQUIRE(duration.count() < 5000); // Less than 5 seconds
        
        // Wait for all messages to be processed
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        REQUIRE(server.m_message_count == message_count);
    }
    
    client.test_disconnect();
    server.test_stop();
}

TEST_CASE("IPC Error Handling", "[ipc][error]") {
    SECTION("Client connection without server") {
        TestIPCClient client;
        REQUIRE(client.test_connect() == false);
        REQUIRE(client.is_test_connected() == false);
    }
    
    SECTION("Send message without connection") {
        TestIPCClient client;
        REQUIRE(client.test_send("test") == false);
    }
    
    SECTION("Multiple clients to same server") {
        TestIPCServer server;
        TestIPCClient client1, client2;
        
        server.test_start();
        
        REQUIRE(client1.test_connect() == true);
        REQUIRE(client2.test_connect() == true);
        
        REQUIRE(client1.test_send("from client 1") == true);
        REQUIRE(client2.test_send("from client 2") == true);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        REQUIRE(server.m_message_count == 2);
        
        client1.test_disconnect();
        client2.test_disconnect();
        server.test_stop();
    }
}