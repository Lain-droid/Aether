using System.Threading.Tasks;

namespace AetherVisor.Frontend.Services
{
    public interface ISetupService
    {
        Task<bool> PrepareIpcAsync();
        Task<bool> ValidateResourcesAsync();
        Task<bool> InitializePolyEngineAsync();
        Task<bool> SimulateInjectAsync();
    }

    public class SetupService : ISetupService
    {
        public Task<bool> PrepareIpcAsync() => Task.FromResult(true);
        public Task<bool> ValidateResourcesAsync() => Task.FromResult(true);
        public Task<bool> InitializePolyEngineAsync() => Task.FromResult(true);
        public Task<bool> SimulateInjectAsync() => Task.FromResult(true);
    }
}
