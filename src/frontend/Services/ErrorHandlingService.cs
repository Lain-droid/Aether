using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging;

namespace AetherVisor.Frontend.Services
{
    public enum ErrorSeverity
    {
        Info,
        Warning,
        Error,
        Critical
    }

    public enum ErrorCategory
    {
        Network,
        FileSystem,
        IPC,
        ScriptExecution,
        Security,
        UI,
        Configuration,
        Unknown
    }

    public class ErrorInfo
    {
        public string Message { get; set; }
        public string Details { get; set; }
        public ErrorSeverity Severity { get; set; }
        public ErrorCategory Category { get; set; }
        public DateTime Timestamp { get; set; }
        public Exception Exception { get; set; }
        public string UserAction { get; set; }
        public bool IsRecoverable { get; set; }
    }

    public interface IErrorHandlingService
    {
        void HandleError(Exception exception, ErrorCategory category, string context = null);
        void HandleError(string message, ErrorSeverity severity, ErrorCategory category, string details = null);
        Task<bool> TryRecoverAsync(ErrorInfo error);
        List<ErrorInfo> GetRecentErrors(int count = 10);
        void ClearErrors();
        event Action<ErrorInfo> ErrorOccurred;
    }

    public class ErrorHandlingService : IErrorHandlingService
    {
        private readonly ILogger<ErrorHandlingService> _logger;
        private readonly List<ErrorInfo> _errorHistory;
        private readonly object _lockObject = new object();

        public event Action<ErrorInfo> ErrorOccurred;

        public ErrorHandlingService(ILogger<ErrorHandlingService> logger)
        {
            _logger = logger;
            _errorHistory = new List<ErrorInfo>();
        }

        public void HandleError(Exception exception, ErrorCategory category, string? context = null)
        {
            var errorInfo = new ErrorInfo
            {
                Message = exception.Message,
                Details = exception.ToString(),
                Severity = DetermineSeverity(exception),
                Category = category,
                Timestamp = DateTime.UtcNow,
                Exception = exception,
                UserAction = GetUserAction(category, exception),
                IsRecoverable = IsRecoverableError(exception, category)
            };

            HandleErrorInternal(errorInfo, context);
        }

        public void HandleError(string message, ErrorSeverity severity, ErrorCategory category, string? details = null)
        {
            var errorInfo = new ErrorInfo
            {
                Message = message,
                Details = details,
                Severity = severity,
                Category = category,
                Timestamp = DateTime.UtcNow,
                UserAction = GetUserAction(category, null),
                IsRecoverable = IsRecoverableError(null, category)
            };

            HandleErrorInternal(errorInfo);
        }

        private void HandleErrorInternal(ErrorInfo errorInfo, string? context = null)
        {
            // Log the error
            var logMessage = $"[{errorInfo.Category}] {errorInfo.Message}";
            if (!string.IsNullOrEmpty(context))
            {
                logMessage = $"[{context}] {logMessage}";
            }

            switch (errorInfo.Severity)
            {
                case ErrorSeverity.Info:
                    _logger.LogInformation(logMessage);
                    break;
                case ErrorSeverity.Warning:
                    _logger.LogWarning(logMessage);
                    break;
                case ErrorSeverity.Error:
                    _logger.LogError(errorInfo.Exception, logMessage);
                    break;
                case ErrorSeverity.Critical:
                    _logger.LogCritical(errorInfo.Exception, logMessage);
                    break;
            }

            // Add to history
            lock (_lockObject)
            {
                _errorHistory.Add(errorInfo);
                if (_errorHistory.Count > 100) // Keep only last 100 errors
                {
                    _errorHistory.RemoveAt(0);
                }
            }

            // Notify subscribers
            ErrorOccurred?.Invoke(errorInfo);

            // Show user notification for critical errors
            if (errorInfo.Severity == ErrorSeverity.Critical)
            {
                ShowCriticalErrorNotification(errorInfo);
            }
        }

