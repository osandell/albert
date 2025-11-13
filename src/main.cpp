// Copyright (C) 2024-2025 Manuel Schneider

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
using namespace std;
namespace albert {
extern int run(int, char **);
}

// Helper function to write error to file
void writeErrorToFile(const string& error) {
    try {
        // Try to write to the same directory as the executable
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        string exeDir = string(exePath);
        size_t lastSlash = exeDir.find_last_of("\\/");
        if (lastSlash != string::npos) {
            exeDir = exeDir.substr(0, lastSlash + 1);
        }
        string logPath = exeDir + "albert_debug.log";
        ofstream logFile(logPath, ios::app);
        if (logFile.is_open()) {
            logFile << "\n=== Early Error (before Qt init) ===\n";
            logFile << error << "\n";
            logFile.close();
        }
    } catch (...) {
        // If we can't write to file, at least try stderr
        cerr << "Failed to write error to log file: " << error << endl;
    }
}

int main(int argc, char **argv)
{
    try {
        return albert::run(argc, argv);
    } catch (const exception &e) {
        string error = string("Exception in main: ") + e.what();
        writeErrorToFile(error);
        cout << error << endl;
    } catch (...) {
        string error = "Unknown exception in main!";
        writeErrorToFile(error);
        cout << error << endl;
    }
    return EXIT_FAILURE;
}
