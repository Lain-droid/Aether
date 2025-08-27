using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using AetherVisor.Frontend.Views;

namespace AetherVisor.Frontend.ViewModels
{
    public class SetupViewModel : INotifyPropertyChanged
    {
        private int _progress;
        public int Progress { get => _progress; private set { _progress = value; OnPropertyChanged(nameof(Progress)); } }

        public ObservableCollection<string> Steps { get; } = new ObservableCollection<string>();

        public ICommand InjectCommand { get; }

        public SetupViewModel()
        {
            InjectCommand = new RelayCommand(_ => true, _ => _ = RunSetupAsync());
        }

        private async Task RunSetupAsync()
        {
            Steps.Clear();
            Progress = 0;
            await Step("IPC kanalı hazırlanıyor", 25);
            await Step("Kaynaklar doğrulanıyor", 55);
            await Step("Polymorphic engine başlatılıyor", 75);
            await Step("User-mode inject akışı simüle ediliyor", 100);

            // Başarılı ise ana pencereye geç
            Application.Current.Dispatcher.Invoke(() =>
            {
                var main = new MainWindow();
                main.Show();
                foreach (Window w in Application.Current.Windows)
                {
                    if (w is SetupWindow) { w.Close(); break; }
                }
            });
        }

        private async Task Step(string title, int progress)
        {
            Steps.Add(title);
            await Task.Delay(500);
            Progress = progress;
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
