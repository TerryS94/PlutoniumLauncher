#include "framework.h"
#include <shlobj.h>
#include <shellapi.h>
#include <string>
#include <cstdlib>
#include <memory>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "PlutoniumLauncher.h"

struct Vec2
{
    float x;
    float y;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Vec2, x, y);
};

static std::string GetLocalAppDataPath()
{
    char* raw = nullptr;
    size_t len = 0;
    if (_dupenv_s(&raw, &len, "LOCALAPPDATA") != 0 || raw == nullptr)
        return "";
    std::unique_ptr<char, decltype(&free)> env(raw, free);
    return std::string(env.get());
}

bool isRunning = true;

Vec2 windowPosition = { 100.0f, 100.0f };
std::string plutoniumPath = GetLocalAppDataPath() + "\\Plutonium";
const std::string BOOTSTRAPPER = plutoniumPath + "\\bin\\plutonium-bootstrapper-win32.exe";
std::string WAW_FOLDER = "";
std::string BO1_FOLDER = "";
std::string BO2_FOLDER = "";
std::string MW3_FOLDER = "";

std::string USERNAME = "";//the username to use on launch
std::string LAUNCH_TARGET_PROCESS = "";//can be iw5mp, iw5sp, t6mp, t6zm, t5mp, t5sp, t4mp, t4sp
std::string LAUNCH_GAME_PATH = "";//the full path of the true game folder, not the Plutonium folder

constexpr int IDC_USERNAME_EDIT = 1001;
constexpr int IDC_EDIT_PLUTONIUM = 1002;
constexpr int IDC_BUTTON_BROWSE_PLUTONIUM = 1003;
constexpr int IDC_EDIT_WAW = 1004;
constexpr int IDC_BUTTON_BROWSE_WAW = 1005;
constexpr int IDC_BUTTON_LAUNCH_T4_SP = 1006;
constexpr int IDC_BUTTON_LAUNCH_T4_MP = 1007;
constexpr int IDC_EDIT_BO1 = 1008;
constexpr int IDC_BUTTON_BROWSE_BO1 = 1009;
constexpr int IDC_BUTTON_LAUNCH_T5_SP = 1010;
constexpr int IDC_BUTTON_LAUNCH_T5_MP = 1011;
constexpr int IDC_EDIT_BO2 = 1012;
constexpr int IDC_BUTTON_BROWSE_BO2 = 1013;
constexpr int IDC_BUTTON_LAUNCH_T6_ZM = 1014;
constexpr int IDC_BUTTON_LAUNCH_T6_MP = 1015;
constexpr int IDC_EDIT_MW3 = 1016;
constexpr int IDC_BUTTON_BROWSE_MW3 = 1017;
constexpr int IDC_BUTTON_LAUNCH_IW5_SP = 1018;
constexpr int IDC_BUTTON_LAUNCH_IW5_MP = 1019;
constexpr int IDC_BUTTON_CLOSE = 1020;
constexpr int IDC_BUTTON_HELP = 1021;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static void loadConfig()
{
    std::ifstream configFile("config.json");
    if (!configFile.is_open())
        return;
    nlohmann::json config;
    configFile >> config;
    USERNAME = config.value("username", "");
    plutoniumPath = config.value("plutoniumPath", plutoniumPath);
    WAW_FOLDER = config.value("wawFolder", WAW_FOLDER);
    BO1_FOLDER = config.value("bo1Folder", BO1_FOLDER);
    BO2_FOLDER = config.value("bo2Folder", BO2_FOLDER);
    MW3_FOLDER = config.value("mw3Folder", MW3_FOLDER);
    windowPosition = config.value("windowPosition", windowPosition);
    configFile.close();
}
static void saveConfig()
{
    nlohmann::json config;
    config["username"] = USERNAME;
    config["plutoniumPath"] = plutoniumPath;
    config["wawFolder"] = WAW_FOLDER;
    config["bo1Folder"] = BO1_FOLDER;
    config["bo2Folder"] = BO2_FOLDER;
    config["mw3Folder"] = MW3_FOLDER;
    config["windowPosition"] = windowPosition;
    std::ofstream configFile("config.json");
    if (configFile.is_open())
        configFile << config.dump(4);

    configFile.close();
}

