# AetherVisor: Advanced Testing Strategy for Adaptive Systems
**Version 6.0 ("Frontend" Release Candidate)**

This document outlines the testing strategy for all systems within the AetherVisor project, updated to cover the new C# Frontend ViewModels and the refactored user-mode backend.

## 1. Testing Philosophy

Testing now covers three distinct areas: the C++ backend/payload, the C# frontend logic (ViewModels), and the IPC that connects them. Frontend testing focuses on state management and command logic in response to user actions and backend events.

## 2. Test Scenarios - Backend (Recap)

- All backend features (AI, Polymorphism, VM, etc.) are tested as per previous versions of this document.
- **New Backend Test:**
    - **Script Analyzer:** Send a script containing "os.execute" to the backend's analysis service. Verify the response is "UNSAFE". Send a script with only a `print("hello")` command. Verify the response is "SAFE".
    - **User-Mode Injection:** Verify that the `Core::Inject` method successfully injects the payload DLL using only user-mode APIs.

## 3. Test Scenarios - Frontend (New)

### 3.1. `SetupViewModel.cs`
- **Objective:** Verify the logic for prerequisite checks.
- **Scenario:**
    1. Run the application with standard user privileges. Instantiate `SetupViewModel` and call `CheckRequirements()`.
    2. **Expected Result:** The `IsAdmin` property should be `false` and the `StatusMessage` should indicate that admin rights are required.
    3. Run the application with administrator privileges. Call `CheckRequirements()`.
    4. **Expected Result:** The `IsAdmin` property should be `true` and the `StatusMessage` should be positive.

### 3.2. `SettingsViewModel.cs`
- **Objective:** Verify that settings can be changed and the save command works.
- **Scenario:**
    1. Instantiate `SettingsViewModel`.
    2. Change the `SelectedTheme` property from `Dark` to `Light`.
    3. Call the `SaveSettingsCommand`.
    4. **Expected Result:** The console (or a log file) should show a "Settings saved!" message with the new "Light" theme value, confirming the command logic is wired correctly.

### 3.3. `MainViewModel.cs` & Unsafe Script Warning
- **Objective:** Verify the entire script execution and warning workflow.
- **Scenario:**
    1. Instantiate `MainViewModel` with a mock `IpcClientService`.
    2. Set `IsInjected = true`.
    3. Set the `ScriptText` to a known "unsafe" script.
    4. Call `ExecuteScriptCommand`.
    5. **Expected Result 1:** The `AnalyzeScript` method on the mock service should be called.
    6. Simulate the service firing the `OnAnalysisResultReceived` event with a payload of "UNSAFE".
    7. **Expected Result 2:** The `MainViewModel.IsWarningVisible` property should now be `true`.
    8. Call the `ExecuteAnywayCommand`.
    9. **Expected Result 3:** The `ExecuteScript` method on the mock service should now be called, and `IsWarningVisible` should become `false`.
