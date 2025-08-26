using System;
using System.ComponentModel;
using System.Windows.Input;

namespace AetherVisor.Frontend.ViewModels
{
    // Re-defining these here for completeness, though they'd be in shared files.
    public class RelayCommand : ICommand
    {
        private readonly Action<object> _execute;
        private readonly Predicate<object> _canExecute;
        public event EventHandler CanExecuteChanged { add { CommandManager.Rebind(); } remove { CommandManager.Rebind(); } }
        public RelayCommand(Action<object> execute, Predicate<object> canExecute = null) { _execute = execute; _canExecute = canExecute; }
        public bool CanExecute(object parameter) => _canExecute?.Invoke(parameter) ?? true;
        public void Execute(object parameter) => _execute(parameter);
    }

    public abstract class ObservableObject : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string propertyName) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public enum Theme
    {
        Light,
        Dark
    }

    public enum InjectionMethod
    {
        CreateRemoteThread,
        ThreadHijacking, // Placeholder
        ManualMap        // Placeholder
    }

    public class SettingsViewModel : ObservableObject
    {
        private Theme _selectedTheme;
        public Theme SelectedTheme
        {
            get => _selectedTheme;
            set { _selectedTheme = value; OnPropertyChanged(nameof(SelectedTheme)); }
        }

        private bool _isAutoInjectEnabled;
        public bool IsAutoInjectEnabled
        {
            get => _isAutoInjectEnabled;
            set { _isAutoInjectEnabled = value; OnPropertyChanged(nameof(IsAutoInjectEnabled)); }
        }

        private InjectionMethod _selectedInjectionMethod;
        public InjectionMethod SelectedInjectionMethod
        {
            get => _selectedInjectionMethod;
            set { _selectedInjectionMethod = value; OnPropertyChanged(nameof(SelectedInjectionMethod)); }
        }

        private string _logFilePath;
        public string LogFilePath
        {
            get => _logFilePath;
            set { _logFilePath = value; OnPropertyChanged(nameof(LogFilePath)); }
        }

        public ICommand SaveSettingsCommand { get; }

        public SettingsViewModel()
        {
            // Load settings from a file or use defaults
            SelectedTheme = Theme.Dark;
            IsAutoInjectEnabled = true;
            SelectedInjectionMethod = InjectionMethod.CreateRemoteThread;
            LogFilePath = "output.log";

            SaveSettingsCommand = new RelayCommand(p => SaveSettings());
        }

        private void SaveSettings()
        {
            // In a real app, this would save to a config file.
            // For now, we can just log it.
            Console.WriteLine("Settings Saved!");
            Console.WriteLine($"- Theme: {SelectedTheme}");
            Console.WriteLine($"- Auto-Inject: {IsAutoInjectEnabled}");
            Console.WriteLine($"- Injection Method: {SelectedInjectionMethod}");
            Console.WriteLine($"- Log File Path: {LogFilePath}");
        }
    }
}
