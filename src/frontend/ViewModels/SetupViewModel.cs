using System;
using System.ComponentModel;
using System.Security.Principal;
using System.Windows.Input;

namespace AetherVisor.Frontend.ViewModels
{
    // Re-defining these here for completeness.
    public class RelayCommand : ICommand
    {
        private readonly Action<object> _execute;
        public event EventHandler CanExecuteChanged { add { CommandManager.Rebind(); } remove { CommandManager.Rebind(); } }
        public RelayCommand(Action<object> execute) { _execute = execute; }
        public bool CanExecute(object parameter) => true;
        public void Execute(object parameter) => _execute(parameter);
    }

    public abstract class ObservableObject : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string propertyName) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public class SetupViewModel : ObservableObject
    {
        private string _statusMessage;
        public string StatusMessage
        {
            get => _statusMessage;
            set { _statusMessage = value; OnPropertyChanged(nameof(StatusMessage)); }
        }

        private bool _isChecking;
        public bool IsChecking
        {
            get => _isChecking;
            set { _isChecking = value; OnPropertyChanged(nameof(IsChecking)); }
        }

        private bool _isAdmin;
        public bool IsAdmin
        {
            get => _isAdmin;
            set { _isAdmin = value; OnPropertyChanged(nameof(IsAdmin)); }
        }

        public ICommand CheckRequirementsCommand { get; }

        public SetupViewModel()
        {
            CheckRequirementsCommand = new RelayCommand(p => CheckRequirements());
        }

        public void CheckRequirements()
        {
            IsChecking = true;
            StatusMessage = "Checking for administrator privileges...";

            using (WindowsIdentity identity = WindowsIdentity.GetCurrent())
            {
                WindowsPrincipal principal = new WindowsPrincipal(identity);
                IsAdmin = principal.IsInRole(WindowsBuiltInRole.Administrator);
            }

            if (IsAdmin)
            {
                StatusMessage = "Administrator privileges confirmed. Ready to proceed!";
            }
            else
            {
                StatusMessage = "Administrator privileges required. Please restart as administrator.";
            }
            IsChecking = false;
        }
    }
}
