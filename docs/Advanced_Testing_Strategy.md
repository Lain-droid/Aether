# AetherVisor: Advanced Testing Strategy for Adaptive Systems
**Version 10.0 ("Architecturally Correct" Release Candidate)**

This document outlines the testing strategy for all systems within the AetherVisor project, updated to cover the final architectural fixes.

## 1. Testing Philosophy

The system is a complex, non-deterministic, adaptive, and deeply obfuscated application. Testing requires a multi-faceted approach focusing on behavioral verification, state inspection, component isolation, and now, end-to-end architectural flow validation.

## 2. Test Scenarios - Core Architectural Flow (NEW)

### 2.1. Security-Checked Startup
- **Objective:** Verify the backend checks for debuggers/sandboxes on startup and communicates the result.
- **Scenario:**
    1.  Launch the backend.
    2.  Launch the frontend and connect.
    3.  **Expected Result:** The `MainViewModel` should receive a `StartupResult` message of "OK" from the `IpcClientService` and update the status text accordingly. (A test with a debugger attached should result in an "UNSAFE_ENVIRONMENT" message).

### 2.2. End-to-End Injection Flow
- **Objective:** Verify that the UI "Inject" button correctly triggers the backend's injection logic.
- **Scenario:**
    1.  With both frontend and backend running and connected, click the "Inject" button on the UI.
    2.  Add a log message at the very beginning of the C++ `Core::Inject` method.
    3.  **Expected Result:** The log message in `Core::Inject` should appear, confirming that the `InjectRequest` IPC message was successfully sent by the frontend, received by the backend's IPC server, and routed to the correct `Core` function.

### 2.3. End-to-End Cleanup Flow
- **Objective:** Verify that the backend correctly signals the payload to self-destruct.
- **Scenario:**
    1.  After a successful injection, close the main frontend application, which should trigger the backend's `Core::Cleanup` method.
    2.  Add a log message at the very beginning of the `ShutdownPayload()` function in `payload/dllmain.cpp`.
    3.  **Expected Result:** The log message in `ShutdownPayload()` should appear. This confirms that the backend sent the `Shutdown` IPC message and that the payload's IPC client successfully received it and called the correct cleanup function.

## 3. Test Scenarios - All Other Features (Recap)

- All other features (AI, Polymorphism, VM, ML, Evasion Modules, etc.) are to be tested as per prior versions of this document. This section validates that the core application lifecycle that enables these features is now working correctly.
