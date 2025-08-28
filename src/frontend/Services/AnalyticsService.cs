using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading.Tasks;
using AetherVisor.Frontend.Models;

namespace AetherVisor.Frontend.Services
{
    public class AnalyticsService
    {
        private readonly string _analyticsDirectory;
        private readonly string _sessionFile;
        private readonly Queue<AnalyticsEvent> _eventQueue;
        private readonly object _lockObject = new();
        
        private SessionData _currentSession;
        private bool _isEnabled;
        private DateTime _sessionStartTime;
        
        public AnalyticsService()
        {
            _analyticsDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Aether", "Analytics");
            
            Directory.CreateDirectory(_analyticsDirectory);
            
            _sessionFile = Path.Combine(_analyticsDirectory, "current_session.json");
            _eventQueue = new Queue<AnalyticsEvent>();
            _isEnabled = true;
            
            InitializeSession();
        }
        
        #region Public Methods
        
        public void TrackEvent(string eventName, Dictionary<string, object> properties = null)
        {
            if (!_isEnabled) return;
            
            var analyticsEvent = new AnalyticsEvent
            {
                EventName = eventName,
                Timestamp = DateTime.UtcNow,
                Properties = properties ?? new Dictionary<string, object>(),
                SessionId = _currentSession.SessionId,
                UserId = _currentSession.UserId
            };
            
            lock (_lockObject)
            {
                _eventQueue.Enqueue(analyticsEvent);
                _currentSession.EventCount++;
            }
            
            // Async save to avoid blocking UI
            Task.Run(SaveEventsToFile);
        }
        
        public void TrackFeatureUsage(string featureName, TimeSpan duration = default)
        {
            var properties = new Dictionary<string, object>
            {
                ["feature"] = featureName,
                ["duration_ms"] = duration.TotalMilliseconds
            };
            
            TrackEvent("feature_used", properties);
        }
        
        public void TrackScriptExecution(string scriptType, TimeSpan executionTime, bool success, string errorMessage = null)
        {
            var properties = new Dictionary<string, object>
            {
                ["script_type"] = scriptType,
                ["execution_time_ms"] = executionTime.TotalMilliseconds,
                ["success"] = success,
                ["lines_count"] = GetScriptLinesCount()
            };
            
            if (!success && !string.IsNullOrEmpty(errorMessage))
            {
                properties["error_message"] = errorMessage;
            }
            
            TrackEvent("script_executed", properties);
        }
        
        public void TrackUserInteraction(string elementName, string action)
        {
            var properties = new Dictionary<string, object>
            {
                ["element"] = elementName,
                ["action"] = action
            };
            
            TrackEvent("user_interaction", properties);
        }
        
        public void TrackPerformanceMetric(string metricName, double value, string unit = null)
        {
            var properties = new Dictionary<string, object>
            {
                ["metric"] = metricName,
                ["value"] = value
            };
            
            if (!string.IsNullOrEmpty(unit))
            {
                properties["unit"] = unit;
            }
            
            TrackEvent("performance_metric", properties);
        }
        
        public void TrackError(Exception exception, string context = null)
        {
            var properties = new Dictionary<string, object>
            {
                ["exception_type"] = exception.GetType().Name,
                ["message"] = exception.Message,
                ["stack_trace"] = exception.StackTrace
            };
            
            if (!string.IsNullOrEmpty(context))
            {
                properties["context"] = context;
            }
            
            TrackEvent("error_occurred", properties);
        }
        
        public void StartSession(string userId = null)
        {
            _sessionStartTime = DateTime.UtcNow;
            _currentSession = new SessionData
            {
                SessionId = Guid.NewGuid().ToString(),
                UserId = userId ?? Environment.UserName,
                StartTime = _sessionStartTime,
                Platform = Environment.OSVersion.ToString(),
                AppVersion = GetAppVersion(),
                EventCount = 0
            };
            
            TrackEvent("session_started", new Dictionary<string, object>
            {
                ["platform"] = _currentSession.Platform,
                ["app_version"] = _currentSession.AppVersion
            });
        }
        
