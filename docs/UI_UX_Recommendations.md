# AetherVisor: UI/UX Recommendations

## 1. Design Philosophy

The user interface (UI) and user experience (UX) of AetherVisor should reflect its nature as a professional development and research tool. The design should be:

- **Clean and Modern:** A minimalist, clutter-free interface that is easy to navigate.
- **Informative:** Provide clear status updates and feedback to the user without being overly verbose.
- **Efficient:** The workflow for writing, executing, and debugging scripts should be as streamlined as possible.
- **Stable:** The UI must be responsive and bug-free, even when handling large amounts of console output or large scripts.

**Chosen Framework:** WPF (Windows Presentation Foundation)
- **Rationale:** WPF allows for the creation of rich, modern, and hardware-accelerated UIs. Its powerful data binding system is a perfect fit for the MVVM (Model-View-ViewModel) architecture, which separates UI logic from business logic, leading to a more maintainable and testable codebase.

## 2. UI Mockup & Components

### 2.1. Main Window Layout

The main window will be divided into three primary sections:

```
+----------------------------------------------------------------------+
| [AetherVisor v1.0] [Status: Injected to Roblox.exe (PID: 1234)] [Inject] | <- Top Bar
+----------------------------------------------------------------------+
|                                                                      |
|   // Advanced Script Editor Pane                                     |
|   // - Syntax Highlighting                                           |
|   // - Line Numbers                                                  |
|   // - Autocomplete                                                   |
|   // - Tabs for multiple scripts                                     |
|                                                                      |
|                                                                      |
|                                                                      |
+----------------------------------------------------------------------+
| [^] [Console Output] [Filter: All] [Clear]                           | <- Resizer & Console Bar
+----------------------------------------------------------------------+
| > print("Hello, world!")                                             |
| Hello, world!                                                        |
| > warn("This is a warning.")                                         |
| [Warning] This is a warning.                                         |
|                                                                      |
+----------------------------------------------------------------------+
```

### 2.2. Component Breakdown

1.  **Top Bar:**
    *   **Status Indicator:** A non-intrusive text block showing the current state: "Not Injected", "Injecting...", "Injection Failed: [Reason]", or "Injected to [Process Name] (PID: [ID])".
    *   **Inject/Eject Button:** A single button that dynamically changes its function. If not injected, it reads "Inject". If injected, it reads "Eject" and triggers the cleanup process.
    *   **File Menu (Optional):** A simple menu for `File -> Open Script`, `File -> Save Script`, `File -> Exit`.

2.  **Script Editor Pane:**
    *   This will be the largest component.
    *   **Implementation:** Use a high-quality third-party control like `AvalonEdit` to provide advanced features out-of-the-box, rather than building from scratch.
    *   **Features:** Must support Luau syntax highlighting, code folding, line numbering, and basic autocomplete for Roblox-specific globals (`game`, `workspace`, etc.).
    *   **Tabs:** Allow the user to have multiple script files open at once.

3.  **Console Output Pane:**
    *   A read-only, scrollable text area.
    *   **Implementation:** Use a `ListView` or a similar virtualized control to ensure high performance even with thousands of lines of output. Do not use a simple `TextBlock` or `TextBox`, as they will suffer from performance issues.
    *   **Features:**
        - **Color Coding:** Errors in red, warnings in yellow, regular prints in white/black.
        - **Filtering:** Buttons or a dropdown to show/hide different message types (All, Info, Warning, Error).
        - **Clear Button:** A button to clear the console log.
        - **Copy:** A right-click context menu to copy a single line or the entire log.

## 3. Error Handling and User Feedback

- **Modal Dialogs:** Use modal dialogs (pop-up windows) only for critical errors that require user confirmation (e.g., "Injection failed. Check logs for details.").
- **Status Bar Updates:** For non-critical errors or status changes, update the text in the top status bar. This is less intrusive.
- **Tooltips:** Provide helpful tooltips for UI elements, such as hovering over the "Inject" button to see more details about the target process.
- **Responsiveness:** All long-running operations (like injection) must be performed on a background thread to prevent the UI from freezing. The UI should show a "working" or "injecting..." state during these operations.
