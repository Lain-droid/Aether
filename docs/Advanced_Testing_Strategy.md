# AetherVisor: Advanced Testing Strategy for Adaptive Systems
**Version 3.0 ("Perfection" Release Candidate)**

This document outlines the testing strategy for all systems within the AetherVisor project, including the final "Perfection" phase features.

## 1. Testing Philosophy

The system is a complex, non-deterministic, adaptive, and deeply obfuscated application. Testing requires a multi-faceted approach focusing on behavioral verification, state inspection, component isolation, and now, foundational architecture validation.

## 2. Test Scenarios - Phase 1 & 2 Features (Recap)

- **AI & Adaptation:** Risk accrual, conditional logic, and AI learning are tested via state inspection and behavioral checks.
- **Polymorphism & Evasion:** String encryption, code mutation, signature scanning, and trampoline hooks are tested via binary analysis, logging, and stability checks.
- **Network Mimicry:** Tested via packet sniffing and validating the buffering/shaping logic.

## 3. Test Scenarios - Phase 3 ("Perfection") Foundational Features

### 3.1. Steganography Layer (`IPC` & `StegoPacket`)
- **Objective:** Verify that IPC communication is successfully hidden within a fake BMP structure.
- **Scenario:**
    1. Use a named pipe monitoring tool (or add extensive logging to the IPC server loop) to capture the raw byte stream being sent from frontend to backend.
    2. **Expected Result 1:** The byte stream should start with the BMP magic bytes (`0x42 0x4D`). The structure should match the `FakeBMPFileHeader` and `FakeBMPInfoHeader`.
    3. **Expected Result 2:** On the receiving end, the extracted `IpcMessage` (after deserialization and unpacking) should be identical to the original message sent. This verifies the round-trip integrity.

### 3.2. Code Virtualization Layer (`VirtualMachine`)
- **Objective:** Verify that the VM skeleton can correctly execute a simple bytecode program.
- **Scenario:**
    1. Create a test function that constructs a simple bytecode `std::vector<uint8_t>`. Program: `PUSH_INT 5`, `PUSH_INT 10`, `ADD`, `HALT`.
    2. Create an instance of the `VirtualMachine`.
    3. Execute the bytecode using `vm.Run(test_program)`.
    4. After execution, inspect the VM's stack.
    5. **Expected Result:** The top value on the stack should be the integer `15`. This confirms the VM can correctly process opcodes and manipulate its stack.

### 3.3. Behavioral Cloning Layer (`BehavioralCloner` & `MLPrimitives`)
- **Objective:** Verify the structural integrity and data flow of the neural network skeleton.
- **Scenario:**
    1. Create an instance of the `BehavioralCloner`.
    2. Create a sample `GameState` input struct.
    3. Call `behavioralCloner.GenerateMouseMovement(sample_state)`.
    4. **Expected Result:** The function should execute without crashing and return a `std::pair<double, double>`. The key is to verify that the input `Matrix` is correctly propagated through the `Layer` objects and that the dimensions of the matrices are compatible at each step. A crash would indicate a flaw in the matrix multiplication or layer logic. This is a "smoke test" for the architecture.
