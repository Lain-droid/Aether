#include <windows.h>
#include <iostream>
#include <string>

class DependencyChecker {
private:
    bool CheckRegistry(const std::string& keyPath) {
        HKEY hKey;
        LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
        return false;
    }
    
    bool CheckFile(const std::string& filePath) {
        DWORD attributes = GetFileAttributesA(filePath.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES);
    }
    
    bool DownloadRuntime() {
        std::cout << "Downloading Visual C++ Redistributable..." << std::endl;
        
        // Use PowerShell for download
        std::string command = "powershell -WindowStyle Hidden -Command \"try { "
                             "Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vc_redist.x64.exe' "
                             "-OutFile 'vc_redist.x64.exe' -UseBasicParsing } catch { exit 1 }\"";
        
        int result = system(command.c_str());
        if (result != 0) {
            std::cout << "Download failed. Check internet connection." << std::endl;
            return false;
        }
        
        // Install silently
        std::cout << "Installing runtime..." << std::endl;
        result = system("vc_redist.x64.exe /quiet /norestart");
        
        // Cleanup
        DeleteFileA("vc_redist.x64.exe");
        
        return (result == 0);
    }
    
public:
    void CheckDependencies() {
        std::cout << "Aether System Check" << std::endl;
        std::cout << "==================" << std::endl << std::endl;
        
        // Check Visual C++ Runtime
        bool hasRuntime = CheckRegistry("SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64");
        if (hasRuntime) {
            std::cout << "[OK] Visual C++ Runtime" << std::endl;
        } else {
            std::cout << "[MISSING] Visual C++ Runtime" << std::endl;
            if (DownloadRuntime()) {
                std::cout << "[OK] Runtime installed" << std::endl;
            } else {
                std::cout << "[ERROR] Installation failed" << std::endl;
            }
        }
        
        // Check Universal CRT
        bool hasUCRT = CheckFile("C:\\Windows\\System32\\ucrtbase.dll");
        if (hasUCRT) {
            std::cout << "[OK] Universal CRT" << std::endl;
        } else {
            std::cout << "[WARNING] Universal CRT missing" << std::endl;
        }
        
        // Check Common Controls
        bool hasComCtl = CheckFile("C:\\Windows\\System32\\comctl32.dll");
        if (hasComCtl) {
            std::cout << "[OK] Common Controls" << std::endl;
        } else {
            std::cout << "[ERROR] Common Controls missing" << std::endl;
        }
        
        std::cout << std::endl << "System check complete." << std::endl;
        std::cout << "Press any key to continue..." << std::endl;
        std::cin.get();
    }
};

int main() {
    DependencyChecker checker;
    checker.CheckDependencies();
    return 0;
}
