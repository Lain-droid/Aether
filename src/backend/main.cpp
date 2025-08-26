#include "Core.h"
#include <iostream>
#include <string>
#include <vector>

void ShowUsage() {
    std::cout << "AetherVisor Backend\n";
    std::cout << "Usage: AetherVisor.Backend.exe [command]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  --inject <process_name>   Injects the payload into the specified process.\n";
    std::cout << "  --help                      Show this help message.\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty() || args[0] == "--help") {
        ShowUsage();
        return 1;
    }

    auto& core = AetherVisor::Backend::Core::GetInstance();
    if (!core.Initialize()) {
        std::cerr << "Failed to initialize AetherVisor Core.\n";
        return 1;
    }

    // This is a simplified command-line parser.
    if (args[0] == "--inject" && args.size() > 1) {
        std::string processNameStr = args[1];
        std::wstring processName(processNameStr.begin(), processNameStr.end());
        std::cout << "Attempting to inject into " << processNameStr << "...\n";
        if (core.Inject(processName)) {
            std::cout << "Injection successful.\n";
        } else {
            std::cerr << "Injection failed.\n";
        }
    } else {
        ShowUsage();
        return 1;
    }

    std::cout << "AetherVisor Backend is running. Press Enter to exit.\n";
    std::cin.get();

    core.Cleanup();

    return 0;
}
