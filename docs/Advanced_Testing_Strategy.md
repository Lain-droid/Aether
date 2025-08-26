# AetherVisor: Advanced Testing Strategy for Adaptive Systems
**Version 7.0 ("Fully Functional Skeletons" Release Candidate)**

This document outlines the testing strategy for all systems within the AetherVisor project, updated to cover the final "Completing the Skeletons" phase where advanced architectures were made functional.

## 1. Testing Philosophy

The system is a complex, non-deterministic, adaptive, and deeply obfuscated application. Testing requires a multi-faceted approach focusing on behavioral verification, state inspection, component isolation, and functional correctness of the core algorithms.

## 2. Test Scenarios - Previous Phases (Recap)

- All previously implemented features are tested as per prior versions of this document.

## 3. Test Scenarios - Phase 6 ("Completing the Skeletons") Functional Features

### 3.1. Advanced AI Controller
- **Objective:** Verify the new predictive and more intelligent learning capabilities.
- **Scenario 1 (Weighted Blame):**
    1. Add detailed logging to `ReportNegativeFeedback` to see the `current_blame_factor` and the resulting new risk weight for each event.
    2. Trigger a sequence of 3-4 risky events.
    3. Call `ReportNegativeFeedback`.
    4. **Expected Result:** The log should show that the most recent event received the highest risk multiplier, and the multiplier decreased for older events in the history.
- **Scenario 2 (Predictive Analysis):**
    1. Create a `std::vector` of `AIEventType`s (e.g., `{ INJECTION_ATTEMPT, MEMORY_PATCH_APPLIED }`).
    2. Call `AIController::AnalyzeActionSequence()` with this vector.
    3. **Expected Result:** The returned `double` should be the sum of the current risk weights for `INJECTION_ATTEMPT` and `MEMORY_PATCH_APPLIED`, confirming the predictive calculation is correct.

### 3.2. Functional Machine Learning Model
- **Objective:** Verify that the mathematically correct backpropagation loop successfully trains the network.
- **Scenario 1 (Loss Function):**
    1. Create two 1x2 `Matrix` objects, `predicted` with values `[0.8, 0.1]` and `actual` with values `[1.0, 0.0]`.
    2. Call `Matrix::mean_squared_error_derivative(predicted, actual)`.
    3. **Expected Result:** The resulting gradient matrix should be `[-0.2, 0.1]`. This verifies the loss calculation is correct.
- **Scenario 2 (Full Training Loop):**
    1. Instantiate `BehavioralCloner`.
    2. Store a copy of the initial weights of the first layer.
    3. Create a sample `GameState` input and a corresponding `expected_output` matrix.
    4. Call `behavioralCloner.Train(...)` for a single epoch with the new, correct backpropagation logic.
    5. Get the new weights of the first layer.
    6. **Expected Result:** The new weights must be different from the initial weights. This confirms that the gradient calculated from the loss function was successfully propagated backward through the layers and used to update the weights, proving the entire learning mechanism is functional.
