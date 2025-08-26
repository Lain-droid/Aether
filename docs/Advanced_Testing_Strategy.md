# AetherVisor: Advanced Testing Strategy for Adaptive Systems
**Version 2.0**

This document outlines the testing strategy for the dynamic, AI-driven, and deeply obfuscated features of the AetherVisor project. It has been updated to cover the "Evolution" phase of development.

## 1. Testing Philosophy

The system is now highly non-deterministic and adaptive. Testing must focus on behavioral verification, state inspection, and component isolation. The goal is to confirm that the *logic* of the adaptive systems is working correctly, even if the exact output is different each time.

## 2. Test Scenarios - Phase 1 Features (Recap)

- **AIController Risk Accrual/Decay:** Verified by reporting events and checking the risk score.
- **Conditional Injection:** Verified by setting risk levels and checking which injection method is logged.
- **Polymorphic Cleanup:** Verified by logging the randomized order of cleanup tasks.

## 3. Test Scenarios - Phase 2 (Evolution) Features

### 3.1. Advanced Polymorphism & Encryption
- **Objective:** Verify string obfuscation and advanced code mutation.
- **Scenario 1 (String Encryption):**
    1. Compile the payload DLL.
    2. Open the resulting DLL file in a hex editor or with a `strings` utility.
    3. Search for the string "ws2_32.dll".
    4. **Expected Result:** The string should **not** be found in plaintext.
- **Scenario 2 (Junk Code Mutation):**
    1. In `PolymorphicEngine.cpp`, log the size of the payload vector before and after `Mutate()` is called.
    2. **Expected Result:** The size after mutation should be larger than the original, and the amount of increase should vary between runs.

### 3.2. Dynamic Function Finding (`SignatureScanner`)
- **Objective:** Verify that the signature scanner correctly finds function addresses.
- **Scenario:**
    1. Use a debugger (like x64dbg) to manually find the address of `ws2_32.dll!send` in a running process.
    2. In `NetworkManager.cpp`, log the address returned by `SignatureScanner::FindPattern` for the `send` function.
    3. **Expected Result:** The logged address must match the address found manually in the debugger.

### 3.3. Advanced Hook Engine (`EventManager` Trampoline)
- **Objective:** Verify the integrity and stability of the new trampoline hooking mechanism.
- **Scenario:**
    1. Hook a function (e.g., `send`).
    2. In the detour, call the original function via the trampoline: `original_send = eventManager.GetOriginal<send_t>(sendAddr); original_send(...)`.
    3. Send a known piece of data.
    4. **Expected Result:** The data should be sent successfully without crashing the application. The detour function should be hit, and the program should behave as if the original function was called correctly. This confirms the trampoline correctly executes the original code and returns.

### 3.4. Advanced AI (`AIController` Learning)
- **Objective:** Verify that the AI's risk weights are updated based on negative feedback.
- **Scenario:**
    1. Add logging in the `AIController` constructor to print the initial risk weight for `AIEventType::INJECTION_ATTEMPT`.
    2. In your test harness, report an `INJECTION_ATTEMPT` event.
    3. Immediately after, call `ReportNegativeFeedback(FeedbackType::KICKED_FROM_GAME)`.
    4. Restart the application (or re-initialize the singleton) and log the new risk weight for `INJECTION_ATTEMPT`.
    5. **Expected Result:** The new risk weight should be higher than the initial weight (e.g., increased by the 20% learning rate).

### 3.5. Network Traffic Mimicry (`NetworkManager`)
- **Objective:** Verify that the `NetworkManager` can profile and shape traffic.
- **Scenario 1 (Profiling):**
    1. Set `NetworkManager::SetMode(NetworkMode::PROFILING)`.
    2. Send several packets of varying sizes and intervals.
    3. Add a temporary "dump profile" function to log the contents of the `m_profile` struct.
    4. **Expected Result:** The logged `avgPacketSize` and `avgPacketIntervalMs` should be a reasonable average of the traffic sent.
- **Scenario 2 (Mimicking):**
    1. Use a packet sniffer like Wireshark to monitor traffic from the application.
    2. Set `NetworkManager::SetMode(NetworkMode::MIMICKING)`.
    3. Call `send()` multiple times with small amounts of data in quick succession.
    4. **Expected Result:** Wireshark should not show a 1:1 correspondence of small packets. It should show fewer, larger packets being sent at intervals that roughly match the learned profile, demonstrating that the internal buffer is working.
