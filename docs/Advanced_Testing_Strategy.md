# AetherVisor: Advanced Testing Strategy for Adaptive Systems
**Version 12.0 ("Maximum Protection" Release Candidate)**

This document outlines the testing strategy for all systems within the AetherVisor project, updated to cover the final "Maximum Protection" hardening features.

## 1. Testing Philosophy

The system is a complex, non-deterministic, adaptive, and deeply obfuscated application. Testing requires a multi-faceted approach focusing on behavioral verification, state inspection, component isolation, and functional correctness of the core algorithms.

## 2. Test Scenarios - Previous Phases (Recap)

- All previously implemented features, including the architecturally correct command-line/log-file system, are tested as per prior versions of this document.

## 3. Test Scenarios - Phase 9 ("Maximum Protection") Features

### 3.1. Hardened Evasion Modules
- **Objective:** Verify the new, more comprehensive evasion checks work as intended.
- **Scenario 1 (Hardware Breakpoints):**
    1.  Launch the backend and set a hardware breakpoint on a function using a debugger.
    2.  Call `DebugEvader::IsDebugging()`.
    3.  **Expected Result:** The function should return `true`.
- **Scenario 2 (Forbidden Module):**
    1.  Use a test utility to load a DLL named "Scylla.dll" into our backend process.
    2.  Call `DebugEvader::IsDebugging()`.
    3.  **Expected Result:** The function should return `true`.
- **Scenario 3 (Suspicious MAC Address):**
    1.  On a VM, configure the network adapter's MAC address to start with `08:00:27` (a VirtualBox prefix).
    2.  Launch the backend and call `SandboxDetector::IsInSandbox()`.
    3.  **Expected Result:** The function should return `true`.
- **Scenario 4 (Suspicious Uptime):**
    1.  On a VM that has been running for less than 10 minutes, launch the backend.
    2.  Call `SandboxDetector::IsInSandbox()`.
    3.  **Expected Result:** The function should return `true`.

### 3.2. Hardened `ManualMapper`
- **Objective:** Verify the mapper can handle complex DLLs with TLS callbacks.
- **Scenario:**
    1.  Create a test DLL that uses Thread Local Storage (TLS) callbacks to write to a log file.
    2.  Inject this DLL using the `ManualMapper`.
    3.  **Expected Result:** The injection should succeed, and the log file should contain messages from the TLS callbacks, proving they were correctly located and executed.

### 3.3. Hardened Polymorphism (Function Encryption)
- **Objective:** Verify the conceptual framework for function encryption.
- **Scenario:**
    1.  This feature is a conceptual placeholder. A test would involve creating a payload with an exported function.
    2.  Run the `PolymorphicEngine`'s `EncryptFunction` on this payload.
    3.  Inspect the resulting byte buffer in a disassembler.
    4.  **Expected Result:** The original assembly for the function should be replaced with different bytes, and a conceptual "decryption stub" should be prepended.
