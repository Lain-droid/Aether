using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Management;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;
using AetherVisor.Frontend.Models;

namespace AetherVisor.Frontend.Services
{
    public class PerformanceMonitor : INotifyPropertyChanged, IDisposable
    {
        private readonly Timer _monitoringTimer;
        private readonly PerformanceCounter _cpuCounter;
        private readonly PerformanceCounter _memoryCounter;
        private readonly PerformanceCounter _diskCounter;
        private readonly Process _currentProcess;
        
        private bool _isMonitoring;
        private int _monitoringInterval = 1000; // 1 second
        
        // Performance metrics
        private float _cpuUsage;
        private long _memoryUsage;
        private float _diskUsage;
        private long _networkSent;
        private long _networkReceived;
        
        // Application-specific metrics
        private int _scriptExecutions;
        private TimeSpan _totalExecutionTime;
        private int _activeConnections;
        private long _processedBytes;
        
        // Collections for historical data
        public ObservableCollection<PerformanceDataPoint> CpuHistory { get; }
        public ObservableCollection<PerformanceDataPoint> MemoryHistory { get; }
        public ObservableCollection<PerformanceDataPoint> DiskHistory { get; }
        public ObservableCollection<PerformanceDataPoint> NetworkHistory { get; }
        
        public ObservableCollection<ApplicationMetric> ApplicationMetrics { get; }
        public ObservableCollection<PerformanceAlert> Alerts { get; }
        
        // Configuration
        private readonly PerformanceConfiguration _config;
        
        public PerformanceMonitor()
        {
            _currentProcess = Process.GetCurrentProcess();
            
            // Initialize performance counters
            _cpuCounter = new PerformanceCounter("Processor", "% Processor Time", "_Total");
            _memoryCounter = new PerformanceCounter("Memory", "Available MBytes");
            _diskCounter = new PerformanceCounter("PhysicalDisk", "% Disk Time", "_Total");
            
            // Initialize collections
            CpuHistory = new ObservableCollection<PerformanceDataPoint>();
            MemoryHistory = new ObservableCollection<PerformanceDataPoint>();
            DiskHistory = new ObservableCollection<PerformanceDataPoint>();
            NetworkHistory = new ObservableCollection<PerformanceDataPoint>();
            ApplicationMetrics = new ObservableCollection<ApplicationMetric>();
            Alerts = new ObservableCollection<PerformanceAlert>();
            
            // Load configuration
            _config = LoadConfiguration();
            
            // Initialize timer
            _monitoringTimer = new Timer(CollectMetrics, null, Timeout.Infinite, Timeout.Infinite);
            
            InitializeApplicationMetrics();
        }
        
        #region Properties
        
        public bool IsMonitoring
        {
            get => _isMonitoring;
            private set
            {
                _isMonitoring = value;
                OnPropertyChanged(nameof(IsMonitoring));
            }
        }
        
        public float CpuUsage
        {
            get => _cpuUsage;
            private set
            {
                _cpuUsage = value;
                OnPropertyChanged(nameof(CpuUsage));
            }
        }
        
        public long MemoryUsage
        {
            get => _memoryUsage;
            private set
            {
                _memoryUsage = value;
                OnPropertyChanged(nameof(MemoryUsage));
            }
        }
        
        public float DiskUsage
        {
            get => _diskUsage;
            private set
            {
                _diskUsage = value;
                OnPropertyChanged(nameof(DiskUsage));
            }
        }
        
        public long NetworkSent
        {
            get => _networkSent;
            private set
            {
                _networkSent = value;
                OnPropertyChanged(nameof(NetworkSent));
            }
        }
        
        public long NetworkReceived
        {
            get => _networkReceived;
            private set
            {
                _networkReceived = value;
                OnPropertyChanged(nameof(NetworkReceived));
            }
        }
        
        public int ScriptExecutions
        {
            get => _scriptExecutions;
            private set
            {
                _scriptExecutions = value;
                OnPropertyChanged(nameof(ScriptExecutions));
            }
        }
        
        public TimeSpan TotalExecutionTime
        {
            get => _totalExecutionTime;
            private set
            {
                _totalExecutionTime = value;
                OnPropertyChanged(nameof(TotalExecutionTime));
            }
        }
        
