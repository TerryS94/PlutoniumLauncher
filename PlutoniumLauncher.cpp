#include "PlutoniumLauncher.h"

static HBRUSH hbrDarkBackground;
static HBRUSH hbrControlBackground;
static COLORREF crText = RGB(220, 220, 220);

static void DisableTheme(HWND hCtrl)
{
    SetWindowTheme(hCtrl, L"", L"");
}
struct Vec2
{
    float x, y;
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
std::string WAW_FOLDER = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty World at War";
std::string BO1_FOLDER = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty Black Ops";
std::string BO2_FOLDER = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty Black Ops II";
std::string MW3_FOLDER = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty Modern Warfare 3";
std::string USERNAME = "";
std::string LAUNCH_TARGET_PROCESS = "";
std::string LAUNCH_GAME_PATH = "";

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
    {
        configFile << config.dump(4);
        configFile.close();
    }
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
    std::string cmd = std::format("\"{}\" {} \"{}\" +name \"{}\" -lan", BOOTSTRAPPER, LAUNCH_TARGET_PROCESS, LAUNCH_GAME_PATH, USERNAME);
    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, plutoniumPath.c_str(), &si, &pi))
    {
        MessageBoxA(nullptr, std::format("Failed to launch {}.exe.\nCheck your paths.", LAUNCH_TARGET_PROCESS).c_str(), "Launcher Error", MB_OK | MB_ICONERROR);
        return;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    isRunning = false;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        if (!std::filesystem::exists(plutoniumPath)) plutoniumPath.clear();
        if (!std::filesystem::exists(WAW_FOLDER)) WAW_FOLDER.clear();
        if (!std::filesystem::exists(BO1_FOLDER)) BO1_FOLDER.clear();
        if (!std::filesystem::exists(BO2_FOLDER)) BO2_FOLDER.clear();
        if (!std::filesystem::exists(MW3_FOLDER)) MW3_FOLDER.clear();

        CreateWindowA("STATIC", "Username",
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            10, 10, 100, 20, hWnd, nullptr, nullptr, nullptr);
        HWND hUser = CreateWindowA("EDIT", USERNAME.c_str(),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 10, 250, 20, hWnd, (HMENU)IDC_USERNAME_EDIT, nullptr, nullptr);
        SendMessageA(hUser, EM_SETLIMITTEXT, 24, 0);
        DisableTheme(hUser);

        CreateWindowA("STATIC", "Made By Terry",
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            535, 10, 100, 20, hWnd, nullptr, nullptr, nullptr);

        CreateWindowA("STATIC", "Plutonium",
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            10, 40, 100, 20, hWnd, nullptr, nullptr, nullptr);
        HWND hPluto = CreateWindowA("EDIT", plutoniumPath.c_str(),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 40, 450, 20, hWnd, (HMENU)IDC_EDIT_PLUTONIUM, nullptr, nullptr);
        SendMessageA(hPluto, EM_SETLIMITTEXT, 256, 0);
        DisableTheme(hPluto);
        HWND btnPluto = CreateWindowA("BUTTON", "Browse",
            WS_VISIBLE | WS_CHILD,
            555, 38, 80, 24, hWnd, (HMENU)IDC_BUTTON_BROWSE_PLUTONIUM, nullptr, nullptr);
        DisableTheme(btnPluto);

        CreateWindowA("STATIC", "World at War",
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            10, 70, 120, 20, hWnd, nullptr, nullptr, nullptr);
        HWND hWaW = CreateWindowA("EDIT", WAW_FOLDER.c_str(),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 70, 300, 20, hWnd, (HMENU)IDC_EDIT_WAW, nullptr, nullptr);
        SendMessageA(hWaW, EM_SETLIMITTEXT, 256, 0);
        DisableTheme(hWaW);
        HWND btnWaWB = CreateWindowA("BUTTON", "Browse",
            WS_VISIBLE | WS_CHILD,
            405, 68, 60, 24, hWnd, (HMENU)IDC_BUTTON_BROWSE_WAW, nullptr, nullptr);
        DisableTheme(btnWaWB);
        HWND btnWaWSP = CreateWindowA("BUTTON", "Launch SP",
            WS_VISIBLE | WS_CHILD,
            470, 68, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_T4_SP, nullptr, nullptr);
        DisableTheme(btnWaWSP);
        HWND btnWaWMP = CreateWindowA("BUTTON", "Launch MP",
            WS_VISIBLE | WS_CHILD,
            555, 68, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_T4_MP, nullptr, nullptr);
        DisableTheme(btnWaWMP);

        CreateWindowA("STATIC", "Black Ops I",
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            10, 100, 120, 20, hWnd, nullptr, nullptr, nullptr);
        HWND hBO1 = CreateWindowA("EDIT", BO1_FOLDER.c_str(),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 100, 300, 20, hWnd, (HMENU)IDC_EDIT_BO1, nullptr, nullptr);
        SendMessageA(hBO1, EM_SETLIMITTEXT, 256, 0);
        DisableTheme(hBO1);
        HWND btnBO1B = CreateWindowA("BUTTON", "Browse",
            WS_VISIBLE | WS_CHILD,
            405, 98, 60, 24, hWnd, (HMENU)IDC_BUTTON_BROWSE_BO1, nullptr, nullptr);
        DisableTheme(btnBO1B);
        HWND btnBO1SP = CreateWindowA("BUTTON", "Launch SP",
            WS_VISIBLE | WS_CHILD,
            470, 98, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_T5_SP, nullptr, nullptr);
        DisableTheme(btnBO1SP);
        HWND btnBO1MP = CreateWindowA("BUTTON", "Launch MP",
            WS_VISIBLE | WS_CHILD,
            555, 98, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_T5_MP, nullptr, nullptr);
        DisableTheme(btnBO1MP);

        CreateWindowA("STATIC", "Black Ops II",
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            10, 130, 120, 20, hWnd, nullptr, nullptr, nullptr);
        HWND hBO2 = CreateWindowA("EDIT", BO2_FOLDER.c_str(),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 130, 300, 20, hWnd, (HMENU)IDC_EDIT_BO2, nullptr, nullptr);
        SendMessageA(hBO2, EM_SETLIMITTEXT, 256, 0);
        DisableTheme(hBO2);
        HWND btnBO2B = CreateWindowA("BUTTON", "Browse",
            WS_VISIBLE | WS_CHILD,
            405, 128, 60, 24, hWnd, (HMENU)IDC_BUTTON_BROWSE_BO2, nullptr, nullptr);
        DisableTheme(btnBO2B);
        HWND btnBO2ZM = CreateWindowA("BUTTON", "Launch ZM",
            WS_VISIBLE | WS_CHILD,
            470, 128, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_T6_ZM, nullptr, nullptr);
        DisableTheme(btnBO2ZM);
        HWND btnBO2MP = CreateWindowA("BUTTON", "Launch MP",
            WS_VISIBLE | WS_CHILD,
            555, 128, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_T6_MP, nullptr, nullptr);
        DisableTheme(btnBO2MP);

        CreateWindowA("STATIC", "MW3",
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            10, 160, 140, 20, hWnd, nullptr, nullptr, nullptr);
        HWND hMW3 = CreateWindowA("EDIT", MW3_FOLDER.c_str(),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            100, 160, 300, 20, hWnd, (HMENU)IDC_EDIT_MW3, nullptr, nullptr);
        SendMessageA(hMW3, EM_SETLIMITTEXT, 256, 0);
        DisableTheme(hMW3);
        HWND btnMW3B = CreateWindowA("BUTTON", "Browse",
            WS_VISIBLE | WS_CHILD,
            405, 158, 60, 24, hWnd, (HMENU)IDC_BUTTON_BROWSE_MW3, nullptr, nullptr);
        DisableTheme(btnMW3B);
        HWND btnMW3SP = CreateWindowA("BUTTON", "Launch SP",
            WS_VISIBLE | WS_CHILD,
            470, 158, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_IW5_SP, nullptr, nullptr);
        DisableTheme(btnMW3SP);
        HWND btnMW3MP = CreateWindowA("BUTTON", "Launch MP",
            WS_VISIBLE | WS_CHILD,
            555, 158, 80, 24, hWnd, (HMENU)IDC_BUTTON_LAUNCH_IW5_MP, nullptr, nullptr);
        DisableTheme(btnMW3MP);

        return 0;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);
        if (code == EN_CHANGE)
        {
            char buf[MAX_PATH];
            switch (id)
            {
            case IDC_USERNAME_EDIT:
                GetDlgItemTextA(hWnd, IDC_USERNAME_EDIT, buf, _countof(buf));
                USERNAME = buf;
                break;
            case IDC_EDIT_PLUTONIUM:
                GetDlgItemTextA(hWnd, IDC_EDIT_PLUTONIUM, buf, _countof(buf));
                plutoniumPath = buf;
                break;
            case IDC_EDIT_WAW:
                GetDlgItemTextA(hWnd, IDC_EDIT_WAW, buf, _countof(buf));
                WAW_FOLDER = buf;
                break;
            case IDC_EDIT_BO1:
                GetDlgItemTextA(hWnd, IDC_EDIT_BO1, buf, _countof(buf));
                BO1_FOLDER = buf;
                break;
            case IDC_EDIT_BO2:
                GetDlgItemTextA(hWnd, IDC_EDIT_BO2, buf, _countof(buf));
                BO2_FOLDER = buf;
                break;
            case IDC_EDIT_MW3:
                GetDlgItemTextA(hWnd, IDC_EDIT_MW3, buf, _countof(buf));
                MW3_FOLDER = buf;
                break;
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
            BROWSEINFOA bi{};
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
            BROWSEINFOA bi{};
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
            BROWSEINFOA bi{};
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
            BROWSEINFOA bi{}; bi.hwndOwner = hWnd; bi.lpszTitle = "Select Black Ops II folder";
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
            BROWSEINFOA bi{}; bi.hwndOwner = hWnd; bi.lpszTitle = "Select MW3 folder";
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
        }
        return 0;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN:
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, crText);
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)hbrControlBackground;
    }
    case WM_MOVE:
        windowPosition = Vec2((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
        break;
    case WM_MOVING:
    {
        RECT* r = (RECT*)lParam;
        windowPosition = Vec2((float)r->left, (float)r->top);
        break;
    }
    case WM_DESTROY:
        isRunning = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    hbrDarkBackground = CreateSolidBrush(RGB(30, 30, 30));
    hbrControlBackground = CreateSolidBrush(RGB(45, 45, 45));

    WNDCLASSA wc{};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "CustomPlutoniumLauncher";
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconA(hInst, MAKEINTRESOURCEA(IDI_SMALL));
    wc.hbrBackground = hbrDarkBackground;
    RegisterClassA(&wc);

    loadConfig();

    HWND hwnd = CreateWindowA(wc.lpszClassName, "Plutonium Launcher", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
        (int)windowPosition.x, (int)windowPosition.y,
        660, 230, nullptr, nullptr, hInst, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (isRunning && GetMessageA(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    saveConfig();
    DeleteObject(hbrDarkBackground);
    DeleteObject(hbrControlBackground);
	UnregisterClassA(wc.lpszClassName, hInst);
	DestroyWindow(hwnd);
    return 0;
}