static void LaunchGame()
{
    if (!std::filesystem::exists(plutoniumPath))
    {
        MessageBoxA(nullptr, "Plutonium folder not found.", "Launcher Error", MB_OK | MB_ICONERROR);
        return;
    }
    if (!std::filesystem::exists(BOOTSTRAPPER))
    {
        MessageBoxA(nullptr, "Bootstrapper not found.", "Launcher Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::string cmd = "\"" + BOOTSTRAPPER + "\""
        + " " + LAUNCH_TARGET_PROCESS
        + " \"" + LAUNCH_GAME_PATH + "\""
        + " +name \"" + USERNAME + "\""
        + " -lan";

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcess(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, plutoniumPath.c_str(), &si, &pi))
    {
        MessageBoxA(nullptr, std::format("Failed to launch {}.exe.\nCheck your paths and working directory.", LAUNCH_TARGET_PROCESS).c_str(), "Launcher Error", MB_OK | MB_ICONERROR);
        return;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    isRunning = false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        if (!std::filesystem::exists(plutoniumPath))
            plutoniumPath.clear();
        if (!std::filesystem::exists(WAW_FOLDER))
            WAW_FOLDER.clear();
        if (!std::filesystem::exists(BO1_FOLDER))
            BO1_FOLDER.clear();
        if (!std::filesystem::exists(BO2_FOLDER))
            BO2_FOLDER.clear();
        if (!std::filesystem::exists(MW3_FOLDER))
            MW3_FOLDER.clear();

        CreateWindowA("STATIC", "Username", WS_VISIBLE | WS_CHILD,
            10, 10, 100, 20,
            hWnd, nullptr, nullptr, nullptr
        );
        HWND hUser = CreateWindowA(
            "EDIT", USERNAME.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 10, 250, 20,
            hWnd, (HMENU)IDC_USERNAME_EDIT, nullptr, nullptr
        );
        SendMessageA(hUser, EM_SETLIMITTEXT, 24, 0);

        CreateWindowA("STATIC", "Made By Terry", WS_VISIBLE | WS_CHILD,
            535, 10, 100, 20,
            hWnd, nullptr, nullptr, nullptr
        );

        CreateWindowA("STATIC", "Plutonium", WS_VISIBLE | WS_CHILD,
            10, 40, 100, 20,
            hWnd, nullptr, nullptr, nullptr
        );
        HWND hPluto = CreateWindowA("EDIT", plutoniumPath.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 40, 450, 20,
            hWnd, (HMENU)IDC_EDIT_PLUTONIUM, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Browse", WS_VISIBLE | WS_CHILD,
            555, 38, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_BROWSE_PLUTONIUM, nullptr, nullptr
        );
        SendMessageA(hPluto, EM_SETLIMITTEXT, 256, 0);

        CreateWindowA("STATIC", "World at War", WS_VISIBLE | WS_CHILD,
            10, 70, 120, 20,
            hWnd, nullptr, nullptr, nullptr
        );
        HWND hWaW = CreateWindowA("EDIT", WAW_FOLDER.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 70, 300, 20,
            hWnd, (HMENU)IDC_EDIT_WAW, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Browse", WS_VISIBLE | WS_CHILD,
            405, 68, 60, 24,
            hWnd, (HMENU)IDC_BUTTON_BROWSE_WAW, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch SP", WS_VISIBLE | WS_CHILD,
            470, 68, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_T4_SP, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch MP", WS_VISIBLE | WS_CHILD,
            555, 68, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_T4_MP, nullptr, nullptr
        );
        SendMessageA(hWaW, EM_SETLIMITTEXT, 256, 0);

        CreateWindowA("STATIC", "Black Ops I", WS_VISIBLE | WS_CHILD,
            10, 100, 120, 20,
            hWnd, nullptr, nullptr, nullptr
        );
        HWND hBO1 = CreateWindowA("EDIT", BO1_FOLDER.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 100, 300, 20,
            hWnd, (HMENU)IDC_EDIT_BO1, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Browse", WS_VISIBLE | WS_CHILD,
            405, 98, 60, 24,
            hWnd, (HMENU)IDC_BUTTON_BROWSE_BO1, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch SP", WS_VISIBLE | WS_CHILD,
            470, 98, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_T5_SP, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch MP", WS_VISIBLE | WS_CHILD,
            555, 98, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_T5_MP, nullptr, nullptr
        );
        SendMessageA(hBO1, EM_SETLIMITTEXT, 256, 0);

        CreateWindowA("STATIC", "Black Ops II", WS_VISIBLE | WS_CHILD,
            10, 130, 120, 20,
            hWnd, nullptr, nullptr, nullptr
        );
        HWND hBO2 = CreateWindowA("EDIT", BO2_FOLDER.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 130, 300, 20,
            hWnd, (HMENU)IDC_EDIT_BO2, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Browse", WS_VISIBLE | WS_CHILD,
            405, 128, 60, 24,
            hWnd, (HMENU)IDC_BUTTON_BROWSE_BO2, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch ZM", WS_VISIBLE | WS_CHILD,
            470, 128, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_T6_ZM, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch MP", WS_VISIBLE | WS_CHILD,
            555, 128, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_T6_MP, nullptr, nullptr
        );
        SendMessageA(hBO2, EM_SETLIMITTEXT, 256, 0);

        CreateWindowA("STATIC", "MW3", WS_VISIBLE | WS_CHILD,
            10, 160, 140, 20,
            hWnd, nullptr, nullptr, nullptr
        );
        HWND hMW3 = CreateWindowA("EDIT", MW3_FOLDER.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 160, 300, 20,
            hWnd, (HMENU)IDC_EDIT_MW3, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Browse", WS_VISIBLE | WS_CHILD,
            405, 158, 60, 24,
            hWnd, (HMENU)IDC_BUTTON_BROWSE_MW3, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch SP", WS_VISIBLE | WS_CHILD,
            470, 158, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_IW5_SP, nullptr, nullptr
        );
        CreateWindowA("BUTTON", "Launch MP", WS_VISIBLE | WS_CHILD,
            555, 158, 80, 24,
            hWnd, (HMENU)IDC_BUTTON_LAUNCH_IW5_MP, nullptr, nullptr
        );
        SendMessageA(hMW3, EM_SETLIMITTEXT, 256, 0);

        return 0;
    }
    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        if (code == EN_CHANGE)
        {
            if (id == IDC_USERNAME_EDIT)
            {
                char buf[64];
                GetDlgItemTextA(hWnd, IDC_USERNAME_EDIT, buf, _countof(buf));
                USERNAME = buf;
            }
            else if (id == IDC_EDIT_PLUTONIUM)
            {
                char buf[MAX_PATH];
                GetDlgItemTextA(hWnd, IDC_EDIT_PLUTONIUM, buf, _countof(buf));
                plutoniumPath = buf;
            }
            else if (id == IDC_EDIT_WAW)
            {
                char buf[MAX_PATH];
                GetDlgItemTextA(hWnd, IDC_EDIT_WAW, buf, _countof(buf));
                WAW_FOLDER = buf;
            }
            else if (id == IDC_EDIT_BO1)
            {
                char buf[MAX_PATH];
                GetDlgItemTextA(hWnd, IDC_EDIT_BO1, buf, _countof(buf));
                BO1_FOLDER = buf;
            }
            else if (id == IDC_EDIT_BO2)
            {
                char buf[MAX_PATH];
                GetDlgItemTextA(hWnd, IDC_EDIT_BO2, buf, _countof(buf));
                BO2_FOLDER = buf;
            }
            else if (id == IDC_EDIT_MW3)
            {
                char buf[MAX_PATH];
                GetDlgItemTextA(hWnd, IDC_EDIT_MW3, buf, _countof(buf));
                MW3_FOLDER = buf;
            }
        }

        switch (id)
        {
        case IDC_BUTTON_LAUNCH_T4_SP:
            LAUNCH_TARGET_PROCESS = "t4sp";
            LAUNCH_GAME_PATH = WAW_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_LAUNCH_T4_MP:
            LAUNCH_TARGET_PROCESS = "t4mp";
            LAUNCH_GAME_PATH = WAW_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_LAUNCH_T5_SP:
            LAUNCH_TARGET_PROCESS = "t5sp";
            LAUNCH_GAME_PATH = BO1_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_LAUNCH_T5_MP:
            LAUNCH_TARGET_PROCESS = "t5mp";
            LAUNCH_GAME_PATH = BO1_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_LAUNCH_T6_ZM:
            LAUNCH_TARGET_PROCESS = "t6zm";
            LAUNCH_GAME_PATH = BO2_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_LAUNCH_T6_MP:
            LAUNCH_TARGET_PROCESS = "t6mp";
            LAUNCH_GAME_PATH = BO2_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_LAUNCH_IW5_SP:
            LAUNCH_TARGET_PROCESS = "iw5sp";
            LAUNCH_GAME_PATH = MW3_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_LAUNCH_IW5_MP:
            LAUNCH_TARGET_PROCESS = "iw5mp";
            LAUNCH_GAME_PATH = MW3_FOLDER;
            LaunchGame();
            break;

        case IDC_BUTTON_BROWSE_PLUTONIUM:
        {
            BROWSEINFOA bi = {};
            bi.hwndOwner = hWnd;
            bi.lpszTitle = "Select Plutonium folder";
            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl)
            {
                char path[MAX_PATH];
                SHGetPathFromIDListA(pidl, path);
                SetDlgItemTextA(hWnd, IDC_EDIT_PLUTONIUM, path);
                CoTaskMemFree(pidl);
            }
            break;
        }

        case IDC_BUTTON_BROWSE_WAW:
        {
            BROWSEINFOA bi = {};
            bi.hwndOwner = hWnd;
            bi.lpszTitle = "Select World at War folder";
            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl)
            {
                char path[MAX_PATH];
                SHGetPathFromIDListA(pidl, path);
                SetDlgItemTextA(hWnd, IDC_EDIT_WAW, path);
                CoTaskMemFree(pidl);
            }
            break;
        }

        case IDC_BUTTON_BROWSE_BO1:
        {
            BROWSEINFOA bi = {};
            bi.hwndOwner = hWnd;
            bi.lpszTitle = "Select Black Ops I folder";
            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl)
            {
                char path[MAX_PATH];
                SHGetPathFromIDListA(pidl, path);
                SetDlgItemTextA(hWnd, IDC_EDIT_BO1, path);
                CoTaskMemFree(pidl);
            }
            break;
        }

        case IDC_BUTTON_BROWSE_BO2:
        {
            BROWSEINFOA bi = {};
            bi.hwndOwner = hWnd;
            bi.lpszTitle = "Select Black Ops II folder";
            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl)
            {
                char path[MAX_PATH];
                SHGetPathFromIDListA(pidl, path);
                SetDlgItemTextA(hWnd, IDC_EDIT_BO2, path);
                CoTaskMemFree(pidl);
            }
            break;
        }

        case IDC_BUTTON_BROWSE_MW3:
        {
            BROWSEINFOA bi = {};
            bi.hwndOwner = hWnd;
            bi.lpszTitle = "Select MW3 folder";
            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl)
            {
                char path[MAX_PATH];
                SHGetPathFromIDListA(pidl, path);
                SetDlgItemTextA(hWnd, IDC_EDIT_MW3, path);
                CoTaskMemFree(pidl);
            }
            break;
        }
        }//end inner switch(id)

        return 0;
    }
    case WM_MOVE:
    {
        windowPosition = Vec2((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        break;
    }

    case WM_MOVING:
    {
        RECT* pRect = (RECT*)lParam;
        windowPosition = Vec2(pRect->left, pRect->top);
        break;
    }
    case WM_DESTROY:
        isRunning = false;
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }
}
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "CustomPlutoniumLauncher";
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCEA(IDI_SMALL));
    RegisterClassA(&wc);
    loadConfig();
    HWND hwnd = CreateWindowA(wc.lpszClassName, "Plutonium Launcher",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        windowPosition.x, windowPosition.y, 660, 230,
        nullptr, nullptr, hInst, nullptr);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (isRunning && GetMessageA(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    saveConfig();
    DestroyWindow(hwnd);
    return 0;
}