        public int ActiveConnections
        {
            get => _activeConnections;
            private set
            {
                _activeConnections = value;
                OnPropertyChanged(nameof(ActiveConnections));
            }
        }
        
        public long ProcessedBytes
        {
            get => _processedBytes;
            private set
            {
                _processedBytes = value;
                OnPropertyChanged(nameof(ProcessedBytes));
            }
        }
        
        public string Status => IsMonitoring ? "Monitoring Active" : "Monitoring Stopped";
        
        #endregion
        
        #region Public Methods
        
        public void StartMonitoring()
        {
            if (IsMonitoring) return;
            
            try
            {
                _monitoringTimer.Change(0, _monitoringInterval);
                IsMonitoring = true;
                
                LogEvent("Performance monitoring started");
            }
            catch (Exception ex)
            {
                LogEvent($"Failed to start monitoring: {ex.Message}");
            }
        }
        
        public void StopMonitoring()
        {
            if (!IsMonitoring) return;
            
            try
            {
                _monitoringTimer.Change(Timeout.Infinite, Timeout.Infinite);
                IsMonitoring = false;
                
                LogEvent("Performance monitoring stopped");
            }
            catch (Exception ex)
            {
                LogEvent($"Failed to stop monitoring: {ex.Message}");
            }
        }
        
        public void SetMonitoringInterval(int intervalMs)
        {
            _monitoringInterval = Math.Max(500, intervalMs); // Minimum 500ms
            
            if (IsMonitoring)
            {
                _monitoringTimer.Change(0, _monitoringInterval);
            }
        }
        
        public PerformanceReport GenerateReport(TimeSpan timespan)
        {
            var endTime = DateTime.Now;
            var startTime = endTime - timespan;
            
            var report = new PerformanceReport
            {
                StartTime = startTime,
                EndTime = endTime,
                Duration = timespan,
                GeneratedAt = DateTime.Now
            };
            
            // CPU Statistics
            var cpuData = CpuHistory.Where(d => d.Timestamp >= startTime).ToList();
            if (cpuData.Any())
            {
                report.CpuStatistics = new PerformanceStatistics
                {
                    Average = cpuData.Average(d => d.Value),
                    Minimum = cpuData.Min(d => d.Value),
                    Maximum = cpuData.Max(d => d.Value),
                    Count = cpuData.Count
                };
            }
            
            // Memory Statistics
            var memoryData = MemoryHistory.Where(d => d.Timestamp >= startTime).ToList();
            if (memoryData.Any())
            {
                report.MemoryStatistics = new PerformanceStatistics
                {
                    Average = memoryData.Average(d => d.Value),
                    Minimum = memoryData.Min(d => d.Value),
                    Maximum = memoryData.Max(d => d.Value),
                    Count = memoryData.Count
                };
            }
            
            // Disk Statistics
            var diskData = DiskHistory.Where(d => d.Timestamp >= startTime).ToList();
            if (diskData.Any())
            {
                report.DiskStatistics = new PerformanceStatistics
                {
                    Average = diskData.Average(d => d.Value),
                    Minimum = diskData.Min(d => d.Value),
                    Maximum = diskData.Max(d => d.Value),
                    Count = diskData.Count
                };
            }
            
            // Network Statistics
            var networkData = NetworkHistory.Where(d => d.Timestamp >= startTime).ToList();
            if (networkData.Any())
            {
                report.NetworkStatistics = new PerformanceStatistics
                {
                    Average = networkData.Average(d => d.Value),
                    Minimum = networkData.Min(d => d.Value),
                    Maximum = networkData.Max(d => d.Value),
                    Count = networkData.Count
                };
            }
            
            // Application Metrics
            report.ApplicationStatistics = new ApplicationStatistics
            {
                TotalScriptExecutions = ScriptExecutions,
                TotalExecutionTime = TotalExecutionTime,
                AverageExecutionTime = ScriptExecutions > 0 ? 
                    TimeSpan.FromMilliseconds(TotalExecutionTime.TotalMilliseconds / ScriptExecutions) : 
                    TimeSpan.Zero,
                TotalProcessedBytes = ProcessedBytes,
                PeakActiveConnections = ActiveConnections
            };
            
            // Recent Alerts
            report.RecentAlerts = Alerts.Where(a => a.Timestamp >= startTime).ToList();
            
            return report;
        }
        
