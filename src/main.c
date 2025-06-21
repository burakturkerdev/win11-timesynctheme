#include <windows.h>

#define LIGHT_THEME_START_HOUR 7
#define DARK_THEME_START_HOUR 19
#define APP_REGISTRY_NAME "AutoThemeChanger"

void AddToStartup() {
    char szPath[MAX_PATH];
    DWORD pathLen = GetModuleFileNameA(NULL, szPath, MAX_PATH);
    if (pathLen == 0) {
        return;
    }

    HKEY hKey;
    const char* keyPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    LONG openResult = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE, &hKey);
    if (openResult != ERROR_SUCCESS) {
        return;
    }

    RegSetValueExA(hKey, APP_REGISTRY_NAME, 0, REG_SZ, (const BYTE*)szPath, pathLen + 1);
    RegCloseKey(hKey);
}

BOOL IsInStartup() {
    HKEY hKey;
    const char* keyPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    BOOL found = FALSE;

    LONG openResult = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_QUERY_VALUE, &hKey);
    if (openResult == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, APP_REGISTRY_NAME, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            found = TRUE;
        }
        RegCloseKey(hKey);
    }
    return found;
}

void setTheme(BOOL isLight) {
    const char* keyPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
    const char* appValueName = "AppsUseLightTheme";
    const char* sysValueName = "SystemUsesLightTheme";
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return;
    }

    DWORD dwValue = isLight ? 1 : 0;

    RegSetValueExA(hKey, appValueName, 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
    RegSetValueExA(hKey, sysValueName, 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));

    RegCloseKey(hKey);

    SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"ImmersiveColorSet", SMTO_NORMAL, 1000, NULL);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!IsInStartup()) {
        AddToStartup();
    }
    
    int currentThemeState = -1; 

    while (1) {
        SYSTEMTIME st;
        GetLocalTime(&st);

        BOOL shouldBeLight = (st.wHour >= LIGHT_THEME_START_HOUR && st.wHour < DARK_THEME_START_HOUR);

        if (shouldBeLight != currentThemeState) {
            setTheme(shouldBeLight);
            currentThemeState = shouldBeLight;
        }

        Sleep(60000);
    }

    return 0;
}