        public async Task<bool> TryRecoverAsync(ErrorInfo error)
        {
            if (!error.IsRecoverable)
            {
                return false;
            }

            try
            {
                switch (error.Category)
                {
                    case ErrorCategory.Network:
                        return await TryRecoverNetworkErrorAsync(error);
                    case ErrorCategory.IPC:
                        return await TryRecoverIPCErrorAsync(error);
                    case ErrorCategory.FileSystem:
                        return await TryRecoverFileSystemErrorAsync(error);
                    case ErrorCategory.ScriptExecution:
                        return await TryRecoverScriptExecutionErrorAsync(error);
                    default:
                        return false;
                }
            }
            catch (Exception recoveryException)
            {
                _logger.LogError(recoveryException, "Error during recovery attempt");
                return false;
            }
        }

        public List<ErrorInfo> GetRecentErrors(int count = 10)
        {
            lock (_lockObject)
            {
                var startIndex = Math.Max(0, _errorHistory.Count - count);
                return _errorHistory.GetRange(startIndex, _errorHistory.Count - startIndex);
            }
        }

        public void ClearErrors()
        {
            lock (_lockObject)
            {
                _errorHistory.Clear();
            }
        }

        private ErrorSeverity DetermineSeverity(Exception exception)
        {
            if (exception is UnauthorizedAccessException || exception is SecurityException)
            {
                return ErrorSeverity.Critical;
            }
            else if (exception is TimeoutException || exception is TaskCanceledException)
            {
                return ErrorSeverity.Warning;
            }
            else if (exception is ArgumentException || exception is InvalidOperationException)
            {
                return ErrorSeverity.Error;
            }
            else
            {
                return ErrorSeverity.Error;
            }
        }

        private string GetUserAction(ErrorCategory category, Exception exception)
        {
            return category switch
            {
                ErrorCategory.Network => "Check your internet connection and try again",
                ErrorCategory.IPC => "Restart the application",
                ErrorCategory.FileSystem => "Check file permissions and disk space",
                ErrorCategory.ScriptExecution => "Review your script for syntax errors",
                ErrorCategory.Security => "Contact support if this persists",
                ErrorCategory.UI => "Try refreshing the interface",
                ErrorCategory.Configuration => "Reset to default settings",
                _ => "Try again or contact support"
            };
        }

        private bool IsRecoverableError(Exception exception, ErrorCategory category)
        {
            if (exception is UnauthorizedAccessException || exception is SecurityException)
            {
                return false;
            }

            return category switch
            {
                ErrorCategory.Network => true,
                ErrorCategory.IPC => true,
                ErrorCategory.FileSystem => true,
                ErrorCategory.ScriptExecution => true,
                ErrorCategory.UI => true,
                ErrorCategory.Configuration => true,
                _ => false
            };
        }

        private async Task<bool> TryRecoverNetworkErrorAsync(ErrorInfo error)
        {
            // Implement network recovery logic
            await Task.Delay(1000); // Simulate retry delay
            return true;
        }

        private async Task<bool> TryRecoverIPCErrorAsync(ErrorInfo error)
        {
            // Implement IPC recovery logic
            await Task.Delay(500);
            return true;
        }

        private async Task<bool> TryRecoverFileSystemErrorAsync(ErrorInfo error)
        {
            // Implement file system recovery logic
            await Task.Delay(200);
            return true;
        }

        private async Task<bool> TryRecoverScriptExecutionErrorAsync(ErrorInfo error)
        {
            // Implement script execution recovery logic
            await Task.Delay(100);
            return true;
        }

        private void ShowCriticalErrorNotification(ErrorInfo errorInfo)
        {
            // In a real implementation, this would show a modal dialog
            // For now, we'll just log it
            _logger.LogCritical("CRITICAL ERROR: {Message}. User action: {UserAction}", 
                errorInfo.Message, errorInfo.UserAction);
        }
    }
}