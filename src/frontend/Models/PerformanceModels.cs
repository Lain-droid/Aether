using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace AetherVisor.Frontend.Models
{
    public class PerformanceDataPoint : INotifyPropertyChanged
    {
        private DateTime _timestamp;
        private float _value;
        private string _category;

        public DateTime Timestamp
        {
            get => _timestamp;
            set { _timestamp = value; OnPropertyChanged(nameof(Timestamp)); }
        }

        public float Value
        {
            get => _value;
            set { _value = value; OnPropertyChanged(nameof(Value)); }
        }

        public string Category
        {
            get => _category;
            set { _category = value; OnPropertyChanged(nameof(Category)); }
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ApplicationMetric : INotifyPropertyChanged
    {
        private string _name;
        private float _value;
        private string _unit;
        private DateTime _lastUpdated;

        public string Name
        {
            get => _name;
            set { _name = value; OnPropertyChanged(nameof(Name)); }
        }

        public float Value
        {
            get => _value;
            set { _value = value; OnPropertyChanged(nameof(Value)); OnPropertyChanged(nameof(FormattedValue)); }
        }

        public string Unit
        {
            get => _unit;
            set { _unit = value; OnPropertyChanged(nameof(Unit)); OnPropertyChanged(nameof(FormattedValue)); }
        }

        public DateTime LastUpdated
        {
            get => _lastUpdated;
            set { _lastUpdated = value; OnPropertyChanged(nameof(LastUpdated)); }
        }

        public string FormattedValue
        {
            get
            {
                return Unit switch
                {
                    "bytes" => FormatBytes((long)Value),
                    "ms" => $"{Value:F2} ms",
                    "%" => $"{Value:F1}%",
                    "MB" => $"{Value:F1} MB",
                    _ => $"{Value:F0} {Unit}"
                };
            }
        }

        private string FormatBytes(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB", "TB" };
            double len = bytes;
            int order = 0;
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len = len / 1024;
            }
            return $"{len:F2} {sizes[order]}";
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class PerformanceAlert : INotifyPropertyChanged
    {
        private string _title;
        private string _message;
        private AlertSeverity _severity;
        private DateTime _timestamp;
        private bool _isRead;

        public string Title
        {
            get => _title;
            set { _title = value; OnPropertyChanged(nameof(Title)); }
        }

        public string Message
        {
            get => _message;
            set { _message = value; OnPropertyChanged(nameof(Message)); }
        }

        public AlertSeverity Severity
        {
            get => _severity;
            set { _severity = value; OnPropertyChanged(nameof(Severity)); OnPropertyChanged(nameof(SeverityColor)); }
        }

        public DateTime Timestamp
        {
            get => _timestamp;
            set { _timestamp = value; OnPropertyChanged(nameof(Timestamp)); }
        }

        public bool IsRead
        {
            get => _isRead;
            set { _isRead = value; OnPropertyChanged(nameof(IsRead)); }
        }

        public string SeverityColor
        {
            get
            {
                return Severity switch
                {
                    AlertSeverity.Info => "#007ACC",
                    AlertSeverity.Warning => "#FFCC02",
                    AlertSeverity.Error => "#D13438",
                    AlertSeverity.Critical => "#8B0000",
                    _ => "#007ACC"
                };
            }
        }

        public string TimestampFormatted => Timestamp.ToString("HH:mm:ss");

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public enum AlertSeverity
    {
        Info,
        Warning,
        Error,
        Critical
    }

    public class PerformanceStatistics
    {
        public float Average { get; set; }
        public float Minimum { get; set; }
        public float Maximum { get; set; }
        public int Count { get; set; }
        public float StandardDeviation { get; set; }
    }

    public class ApplicationStatistics
    {
        public int TotalScriptExecutions { get; set; }
        public TimeSpan TotalExecutionTime { get; set; }
        public TimeSpan AverageExecutionTime { get; set; }
        public long TotalProcessedBytes { get; set; }
        public int PeakActiveConnections { get; set; }
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
    }

    public class PerformanceReport
    {
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
        public TimeSpan Duration { get; set; }
        public DateTime GeneratedAt { get; set; }

        public PerformanceStatistics CpuStatistics { get; set; }
        public PerformanceStatistics MemoryStatistics { get; set; }
        public PerformanceStatistics DiskStatistics { get; set; }
        public PerformanceStatistics NetworkStatistics { get; set; }

        public ApplicationStatistics ApplicationStatistics { get; set; }
        public List<PerformanceAlert> RecentAlerts { get; set; } = new();

        public string Summary
        {
            get
            {
                var lines = new List<string>
                {
                    $"Performance Report ({Duration.TotalHours:F1} hours)",
                    $"Generated: {GeneratedAt:yyyy-MM-dd HH:mm:ss}",
                    "",
                    "System Performance:",
                    $"  CPU: Avg {CpuStatistics?.Average:F1}%, Peak {CpuStatistics?.Maximum:F1}%",
                    $"  Memory: Avg {MemoryStatistics?.Average / (1024 * 1024):F1} MB, Peak {MemoryStatistics?.Maximum / (1024 * 1024):F1} MB",
                    $"  Disk: Avg {DiskStatistics?.Average:F1}%, Peak {DiskStatistics?.Maximum:F1}%",
                    "",
                    "Application Performance:",
                    $"  Script Executions: {ApplicationStatistics?.TotalScriptExecutions:N0}",
                    $"  Average Execution Time: {ApplicationStatistics?.AverageExecutionTime.TotalMilliseconds:F2} ms",
                    $"  Total Processed: {FormatBytes(ApplicationStatistics?.TotalProcessedBytes ?? 0)}",
                    $"  Peak Connections: {ApplicationStatistics?.PeakActiveConnections:N0}",
                    "",
                    $"Alerts: {RecentAlerts?.Count:N0}"
                };

                return string.Join(Environment.NewLine, lines);
            }
        }

        private string FormatBytes(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB", "TB" };
            double len = bytes;
            int order = 0;
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len = len / 1024;
            }
            return $"{len:F2} {sizes[order]}";
        }
    }

    public class SystemInfo
    {
        public string OperatingSystem { get; set; }
        public string ProcessorName { get; set; }
        public int ProcessorCores { get; set; }
        public long TotalMemory { get; set; }
        public long AvailableMemory { get; set; }
        public string MachineName { get; set; }
        public string UserName { get; set; }
        public TimeSpan SystemUptime { get; set; }
        public DateTime LastBootTime { get; set; }

        public float MemoryUsagePercentage => TotalMemory > 0 ? 
            ((float)(TotalMemory - AvailableMemory) / TotalMemory) * 100 : 0;

        public string FormattedTotalMemory => FormatBytes(TotalMemory);
        public string FormattedAvailableMemory => FormatBytes(AvailableMemory);

        private string FormatBytes(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB", "TB" };
            double len = bytes;
            int order = 0;
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len = len / 1024;
            }
            return $"{len:F2} {sizes[order]}";
        }
    }

    public class ProcessInfo
    {
        public int ProcessId { get; set; }
        public string ProcessName { get; set; }
        public long WorkingSet { get; set; }
        public long PrivateMemorySize { get; set; }
        public TimeSpan TotalProcessorTime { get; set; }
        public DateTime StartTime { get; set; }
        public int ThreadCount { get; set; }
        public int HandleCount { get; set; }

        public string FormattedWorkingSet => FormatBytes(WorkingSet);
        public string FormattedPrivateMemory => FormatBytes(PrivateMemorySize);
        public TimeSpan RunTime => DateTime.Now - StartTime;

        private string FormatBytes(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB", "TB" };
            double len = bytes;
            int order = 0;
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len = len / 1024;
            }
            return $"{len:F2} {sizes[order]}";
        }
    }

    public class NetworkStatistics
    {
        public long BytesSent { get; set; }
        public long BytesReceived { get; set; }
        public long PacketsSent { get; set; }
        public long PacketsReceived { get; set; }
        public int ActiveConnections { get; set; }
        public DateTime LastUpdated { get; set; }

        public long TotalBytes => BytesSent + BytesReceived;
        public long TotalPackets => PacketsSent + PacketsReceived;

        public string FormattedBytesSent => FormatBytes(BytesSent);
        public string FormattedBytesReceived => FormatBytes(BytesReceived);
        public string FormattedTotalBytes => FormatBytes(TotalBytes);

        private string FormatBytes(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB", "TB" };
            double len = bytes;
            int order = 0;
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len = len / 1024;
            }
            return $"{len:F2} {sizes[order]}";
        }
    }
}