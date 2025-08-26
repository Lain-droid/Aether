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

        private string _scriptText = "-- Welcome to AetherVisor!\nprint('Hello from the other side!')";
        public string ScriptText { get => _scriptText; set { _scriptText = value; OnPropertyChanged(nameof(ScriptText)); } }

        private string _statusText = "Ready. Please attach to Roblox.";
        public string StatusText { get => _statusText; set { _statusText = value; OnPropertyChanged(nameof(StatusText)); } }

        private bool _isInjected = false;
        public bool IsInjected { get => _isInjected; set { _isInjected = value; OnPropertyChanged(nameof(IsInjected)); } }

        private bool _isWarningVisible = false;
        public bool IsWarningVisible { get => _isWarningVisible; set { _isWarningVisible = value; OnPropertyChanged(nameof(IsWarningVisible)); } }

        public ObservableCollection<string> ConsoleOutput { get; } = new ObservableCollection<string>();

        public ICommand ExecuteScriptCommand { get; }
        public ICommand ExecuteAnywayCommand { get; }
        public ICommand DismissWarningCommand { get; }
        public ICommand InjectCommand { get; }

        public MainViewModel(IIpcClientService ipcClientService)
        {
            _ipcClientService = ipcClientService;

            // Subscribe to events from the IPC service
            _ipcClientService.OnConsoleOutputReceived += (output) => App.Current.Dispatcher.Invoke(() => ConsoleOutput.Add(output));
            _ipcClientService.OnStatusUpdated += (status) => App.Current.Dispatcher.Invoke(() => {
                StatusText = status;
                IsInjected = status == "Injected";
            });
            _ipcClientService.OnAnalysisResultReceived += HandleAnalysisResult;

            // Setup Commands
            ExecuteScriptCommand = new RelayCommand(p => _ipcClientService.AnalyzeScript(ScriptText), p => IsInjected && !string.IsNullOrEmpty(ScriptText));
            ExecuteAnywayCommand = new RelayCommand(p => { IsWarningVisible = false; _ipcClientService.ExecuteScript(ScriptText); });
            DismissWarningCommand = new RelayCommand(p => IsWarningVisible = false);
            InjectCommand = new RelayCommand(p => _ipcClientService.ConnectAndInject(), p => !IsInjected);

            ConsoleOutput.Add("AetherVisor Frontend Initialized.");
        }

        private void HandleAnalysisResult(string result)
        {
            App.Current.Dispatcher.Invoke(() => {
                if (result == "SAFE")
                {
                    _ipcClientService.ExecuteScript(ScriptText);
                }
                else // UNSAFE
                {
                    IsWarningVisible = true;
                }
            });
        }
    }
}
