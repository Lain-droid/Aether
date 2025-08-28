# Aether API Documentation

## Overview

Aether is a user-mode Luau scripting environment with a C++ backend and C# WPF frontend communicating via IPC.

## Backend API (C++)

### Core Components

#### Core Class
```cpp
namespace AetherVisor::Backend {
    class Core {
    public:
        static Core& GetInstance();
        bool Initialize();
        bool Inject(const std::wstring& processName);
        void Cleanup();
        bool ExecuteScript(const std::string& script);
    };
}
```

#### AIController
```cpp
namespace AetherVisor::Backend {
    class AIController {
    public:
        static AIController& GetInstance();
        RiskLevel GetCurrentRiskLevel();
        bool ShouldPerformAction(RiskLevel level);
        void ReportEvent(AIEventType eventType);
    };
}
```

#### PolymorphicEngine
```cpp
namespace AetherVisor::Backend {
    class PolymorphicEngine {
    public:
        static PolymorphicEngine& GetInstance();
        void Mutate(std::vector<unsigned char>& payload);
        std::vector<unsigned char> MetamorphicGeneration(
            const std::vector<unsigned char>& payload, 
            int metamorphic_level);
    };
}
```

### IPC Protocol

#### Message Format
```
[Length: uint32][Operation: uint8][Payload: bytes]
```

#### Operations
- `1`: Inject
- `2`: Execute Script
- `3`: Configuration
- `4`: Bypass

#### Example Usage
```cpp
// Send script for execution
std::string script = "print('Hello from Aether')";
core.ExecuteScript(script);
```

## Frontend API (C#)

### Services

#### IpcClientService
```csharp
public interface IIpcClientService
{
    event Action<string> ConsoleMessageReceived;
    Task ConnectAsync();
    Task SendScriptAsync(string script);
    void Disconnect();
}
```

#### LuauIntelliSense
```csharp
public class LuauIntelliSense
{
    public List<CompletionItem> GetCompletions(string text, int position);
    public List<Diagnostic> GetDiagnostics(string text);
    public string GetHoverInfo(string text, int position);
}
```

#### AnalyticsService
```csharp
public class AnalyticsService
{
    public void TrackEvent(string eventName, Dictionary<string, object> properties);
    public void TrackError(Exception exception, string context);
    public AnalyticsReport GenerateReport(DateTime start, DateTime end);
}
```

### ViewModels

#### MainWindowViewModel
```csharp
public class MainWindowViewModel : INotifyPropertyChanged
{
    public ICommand InjectCommand { get; }
    public ICommand ExecuteScriptCommand { get; }
    public ObservableCollection<ScriptTab> Tabs { get; }
    public ScriptTab ActiveTab { get; set; }
}
```

#### ScriptEditorViewModel
```csharp
public class ScriptEditorViewModel : INotifyPropertyChanged
{
    public string ScriptText { get; set; }
    public List<CompletionItem> Completions { get; }
    public List<Diagnostic> Diagnostics { get; }
    public ICommand ExecuteCommand { get; }
}
```

## Configuration

### Backend Configuration
```cpp
struct BackendConfig {
    bool enable_ai_controller = true;
    bool enable_polymorphic_engine = true;
    bool enable_security_hardening = true;
    std::string log_level = "info";
    std::string ipc_pipe_name = "AetherPipe";
};
```

### Frontend Configuration
```csharp
public class AppSettings
{
    public string EditorTheme { get; set; } = "Dark";
    public string EditorFontFamily { get; set; } = "Consolas";
    public int EditorFontSize { get; set; } = 12;
    public bool EnableIntelliSense { get; set; } = true;
    public bool EnableAutoComplete { get; set; } = true;
}
```

## Error Handling

### Backend Errors
```cpp
enum class ErrorCode {
    SUCCESS = 0,
    INJECTION_FAILED = 1,
    SCRIPT_EXECUTION_FAILED = 2,
    IPC_CONNECTION_FAILED = 3,
    SECURITY_VIOLATION = 4
};

struct ErrorInfo {
    ErrorCode code;
    std::string message;
    std::string details;
};
```

### Frontend Errors
```csharp
public class AetherException : Exception
{
    public ErrorCode Code { get; }
    public string Details { get; }
    
    public AetherException(ErrorCode code, string message, string details = null)
        : base(message)
    {
        Code = code;
        Details = details;
    }
}
```

## Security Features

### Anti-Detection
- Process injection detection evasion
- Memory pattern obfuscation
- Timing attack prevention
- Debugger detection

### Code Protection
- Polymorphic code generation
- Metamorphic transformations
- Anti-analysis techniques
- Signature masking

## Performance Monitoring

### Metrics
- Script execution time
- Memory usage
- IPC latency
- Error rates
- User interaction patterns

### Logging
```cpp
// Backend logging
spdlog::info("Script executed successfully");
spdlog::warn("Security warning: {}", warning);
spdlog::error("Error occurred: {}", error);

// Frontend logging
_logger.LogInformation("User executed script");
_logger.LogWarning("Performance degradation detected");
_logger.LogError("Connection failed", exception);
```

## Testing

### Backend Tests
```cpp
TEST_CASE("Core initialization", "[core]") {
    auto& core = Core::GetInstance();
    REQUIRE(core.Initialize() == true);
}

TEST_CASE("Script execution", "[core]") {
    auto& core = Core::GetInstance();
    REQUIRE(core.ExecuteScript("print('test')") == true);
}
```

### Frontend Tests
```csharp
[Test]
public void ScriptExecution_ValidScript_ReturnsSuccess()
{
    var service = new IpcClientService();
    var result = await service.SendScriptAsync("print('test')");
    Assert.IsTrue(result);
}
```

## Deployment

### Build Artifacts
- `Aether.exe` - Main frontend application
- `aether_backend.dll` - Backend library
- `*.dll` - Dependency libraries

### Installation
1. Extract all files to a directory
2. Run `Aether.exe` as Administrator
3. Accept security warnings
4. Use the application

### Updates
- Automatic update checking
- Incremental downloads
- Rollback capability
- Version compatibility checks