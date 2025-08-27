using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;
using System;

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

        private void ExecuteScript()
        {
            ConsoleOutput.Add($"Execute requested at {DateTime.Now:HH:mm:ss}");
        }

        private void Inject()
        {
            ConsoleOutput.Add("Inject requested");
        }
    }
}
