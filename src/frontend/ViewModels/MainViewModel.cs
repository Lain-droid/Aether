using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;
using System;
using System.Windows;
using AetherVisor.Frontend.Services;

namespace AetherVisor.Frontend.ViewModels
{
    // A base class for observable properties, essential for MVVM
    public abstract class ObservableObject : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class RelayCommand : ICommand
    {
        private readonly Predicate<object> _canExecute;
        private readonly Action<object> _execute;

        public RelayCommand(Predicate<object> canExecute, Action<object> execute)
        {
            _canExecute = canExecute ?? (_ => true);
            _execute = execute ?? (_ => { });
        }

        public bool CanExecute(object parameter) => _canExecute(parameter);
        public void Execute(object parameter) => _execute(parameter);
        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }

    public class MainViewModel : ObservableObject
    {
        private string _scriptText = "-- Aether script here\nprint('ready')";
        public string ScriptText
        {
            get => _scriptText;
            set { _scriptText = value; OnPropertyChanged(nameof(ScriptText)); }
        }

        public ObservableCollection<string> ConsoleOutput { get; } = new ObservableCollection<string>();

        public ICommand ExecuteScriptCommand { get; }
        public ICommand InjectCommand { get; }
        private readonly IIpcClientService _ipc = new IpcClientService();

        public MainViewModel()
        {
            ExecuteScriptCommand = new RelayCommand(_ => CanExecuteScript(), _ => ExecuteScript());
            InjectCommand = new RelayCommand(_ => true, _ => Inject());

            ConsoleOutput.Add("AetherVisor initialized");
            ConsoleOutput.Add("Idle");
        }

        private bool CanExecuteScript()
        {
            return !string.IsNullOrWhiteSpace(ScriptText);
        }

        private async void ExecuteScript()
        {
            ConsoleOutput.Add($"Execute requested at {DateTime.Now:HH:mm:ss}");
            try {
                await _ipc.ConnectAsync();
                await _ipc.SendScriptAsync(ScriptText);
                ConsoleOutput.Add("Script sent to backend");
            } catch (Exception ex) {
                ConsoleOutput.Add($"IPC error: {ex.Message}");
            }
        }

        private void Inject()
        {
            ConsoleOutput.Add("Inject requested");
        }
    }
}