        public void EndSession()
        {
            if (_currentSession == null) return;
            
            _currentSession.EndTime = DateTime.UtcNow;
            _currentSession.Duration = _currentSession.EndTime.Value - _currentSession.StartTime;
            
            TrackEvent("session_ended", new Dictionary<string, object>
            {
                ["duration_minutes"] = _currentSession.Duration.TotalMinutes,
                ["event_count"] = _currentSession.EventCount
            });
            
            SaveSessionData();
        }
        
        public async Task<AnalyticsReport> GenerateReportAsync(DateTime startDate, DateTime endDate)
        {
            var events = await LoadEventsAsync(startDate, endDate);
            var sessions = await LoadSessionsAsync(startDate, endDate);
            
            var report = new AnalyticsReport
            {
                StartDate = startDate,
                EndDate = endDate,
                GeneratedAt = DateTime.UtcNow,
                TotalEvents = events.Count,
                TotalSessions = sessions.Count
            };
            
            // Calculate metrics
            CalculateUsageMetrics(report, events, sessions);
            CalculateFeatureUsage(report, events);
            CalculatePerformanceMetrics(report, events);
            CalculateErrorMetrics(report, events);
            
            return report;
        }
        
        public async Task<List<AnalyticsEvent>> GetEventsAsync(DateTime startDate, DateTime endDate, string eventType = null)
        {
            var events = await LoadEventsAsync(startDate, endDate);
            
            if (!string.IsNullOrEmpty(eventType))
            {
                events = events.Where(e => e.EventName == eventType).ToList();
            }
            
            return events.OrderBy(e => e.Timestamp).ToList();
        }
        
        public void SetEnabled(bool enabled)
        {
            _isEnabled = enabled;
            
            if (enabled)
            {
                TrackEvent("analytics_enabled");
            }
        }
        
        public void ClearData()
        {
            lock (_lockObject)
            {
                _eventQueue.Clear();
            }
            
            // Clear all analytics files
            var files = Directory.GetFiles(_analyticsDirectory, "*.json");
            foreach (var file in files)
            {
                try
                {
                    File.Delete(file);
                }
                catch
                {
                    // Ignore errors when deleting
                }
            }
            
            TrackEvent("analytics_data_cleared");
        }
        
        public async Task ExportDataAsync(string filePath, DateTime startDate, DateTime endDate)
        {
            var report = await GenerateReportAsync(startDate, endDate);
            var events = await GetEventsAsync(startDate, endDate);
            var sessions = await LoadSessionsAsync(startDate, endDate);
            
            var exportData = new
            {
                Report = report,
                Events = events,
                Sessions = sessions,
                ExportedAt = DateTime.UtcNow
            };
            
            var json = JsonSerializer.Serialize(exportData, new JsonSerializerOptions
            {
                WriteIndented = true
            });
            
            await File.WriteAllTextAsync(filePath, json);
        }
        
        #endregion
        
        #region Private Methods
        
        private void InitializeSession()
        {
            StartSession();
        }
        
