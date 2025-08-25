using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;

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
        private string _scriptText = "-- Welcome to AetherVisor!\nprint('Hello from the other side!')";
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
            // In a real app, these commands would be implemented with RelayCommand or similar
            // ExecuteScriptCommand = new RelayCommand(p => CanExecuteScript(), p => ExecuteScript());
            // InjectCommand = new RelayCommand(p => CanInject(), p => Inject());

            ConsoleOutput.Add("AetherVisor Frontend Initialized.");
            ConsoleOutput.Add("Waiting for injection...");
        }
    }
}
