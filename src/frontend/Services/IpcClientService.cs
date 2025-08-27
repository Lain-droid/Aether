using System;
using System.IO;
using System.IO.Pipes;
using System.Text;
using System.Threading.Tasks;

namespace AetherVisor.Frontend.Services
{
    // Interface for the IPC client service for dependency injection
    public interface IIpcClientService
    {
        event Action<string> ConsoleMessageReceived;
        Task ConnectAsync();
        Task SendScriptAsync(string script);
        void Disconnect();
    }

    public class IpcClientService : IIpcClientService
    {
        public event Action<string> ConsoleMessageReceived;
        private NamedPipeClientStream _pipe;

        public Task ConnectAsync()
        {
            _pipe = new NamedPipeClientStream(".", "AetherPipe", PipeDirection.InOut, PipeOptions.Asynchronous);
            return _pipe.ConnectAsync(1500);
        }

        public Task SendScriptAsync(string script)
        {
            if (_pipe == null || !_pipe.IsConnected) throw new IOException("Pipe not connected");
            // protocol: [len(uint32)][op(1)][payload]
            byte[] payload = Encoding.UTF8.GetBytes(script ?? string.Empty);
            using (var ms = new MemoryStream())
            using (var bw = new BinaryWriter(ms))
            {
                bw.Write((uint)(1 + payload.Length));
                bw.Write((byte)2); // Execute opcode
                bw.Write(payload);
                bw.Flush();
                var buffer = ms.ToArray();
                _pipe.Write(buffer, 0, buffer.Length);
                _pipe.Flush();
            }
            return Task.CompletedTask;
        }

        public void Disconnect()
        {
            _pipe?.Dispose();
            _pipe = null;
        }

        // A private method would listen for incoming messages from the backend
        private void ListenForMessages()
        {
            // while(connected) {
            //   var message = ReadMessageFromPipe();
            //   ConsoleMessageReceived?.Invoke(message.Payload);
            // }
        }
    }
}