        public void RecordScriptExecution(TimeSpan executionTime, long processedBytes)
        {
            ScriptExecutions++;
            TotalExecutionTime = TotalExecutionTime.Add(executionTime);
            ProcessedBytes += processedBytes;
            
            // Update application metrics
            var metric = ApplicationMetrics.FirstOrDefault(m => m.Name == "Script Executions");
            if (metric != null)
            {
                metric.Value = ScriptExecutions;
                metric.LastUpdated = DateTime.Now;
            }
            
            CheckPerformanceThresholds();
        }
        
        public void RecordNetworkActivity(long bytesSent, long bytesReceived)
        {
            NetworkSent += bytesSent;
            NetworkReceived += bytesReceived;
            
            var timestamp = DateTime.Now;
            var totalBytes = bytesSent + bytesReceived;
            
            Application.Current.Dispatcher.BeginInvoke(() =>
            {
                NetworkHistory.Add(new PerformanceDataPoint
                {
                    Timestamp = timestamp,
                    Value = totalBytes,
                    Category = "Network"
                });
                
                // Maintain history size
                if (NetworkHistory.Count > _config.MaxHistoryPoints)
                {
                    NetworkHistory.RemoveAt(0);
                }
            });
        }
        
        public void RecordConnectionActivity(int activeConnections)
        {
            ActiveConnections = activeConnections;
            
            var metric = ApplicationMetrics.FirstOrDefault(m => m.Name == "Active Connections");
            if (metric != null)
            {
                metric.Value = activeConnections;
                metric.LastUpdated = DateTime.Now;
            }
        }
        
        public void ClearHistory()
        {
            Application.Current.Dispatcher.BeginInvoke(() =>
            {
                CpuHistory.Clear();
                MemoryHistory.Clear();
                DiskHistory.Clear();
                NetworkHistory.Clear();
                Alerts.Clear();
            });
        }
        
