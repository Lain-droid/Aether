using AetherVisor.Backend; // Assuming we have a shared reference to the backend's enums
using System;
using System.IO.Pipes;
using System.Text;
using System.Threading.Tasks;

namespace AetherVisor.Frontend.Services
{
    public interface IIpcClientService
    {
        event Action<string> OnConsoleOutputReceived;
        event Action<string> OnStatusUpdated;
        event Action<string> OnAnalysisResultReceived;
        Task ConnectAndInject();
        void ExecuteScript(string script);
        void AnalyzeScript(string script);
    }

    public class IpcClientService : IIpcClientService
    {
        public event Action<string> OnConsoleOutputReceived;
        public event Action<string> OnStatusUpdated;
        public event Action<string> OnAnalysisResultReceived;

        private NamedPipeClientStream _pipeClient;
        private Task _listenerTask;

        public IpcClientService()
        {
            _pipeClient = new NamedPipeClientStream(".", "AetherVisor_Session_Pipe", PipeDirection.InOut, PipeOptions.Asynchronous);
        }

        public async Task ConnectAndInject()
        {
            try
            {
                await _pipeClient.ConnectAsync(5000);
                OnStatusUpdated?.Invoke("Connected. Injecting...");
                // In a real app, we'd send an inject message. For now, we'll assume it's injected.
                OnStatusUpdated?.Invoke("Injected");
                _listenerTask = Task.Run(ListenForMessages);
            }
            catch (Exception ex)
            {
                OnStatusUpdated?.Invoke($"Connection failed: {ex.Message}");
            }
        }

        public void ExecuteScript(string script)
        {
            var message = new IpcMessage { Type = MessageType.ExecuteScript, Payload = script };
            SendMessage(message);
        }

        public void AnalyzeScript(string script)
        {
            var message = new IpcMessage { Type = MessageType.AnalyzeScriptRequest, Payload = script };
            SendMessage(message);
        }

        private void SendMessage(IpcMessage message)
        {
            if (!_pipeClient.IsConnected) return;
            // This needs to use the same StegoPacket serialization as the backend.
            // For simplicity, we send raw bytes here.
            // var stegoPacket = PackMessage(message);
            // var buffer = stegoPacket.Serialize();
            var buffer = Encoding.UTF8.GetBytes($"{message.Type}:{message.Payload}"); // Simplified protocol
            _pipeClient.Write(buffer, 0, buffer.Length);
        }

        private async void ListenForMessages()
        {
            var buffer = new byte[8192];
            while (_pipeClient.IsConnected)
            {
                try
                {
                    var bytesRead = await _pipeClient.ReadAsync(buffer, 0, buffer.Length);
                    if (bytesRead > 0)
                    {
                        // Here we would deserialize the StegoPacket.
                        // For simplicity, we parse the simplified protocol.
                        var rawMessage = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                        var parts = rawMessage.Split(new[] { ':' }, 2);
                        var messageType = (MessageType)Enum.Parse(typeof(MessageType), parts[0]);
                        var payload = parts[1];

                        switch (messageType)
                        {
                            case MessageType.ConsoleOutput:
                                OnConsoleOutputReceived?.Invoke(payload);
                                break;
                            case MessageType.StatusUpdate:
                                OnStatusUpdated?.Invoke(payload);
                                break;
                            case MessageType.AnalyzeScriptResponse:
                                OnAnalysisResultReceived?.Invoke(payload);
                                break;
                        }
                    }
                }
                catch { /* Pipe closed or error */ break; }
            }
        }
    }
}