        private async void SaveEventsToFile()
        {
            if (_eventQueue.Count == 0) return;
            
            List<AnalyticsEvent> eventsToSave;
            lock (_lockObject)
            {
                eventsToSave = new List<AnalyticsEvent>(_eventQueue);
                _eventQueue.Clear();
            }
            
            var fileName = $"events_{DateTime.UtcNow:yyyy-MM-dd}.json";
            var filePath = Path.Combine(_analyticsDirectory, fileName);
            
            try
            {
                List<AnalyticsEvent> existingEvents = new();
                
                if (File.Exists(filePath))
                {
                    var existingJson = await File.ReadAllTextAsync(filePath);
                    existingEvents = JsonSerializer.Deserialize<List<AnalyticsEvent>>(existingJson) ?? new();
                }
                
                existingEvents.AddRange(eventsToSave);
                
                var json = JsonSerializer.Serialize(existingEvents, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                await File.WriteAllTextAsync(filePath, json);
            }
            catch (Exception ex)
            {
                // Log error: Debug statement removed
            }
        }
        
        private void SaveSessionData()
        {
            try
            {
                var json = JsonSerializer.Serialize(_currentSession, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                File.WriteAllText(_sessionFile, json);
                
                // Also save to historical sessions
                var sessionsFile = Path.Combine(_analyticsDirectory, $"sessions_{DateTime.UtcNow:yyyy-MM}.json");
                List<SessionData> sessions = new();
                
                if (File.Exists(sessionsFile))
                {
                    var existingJson = File.ReadAllText(sessionsFile);
                    sessions = JsonSerializer.Deserialize<List<SessionData>>(existingJson) ?? new();
                }
                
                sessions.Add(_currentSession);
                
                var sessionsJson = JsonSerializer.Serialize(sessions, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                File.WriteAllText(sessionsFile, sessionsJson);
            }
            catch (Exception ex)
            {
                // Log error: Debug statement removed
            }
        }
        
        private async Task<List<AnalyticsEvent>> LoadEventsAsync(DateTime startDate, DateTime endDate)
        {
            var allEvents = new List<AnalyticsEvent>();
            
            var currentDate = startDate.Date;
            while (currentDate <= endDate.Date)
            {
                var fileName = $"events_{currentDate:yyyy-MM-dd}.json";
                var filePath = Path.Combine(_analyticsDirectory, fileName);
                
                if (File.Exists(filePath))
                {
                    try
                    {
                        var json = await File.ReadAllTextAsync(filePath);
                        var events = JsonSerializer.Deserialize<List<AnalyticsEvent>>(json);
                        
                        if (events != null)
                        {
                            // Filter by time range
                            var filteredEvents = events.Where(e => 
                                e.Timestamp >= startDate && e.Timestamp <= endDate).ToList();
                            
                            allEvents.AddRange(filteredEvents);
                        }
                    }
                    catch (Exception ex)
                    {
                        // Log error: Debug statement removed
                    }
                }
                
                currentDate = currentDate.AddDays(1);
            }
            
            return allEvents;
        }
        
        private async Task<List<SessionData>> LoadSessionsAsync(DateTime startDate, DateTime endDate)
        {
            var allSessions = new List<SessionData>();
            
            var files = Directory.GetFiles(_analyticsDirectory, "sessions_*.json");
            
            foreach (var file in files)
            {
                try
                {
                    var json = await File.ReadAllTextAsync(file);
                    var sessions = JsonSerializer.Deserialize<List<SessionData>>(json);
                    
                    if (sessions != null)
                    {
                        var filteredSessions = sessions.Where(s =>
                            s.StartTime >= startDate && s.StartTime <= endDate).ToList();
                        
                        allSessions.AddRange(filteredSessions);
                    }
                }
                catch (Exception ex)
                {
                    // Log error: Debug statement removed
                }
            }
            
            return allSessions;
        }
        
        private void CalculateUsageMetrics(AnalyticsReport report, List<AnalyticsEvent> events, List<SessionData> sessions)
        {
            if (sessions.Any())
            {
                report.AverageSessionDuration = TimeSpan.FromMinutes(
                    sessions.Where(s => s.Duration.HasValue)
                           .Average(s => s.Duration.Value.TotalMinutes));
                
                report.UniqueUsers = sessions.Select(s => s.UserId).Distinct().Count();
            }
            
            // Calculate most used features
            var featureEvents = events.Where(e => e.EventName == "feature_used").ToList();
            report.MostUsedFeatures = featureEvents
                .GroupBy(e => e.Properties.ContainsKey("feature") ? e.Properties["feature"].ToString() : "Unknown")
                .OrderByDescending(g => g.Count())
                .Take(10)
                .ToDictionary(g => g.Key, g => g.Count());
        }
        
        private void CalculateFeatureUsage(AnalyticsReport report, List<AnalyticsEvent> events)
        {
            var scriptEvents = events.Where(e => e.EventName == "script_executed").ToList();
            report.ScriptExecutions = scriptEvents.Count;
            
            if (scriptEvents.Any())
            {
                var successfulExecutions = scriptEvents.Count(e => 
                    e.Properties.ContainsKey("success") && (bool)e.Properties["success"]);
                
                report.ScriptSuccessRate = (double)successfulExecutions / scriptEvents.Count * 100;
                
                var executionTimes = scriptEvents
                    .Where(e => e.Properties.ContainsKey("execution_time_ms"))
                    .Select(e => (double)e.Properties["execution_time_ms"])
                    .ToList();
                
                if (executionTimes.Any())
                {
                    report.AverageScriptExecutionTime = TimeSpan.FromMilliseconds(executionTimes.Average());
                }
            }
        }
        
        private void CalculatePerformanceMetrics(AnalyticsReport report, List<AnalyticsEvent> events)
        {
            var performanceEvents = events.Where(e => e.EventName == "performance_metric").ToList();
            
            report.PerformanceMetrics = new Dictionary<string, double>();
            
            foreach (var metricGroup in performanceEvents.GroupBy(e => 
                e.Properties.ContainsKey("metric") ? e.Properties["metric"].ToString() : "Unknown"))
            {
                var values = metricGroup
                    .Where(e => e.Properties.ContainsKey("value"))
                    .Select(e => Convert.ToDouble(e.Properties["value"]))
                    .ToList();
                
                if (values.Any())
                {
                    report.PerformanceMetrics[metricGroup.Key] = values.Average();
                }
            }
        }
        
        private void CalculateErrorMetrics(AnalyticsReport report, List<AnalyticsEvent> events)
        {
            var errorEvents = events.Where(e => e.EventName == "error_occurred").ToList();
            report.ErrorCount = errorEvents.Count;
            
            report.CommonErrors = errorEvents
                .GroupBy(e => e.Properties.ContainsKey("exception_type") ? 
                    e.Properties["exception_type"].ToString() : "Unknown")
                .OrderByDescending(g => g.Count())
                .Take(5)
                .ToDictionary(g => g.Key, g => g.Count());
        }
        
        private string GetAppVersion()
        {
            try
            {
                var assembly = System.Reflection.Assembly.GetExecutingAssembly();
                var version = assembly.GetName().Version;
                return version?.ToString() ?? "1.0.0.0";
            }
            catch
            {
                return "1.0.0.0";
            }
        }
        
        private int GetScriptLinesCount()
        {
            // This would be implemented to get the actual script line count
            // For now, return a placeholder
            return 0;
        }
        
        #endregion
    }
    
    #region Analytics Models
    
    public class AnalyticsEvent
    {
        public string EventName { get; set; }
        public DateTime Timestamp { get; set; }
        public Dictionary<string, object> Properties { get; set; } = new();
        public string SessionId { get; set; }
        public string UserId { get; set; }
    }
    
    public class SessionData
    {
        public string SessionId { get; set; }
        public string UserId { get; set; }
        public DateTime StartTime { get; set; }
        public DateTime? EndTime { get; set; }
        public TimeSpan? Duration { get; set; }
        public string Platform { get; set; }
        public string AppVersion { get; set; }
        public int EventCount { get; set; }
    }
    
    public class AnalyticsReport
    {
        public DateTime StartDate { get; set; }
        public DateTime EndDate { get; set; }
        public DateTime GeneratedAt { get; set; }
        
        public int TotalEvents { get; set; }
        public int TotalSessions { get; set; }
        public int UniqueUsers { get; set; }
        public TimeSpan AverageSessionDuration { get; set; }
        
        public Dictionary<string, int> MostUsedFeatures { get; set; } = new();
        
        public int ScriptExecutions { get; set; }
        public double ScriptSuccessRate { get; set; }
        public TimeSpan AverageScriptExecutionTime { get; set; }
        
        public Dictionary<string, double> PerformanceMetrics { get; set; } = new();
        
        public int ErrorCount { get; set; }
        public Dictionary<string, int> CommonErrors { get; set; } = new();
    }
    
    #endregion
}