        public void ExportData(string filePath, TimeSpan timespan)
        {
            try
            {
                var report = GenerateReport(timespan);
                var json = System.Text.Json.JsonSerializer.Serialize(report, new System.Text.Json.JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                File.WriteAllText(filePath, json);
                LogEvent($"Performance data exported to {filePath}");
            }
            catch (Exception ex)
            {
                LogEvent($"Failed to export data: {ex.Message}");
            }
        }
        
        #endregion
        
        #region Private Methods
        
        private void CollectMetrics(object state)
        {
            try
            {
                var timestamp = DateTime.Now;
                
                // Collect system metrics
                var cpuUsage = _cpuCounter.NextValue();
                var availableMemory = _memoryCounter.NextValue();
                var diskUsage = _diskCounter.NextValue();
                
                // Get process-specific metrics
                _currentProcess.Refresh();
                var processMemory = _currentProcess.WorkingSet64;
                
                // Update properties on UI thread
                Application.Current.Dispatcher.BeginInvoke(() =>
                {
                    CpuUsage = cpuUsage;
                    MemoryUsage = processMemory;
                    DiskUsage = diskUsage;
                    
                    // Add to history
                    AddToHistory(CpuHistory, timestamp, cpuUsage, "CPU");
                    AddToHistory(MemoryHistory, timestamp, processMemory, "Memory");
                    AddToHistory(DiskHistory, timestamp, diskUsage, "Disk");
                    
                    // Update application metrics
                    UpdateApplicationMetrics();
                    
                    // Check thresholds
                    CheckPerformanceThresholds();
                });
            }
            catch (Exception ex)
            {
                LogEvent($"Error collecting metrics: {ex.Message}");
            }
        }
        
        private void AddToHistory(ObservableCollection<PerformanceDataPoint> collection, 
                                 DateTime timestamp, float value, string category)
        {
            collection.Add(new PerformanceDataPoint
            {
                Timestamp = timestamp,
                Value = value,
                Category = category
            });
            
            // Maintain collection size
            if (collection.Count > _config.MaxHistoryPoints)
            {
                collection.RemoveAt(0);
            }
        }
        
        private void InitializeApplicationMetrics()
        {
            var metrics = new[]
            {
                new ApplicationMetric { Name = "Script Executions", Value = 0, Unit = "count" },
                new ApplicationMetric { Name = "Execution Time", Value = 0, Unit = "ms" },
                new ApplicationMetric { Name = "Active Connections", Value = 0, Unit = "count" },
                new ApplicationMetric { Name = "Processed Bytes", Value = 0, Unit = "bytes" },
                new ApplicationMetric { Name = "Memory Usage", Value = 0, Unit = "MB" },
                new ApplicationMetric { Name = "CPU Usage", Value = 0, Unit = "%" }
            };
            
            foreach (var metric in metrics)
            {
                ApplicationMetrics.Add(metric);
            }
        }
        
        private void UpdateApplicationMetrics()
        {
            var metrics = ApplicationMetrics.ToList();
            
            foreach (var metric in metrics)
            {
                switch (metric.Name)
                {
                    case "Memory Usage":
                        metric.Value = MemoryUsage / (1024 * 1024); // Convert to MB
                        break;
                    case "CPU Usage":
                        metric.Value = CpuUsage;
                        break;
                    case "Execution Time":
                        metric.Value = TotalExecutionTime.TotalMilliseconds;
                        break;
                }
                metric.LastUpdated = DateTime.Now;
            }
        }
        
        private void CheckPerformanceThresholds()
        {
            // CPU threshold
            if (CpuUsage > _config.CpuThreshold)
            {
                CreateAlert("High CPU Usage", $"CPU usage is {CpuUsage:F1}%", AlertSeverity.Warning);
            }
            
            // Memory threshold
            var memoryMB = MemoryUsage / (1024 * 1024);
            if (memoryMB > _config.MemoryThreshold)
            {
                CreateAlert("High Memory Usage", $"Memory usage is {memoryMB:F1} MB", AlertSeverity.Warning);
            }
            
            // Disk threshold
            if (DiskUsage > _config.DiskThreshold)
            {
                CreateAlert("High Disk Usage", $"Disk usage is {DiskUsage:F1}%", AlertSeverity.Warning);
            }
        }
        
        private void CreateAlert(string title, string message, AlertSeverity severity)
        {
            var alert = new PerformanceAlert
            {
                Title = title,
                Message = message,
                Severity = severity,
                Timestamp = DateTime.Now
            };
            
            Application.Current.Dispatcher.BeginInvoke(() =>
            {
                Alerts.Add(alert);
                
                // Maintain alerts collection size
                if (Alerts.Count > _config.MaxAlerts)
                {
                    Alerts.RemoveAt(0);
                }
            });
            
            LogEvent($"Performance Alert: {title} - {message}");
        }
        
        private PerformanceConfiguration LoadConfiguration()
        {
            return new PerformanceConfiguration
            {
                MaxHistoryPoints = 1000,
                MaxAlerts = 100,
                CpuThreshold = 80.0f,
                MemoryThreshold = 1024, // 1GB
                DiskThreshold = 80.0f,
                NetworkThreshold = 10485760 // 10MB
            };
        }
        
        private void LogEvent(string message)
        {
            System.Diagnostics.Debug.WriteLine($"[PerformanceMonitor] {DateTime.Now:HH:mm:ss} - {message}");
        }
        
        #endregion
        
        #region INotifyPropertyChanged
        
        public event PropertyChangedEventHandler PropertyChanged;
        
        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
        
        #endregion
        
        #region IDisposable
        
        public void Dispose()
        {
            StopMonitoring();
            
            _monitoringTimer?.Dispose();
            _cpuCounter?.Dispose();
            _memoryCounter?.Dispose();
            _diskCounter?.Dispose();
            _currentProcess?.Dispose();
        }
        
        #endregion
    }
    
    #region Supporting Classes
    
    public class PerformanceConfiguration
    {
        public int MaxHistoryPoints { get; set; } = 1000;
        public int MaxAlerts { get; set; } = 100;
        public float CpuThreshold { get; set; } = 80.0f;
        public float MemoryThreshold { get; set; } = 1024.0f; // MB
        public float DiskThreshold { get; set; } = 80.0f;
        public long NetworkThreshold { get; set; } = 10485760; // bytes
    }
    
    #endregion
}