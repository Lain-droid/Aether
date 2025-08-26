using AetherVisor.Frontend.Services;
using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;

namespace AetherVisor.Frontend.ViewModels
{
    // A simple ICommand implementation
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

    public class MainViewModel : ObservableObject
    {
        private readonly IIpcClientService _ipcClientService;

        private string _scriptText = "-- Welcome to AetherVisor! Logs will appear here.";
        public string ScriptText { get => _scriptText; set { _scriptText = value; OnPropertyChanged(nameof(ScriptText)); } }

        private string _statusText = "Ready to Inject.";
        public string StatusText { get => _statusText; set { _statusText = value; OnPropertyChanged(nameof(StatusText)); } }

        private bool _isWarningVisible = false;
        public bool IsWarningVisible { get => _isWarningVisible; set { _isWarningVisible = value; OnPropertyChanged(nameof(IsWarningVisible)); } }

        public ObservableCollection<string> ConsoleOutput { get; } = new ObservableCollection<string>();

        public ICommand InjectCommand { get; }
        public ICommand ExecuteScriptCommand { get; }
        public ICommand ExecuteAnywayCommand { get; }
        public ICommand DismissWarningCommand { get; }

        public MainViewModel(IIpcClientService ipcClientService)
        {
            _ipcClientService = ipcClientService;

            // Subscribe to events
            _ipcClientService.OnConsoleOutputReceived += (output) => App.Current.Dispatcher.Invoke(() => ConsoleOutput.Add(output));

            // Start monitoring the log file for console output
            _ipcClientService.StartLogMonitoring("output.log");

            // Setup Commands
            InjectCommand = new RelayCommand(p => _ipcClientService.LaunchBackend("--inject RobloxPlayerBeta.exe"));
            // Note: Script execution and analysis would now also need to be done via command-line arguments
            // This simplifies the example to focus on the main injection flow.
            ExecuteScriptCommand = new RelayCommand(p => { /* Launch backend with --execute flag */ });
            ExecuteAnywayCommand = new RelayCommand(p => { /* Launch backend with --execute flag */ });
            DismissWarningCommand = new RelayCommand(p => IsWarningVisible = false);

            ConsoleOutput.Add("AetherVisor Frontend Initialized.");
            ConsoleOutput.Add("Click 'Inject' to start.");
        }
    }
}
