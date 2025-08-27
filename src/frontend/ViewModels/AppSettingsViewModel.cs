using System;
using System.ComponentModel;
using System.Windows.Input;
using AetherVisor.Frontend.Services;

namespace AetherVisor.Frontend.ViewModels
{
    public class AppSettingsViewModel : INotifyPropertyChanged
    {
        private double _editorFontSize = 14;
        public double EditorFontSize { get => _editorFontSize; set { _editorFontSize = value; OnPropertyChanged(nameof(EditorFontSize)); } }

        private bool _autocompleteEnabled = true;
        public bool AutocompleteEnabled { get => _autocompleteEnabled; set { _autocompleteEnabled = value; OnPropertyChanged(nameof(AutocompleteEnabled)); } }

        private double _aiSensitivity = 0.5;
        public double AiSensitivity { get => _aiSensitivity; set { _aiSensitivity = value; OnPropertyChanged(nameof(AiSensitivity)); } }

        public ICommand SaveCommand { get; }
        private readonly IIpcClientService _ipc = new IpcClientService();

        public AppSettingsViewModel()
        {
            SaveCommand = new RelayCommand(_ => true, _ => Save());
        }

        private async void Save()
        {
            // Send AI sensitivity to backend via IPC op=3
            try {
                await _ipc.ConnectAsync();
                await IpcSendAiAsync(AiSensitivity);
            } catch { }
        }

        private async System.Threading.Tasks.Task IpcSendAiAsync(double sensitivity)
        {
            var client = _ipc as IpcClientService;
            var field = typeof(IpcClientService).GetField("_pipe", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
            var pipe = field?.GetValue(client) as System.IO.Pipes.NamedPipeClientStream;
            if (pipe == null || !pipe.IsConnected) return;
            using (var ms = new System.IO.MemoryStream())
            using (var bw = new System.IO.BinaryWriter(ms))
            {
                bw.Write((uint)(1 + sizeof(double)));
                bw.Write((byte)3); // op=3 config
                bw.Write(sensitivity);
                bw.Flush();
                var buffer = ms.ToArray();
                await pipe.WriteAsync(buffer, 0, buffer.Length);
                await pipe.FlushAsync();
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
