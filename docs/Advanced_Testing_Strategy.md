# AetherVisor: Advanced Testing Strategy for Adaptive Systems
**Version 5.0 ("Fully Detailed" Release Candidate)**

This document outlines the testing strategy for all systems within the AetherVisor project, updated to cover the final "Focused Detailing" phase of development where architectural skeletons were fleshed out into functional components.

## 1. Testing Philosophy

The system is a complex, non-deterministic, adaptive, and deeply obfuscated application. Testing requires a multi-faceted approach focusing on behavioral verification, state inspection, component isolation, and functional correctness of the core algorithms.

## 2. Test Scenarios - Previous Phases (Recap)

- AI, Polymorphism, Network Mimicry, Signature Scanning, Trampoline Hooks, Steganography, and foundational VM/ML skeletons have been tested per previous versions of this document.

## 3. Test Scenarios - Phase 5 ("Focused Detailing") Functional Features

### 3.1. Functional VM & Compiler
- **Objective:** Verify that the compiler correctly translates complex expressions and the VM executes them, including control flow.
- **Scenario 1 (Compiler Correctness):**
    1. Create an instance of the `Compiler` and `VirtualMachine`.
    2. Call `compiler.Compile("10 * (5 - 2)")`. The expected RPN is `10 5 2 - *`. The expected bytecode is `PUSH 10, PUSH 5, PUSH 2, SUB, MUL, HALT`.
    3. Pass the resulting bytecode to `vm.Run()`.
    4. **Expected Result:** The top value on the VM's stack should be `30`.
- **Scenario 2 (VM Control Flow):**
    1. Manually construct bytecode for a program like: `PUSH 0, JMP_IF_ZERO <offset_to_end>, PUSH 10, HALT, <end_label>: PUSH 20, HALT`.
    2. Run this on the VM.
    3. **Expected Result:** The final value on the stack should be `20`, proving the jump was taken.
    4. Repeat with `PUSH 1` at the start. The final value should be `10`, proving the jump was not taken.
- **Scenario 3 (Native Call):**
    1. Register a simple C++ lambda `[] { ... }` with the VM using `RegisterNativeFunction("test_func")`.
    2. Compile and run bytecode for `CALL_NATIVE "test_func"`.
    3. **Expected Result:** The C++ lambda should be executed.

### 3.2. Functional Behavioral Cloner (`Layer` & `BehavioralCloner`)
- **Objective:** Verify that the `Train` method correctly modifies the network's weights.
- **Scenario:**
    1. Create a `BehavioralCloner` instance.
    2. Store a copy of the initial weights of the first layer.
    3. Create a sample `GameState` input and a corresponding `expected_output` matrix.
    4. Call `behavioralCloner.Train(...)` for a single epoch.
    5. Get the new weights of the first layer.
    6. **Expected Result:** The new weights should be different from the initial weights. This confirms that the backpropagation algorithm is correctly calculating gradients and updating the weights, demonstrating that the learning mechanism is functional.
