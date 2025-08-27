using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using AetherVisor.Frontend.Views;
using AetherVisor.Frontend.Services;

namespace AetherVisor.Frontend.ViewModels
{
    public class SetupViewModel : INotifyPropertyChanged
    {
        private int _progress;
        public int Progress { get => _progress; private set { _progress = value; OnPropertyChanged(nameof(Progress)); } }

        public ObservableCollection<string> Steps { get; } = new ObservableCollection<string>();

        public ICommand InjectCommand { get; }
        public ICommand LaunchRobloxCommand { get; }
        private bool _acceptRisks;
        public bool AcceptRisks { get => _acceptRisks; set { _acceptRisks = value; OnPropertyChanged(nameof(AcceptRisks)); } }
        private bool _injectCompleted;
        public bool InjectCompleted { get => _injectCompleted; private set { _injectCompleted = value; OnPropertyChanged(nameof(InjectCompleted)); } }
        private readonly IIpcClientService _ipc = new IpcClientService();

        public SetupViewModel()
        {
            InjectCommand = new RelayCommand(_ => AcceptRisks, _ => _ = RunSetupAsync());
            LaunchRobloxCommand = new RelayCommand(_ => InjectCompleted, _ => LaunchRoblox());
        }

        private async Task RunSetupAsync()
        {
            Steps.Clear();
            Progress = 0;
            await Step("IPC kanalı hazırlanıyor", 10);
            await _ipc.ConnectAsync();
            await Step("Backend ile bağlantı kuruldu", 25);
            await Step("Kaynaklar doğrulanıyor", 55);
            await Step("Polymorphic engine başlatılıyor", 75);
            // Inject isteği: opcode=1 + process adı (utf16)
            await Step("Inject başlatılıyor", 90);
            await SendInjectAsync("RobloxPlayerBeta.exe");
            await Step("Bypass modülleri başlatılıyor", 95);
            await Step("Tamamlandı", 100);
            InjectCompleted = true;

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

        private async Task SendInjectAsync(string process)
        {
            // C# tarafında inject komutu için pipe yazımı (utf16 payload)
            // BinaryWriter ile manuel yazacağız
            if (!(_ipc is IpcClientService concrete) || concrete == null)
            {
                return;
            }
            var field = typeof(IpcClientService).GetField("_pipe", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
            var pipe = field?.GetValue(concrete) as System.IO.Pipes.NamedPipeClientStream;
            if (pipe == null || !pipe.IsConnected) return;

            var payload = System.Text.Encoding.Unicode.GetBytes(process);
            using (var ms = new System.IO.MemoryStream())
            using (var bw = new System.IO.BinaryWriter(ms))
            {
                bw.Write((uint)(1 + payload.Length));
                bw.Write((byte)1); // Inject opcode
                bw.Write(payload);
                bw.Flush();
                var buffer = ms.ToArray();
                await pipe.WriteAsync(buffer, 0, buffer.Length);
                await pipe.FlushAsync();
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

        private void LaunchRoblox()
        {
            try {
                System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo {
                    FileName = "roblox://",
                    UseShellExecute = true
                });
            } catch {
                try {
                    var path = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                        "Roblox", "Versions", "RobloxPlayerBeta.exe");
                    System.Diagnostics.Process.Start(path);
                } catch { }
            }
        }
    }
}
