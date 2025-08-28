using System.ComponentModel;
using AetherVisor.Frontend.Services;

namespace AetherVisor.Frontend.Models
{
    public class PluginInfo : INotifyPropertyChanged
    {
        private bool _isEnabled;
        private bool _isActive;
        private string _status;

        public string Id { get; set; }
        public string Name { get; set; }
        public string Version { get; set; }
        public string Description { get; set; }
        public string Author { get; set; }
        public string Icon { get; set; }
        public string Website { get; set; }
        public string[] Dependencies { get; set; } = new string[0];
        public string MinimumAppVersion { get; set; }
        public IPlugin Plugin { get; set; }

        public bool IsEnabled
        {
            get => _isEnabled;
            set { _isEnabled = value; OnPropertyChanged(nameof(IsEnabled)); }
        }

        public bool IsActive
        {
            get => _isActive;
            set { _isActive = value; OnPropertyChanged(nameof(IsActive)); UpdateStatus(); }
        }

        public string Status
        {
            get => _status;
            private set { _status = value; OnPropertyChanged(nameof(Status)); }
        }

        public bool CanActivate => IsEnabled && !IsActive;
        public bool CanDeactivate => IsActive;

        private void UpdateStatus()
        {
            Status = IsActive ? "Active" : (IsEnabled ? "Loaded" : "Disabled");
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}