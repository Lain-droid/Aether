using System;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;

namespace AetherVisor.Frontend.Services
{
    public interface IIpcClientService
    {
        event Action<string> OnConsoleOutputReceived;
        void LaunchBackend(string arguments);
        void StartLogMonitoring(string logFilePath);
    }

    public class IpcClientService : IIpcClientService
    {
        public event Action<string> OnConsoleOutputReceived;
        private FileSystemWatcher _logWatcher;
        private long _lastReadPosition = 0;

        public void LaunchBackend(string arguments)
        {
            try
            {
                // Assuming the backend executable is in a known location
                string backendPath = "AetherVisor.Backend.exe";
                Process.Start(backendPath, arguments);
            }
            catch (Exception ex)
            {
                OnConsoleOutputReceived?.Invoke($"Error launching backend: {ex.Message}");
            }
        }

        public void StartLogMonitoring(string logFilePath)
        {
            // Ensure the log file exists before watching it
            if (!File.Exists(logFilePath))
            {
                File.Create(logFilePath).Close();
            }

            _logWatcher = new FileSystemWatcher
            {
                Path = Path.GetDirectoryName(logFilePath),
                Filter = Path.GetFileName(logFilePath),
                NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.Size
            };

            _logWatcher.Changed += OnLogFileChanged;
            _logWatcher.EnableRaisingEvents = true;
            ReadNewLogEntries(logFilePath); // Read any existing content
        }

        private void OnLogFileChanged(object sender, FileSystemEventArgs e)
        {
            ReadNewLogEntries(e.FullPath);
        }

        private void ReadNewLogEntries(string logFilePath)
        {
            try
            {
                using (var fs = new FileStream(logFilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                {
                    if (fs.Length > _lastReadPosition)
                    {
                        fs.Seek(_lastReadPosition, SeekOrigin.Begin);
                        using (var reader = new StreamReader(fs))
                        {
                            string newContent = reader.ReadToEnd();
                            if (!string.IsNullOrEmpty(newContent))
                            {
                                // Split into lines and send them one by one
                                var lines = newContent.Split(new[] { Environment.NewLine }, StringSplitOptions.RemoveEmptyEntries);
                                foreach (var line in lines)
                                {
                                    App.Current.Dispatcher.Invoke(() => OnConsoleOutputReceived?.Invoke(line));
                                }
                            }
                        }
                        _lastReadPosition = fs.Position;
                    }
                }
            }
            catch (Exception ex)
            {
                // Log errors silently or to a different channel
                Debug.WriteLine($"Error reading log file: {ex.Message}");
            }
        }
    }
}
