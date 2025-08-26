# AetherVisor: Advanced Testing Strategy for Adaptive Systems

This document outlines the testing strategy for the new, dynamic, and AI-driven features added to the AetherVisor project. It complements the existing `Security_And_Testing.md` with scenarios specifically designed for these advanced capabilities.

## 1. Testing Philosophy

The new systems are non-deterministic by design. Testing them requires a different approach than traditional unit testing. The focus should be on:
- **Behavioral Verification:** Does the system's behavior change correctly in response to stimuli (like AI risk level)?
- **State Inspection:** Can we query the state of the system's components (e.g., the AI's risk score) to verify they are correct?
- **Component Isolation:** Can we test individual components (like the `MemoryPatcher`) by manually setting the conditions they depend on (e.g., forcing a high risk level)?

## 2. Test Scenarios per Module

### 2.1. AIController
- **Objective:** Verify that the risk score changes correctly based on events.
- **Scenario 1 (Risk Accrual):**
    1. Start with a risk score of 0 (`RiskLevel::NONE`).
    2. Sequentially call `ReportEvent` with various `AIEventType`s (e.g., `HOOK_CALLED`, `MEMORY_PATCH_APPLIED`, `SUSPICIOUS_API_CALL`).
    3. After each call, query `GetCurrentRiskLevel()` and verify that the risk level increases as expected (e.g., from NONE to LOW, then to MEDIUM, etc.).
- **Scenario 2 (Risk Decay):**
    1. Bring the risk score to a HIGH level.
    2. Repeatedly call `ShouldPerformAction()` without reporting any new events.
    3. Verify that the internal `m_riskScore` gradually decreases and that the `RiskLevel` eventually drops back to MEDIUM, then LOW.

### 2.2. Execution Layer (`Core` & `PolymorphicEngine`)
- **Objective:** Verify that injection is conditional and the payload is polymorphic.
- **Scenario 1 (Conditional Injection):**
    1. Manually set the risk level in `AIController` to `LOW`. Call `Core::Inject()`. Verify (via logging or debugging) that the `LeverageCallback` injection method is chosen.
    2. Set risk to `MEDIUM`. Call `Inject()`. Verify `HijackThread` is chosen.
    3. Set risk to `HIGH`. Call `Inject()`. Verify `QueueUserApc` is chosen.
- **Scenario 2 (Polymorphic Payload):**
    1. Load the payload DLL into a buffer.
    2. Calculate its SHA-256 hash.
    3. Pass the buffer to `PolymorphicEngine::Mutate()`.
    4. Recalculate the hash. Verify that the new hash is **different** from the original.
    5. Repeat multiple times and confirm the hash is different each time.

### 2.3. Memory Layer (`MemoryPatcher` & `EphemeralMemory`)
- **Objective:** Verify conditional patching and secure ephemeral memory management.
- **Scenario 1 (Conditional Patching):**
    1. Define a target memory region.
    2. Set AI risk to `HIGH`. Call `MemoryPatcher::ApplyPatchConditionally()` with `requiredLevel = RiskLevel::MEDIUM`.
    3. Verify that the patch was **not** applied.
    4. Set AI risk to `LOW`. Call the same function.
    5. Verify that the patch **was** applied.
    6. Call `RevertPatch()` and verify the original memory is restored.
- **Scenario 2 (Ephemeral Memory):**
    1. Create an `EphemeralMemory` object.
    2. Write known data to it using `Write()`.
    3. Use a debugger or memory scanner to find the allocated block and confirm the data is present.
    4. Let the `EphemeralMemory` object go out of scope, triggering its destructor.
    5. Verify that the memory region has been zeroed-out and deallocated.

### 2.4. Network Layer (`NetworkManager`)
- **Objective:** Verify that network calls are intercepted.
- **Scenario:**
    1. Install the `NetworkManager` hooks.
    2. In the target process, make a call to `send()` or `recv()`.
    3. Verify (via logging or debugging) that the `Detour_Send` or `Detour_Recv` function is executed.
    4. Check the `AIController`'s risk score to ensure it increased slightly due to the network event report.

### 2.5. Cleanup Layer (`Core::Cleanup`)
- **Objective:** Verify that the cleanup process is randomized.
- **Scenario:**
    1. Add detailed logging to the beginning of each cleanup task lambda in `Core::Cleanup()`.
    2. Run the program and call `Cleanup()`. Record the order of the log messages.
    3. Restart the program and call `Cleanup()` again.
    4. Verify that the order of the log messages is **different** from the first run. Repeat several times to confirm randomness.
