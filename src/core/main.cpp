#include <windows.h>
#include <shellapi.h>
#include <string>
#include <memory>
#include "Engine.h"
#include "GDIRenderer.h"
#include "DX12Renderer.h"
#include "Logger.h"

// Renderer selection enum
enum class RendererType
{
    GDI,
    DirectX12
};

// Global variables
Engine* g_engine = nullptr;
HWND g_hwnd = nullptr;
RendererType g_selectedRenderer = RendererType::DirectX12; // Default renderer

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::unique_ptr<IRenderer> CreateRenderer(RendererType type);
RendererType SelectRendererFromCommandLine(int argc, char* argv[]);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Clear previous log
    Logger::ClearLog();
    Logger::Log("Application starting...");

    // Parse command line to select renderer
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Convert to char** for easier parsing
    char** argv = new char*[argc];
    for (int i = 0; i < argc; i++)
    {
        int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        argv[i] = new char[size];
        WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], size, nullptr, nullptr);
    }

    g_selectedRenderer = SelectRendererFromCommandLine(argc, argv);

    // Cleanup argv
    for (int i = 0; i < argc; i++)
        delete[] argv[i];
    delete[] argv;
    LocalFree(argvW);

    // Register window class
    const wchar_t CLASS_NAME[] = L"GraphicsEngineWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // No automatic background - renderers handle their own drawing

    RegisterClassW(&wc);

    // Create window
    RECT windowRect = { 0, 0, 1280, 720 };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    g_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Graphics Engine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (g_hwnd == nullptr)
    {
        return 0;
    }

    // Create engine with selected renderer
    Logger::Log("Creating engine and renderer...");
    g_engine = new Engine(1280, 720);

    try
    {
        auto renderer = CreateRenderer(g_selectedRenderer);
        g_engine->Initialize(g_hwnd, std::move(renderer));
        Logger::Log("Engine and renderer initialized successfully");
    }
    catch (const std::exception& e)
    {
        Logger::LogError(std::string("Failed to initialize: ") + e.what());
        MessageBoxA(g_hwnd, e.what(), "Initialization Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    Logger::Log("Showing window...");
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // Set up timer for continuous updates (only needed for GDI renderer)
    if (g_selectedRenderer == RendererType::GDI)
    {
        SetTimer(g_hwnd, 1, 16, nullptr); // ~60 FPS
    }

    Logger::Log("Entering message loop...");
    // Message loop
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Render frame
            if (g_engine)
            {
                try
                {
                    g_engine->Update();
                    g_engine->Render();
                }
                catch (const std::exception& e)
                {
                    Logger::LogError(std::string("Rendering error: ") + e.what());
                    PostQuitMessage(1);
                    break;
                }
            }

            // Small sleep to prevent CPU spinning at 100%
            Sleep(1);
        }
    }

    // Cleanup
    if (g_engine)
    {
        g_engine->OnDestroy();
        delete g_engine;
        g_engine = nullptr;
    }

    return (int)msg.wParam;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        // Press 'G' to switch to GDI, 'D' to switch to DirectX 12, ESC to quit
        if (wParam == 'G' || wParam == 'D')
        {
            RendererType newRenderer = (wParam == 'G') ? RendererType::GDI : RendererType::DirectX12;

            if (newRenderer != g_selectedRenderer)
            {
                Logger::Log("Switching renderer...");
                g_selectedRenderer = newRenderer;

                try
                {
                    // Switch renderer
                    auto renderer = CreateRenderer(g_selectedRenderer);
                    g_engine->SwitchRenderer(std::move(renderer));

                    // Manage timer
                    if (g_selectedRenderer == RendererType::GDI)
                        SetTimer(hwnd, 1, 16, nullptr);
                    else
                        KillTimer(hwnd, 1);

                    Logger::Log("Renderer switched successfully");
                }
                catch (const std::exception& e)
                {
                    Logger::LogError(std::string("Failed to switch renderer: ") + e.what());
                }
            }
        }
        else if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        return 0;

    case WM_PAINT:
        if (g_selectedRenderer == RendererType::GDI && g_engine)
        {
            g_engine->Update();
            g_engine->Render();
        }
        ValidateRect(hwnd, nullptr);
        return 0;

    case WM_TIMER:
        if (g_selectedRenderer == RendererType::GDI)
        {
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_ERASEBKGND:
        // Don't erase background - renderers handle their own drawing
        return 1;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Create renderer based on type
std::unique_ptr<IRenderer> CreateRenderer(RendererType type)
{
    switch (type)
    {
    case RendererType::GDI:
        Logger::Log("Creating GDI Renderer...");
        return std::make_unique<GDIRenderer>();

    case RendererType::DirectX12:
        Logger::Log("Creating DirectX 12 Renderer...");
        return std::make_unique<DX12Renderer>();

    default:
        return nullptr;
    }
}

// Parse command line arguments to select renderer
RendererType SelectRendererFromCommandLine(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--renderer=gdi" || arg == "-gdi")
        {
            return RendererType::GDI;
        }
        else if (arg == "--renderer=dx12" || arg == "-dx12")
        {
            return RendererType::DirectX12;
        }
        else if (arg == "--help" || arg == "-h")
        {
            MessageBoxA(nullptr,
                "Graphics Engine - Random Number Display\n\n"
                "Command line options:\n"
                "  --renderer=gdi or -gdi    : Use GDI renderer\n"
                "  --renderer=dx12 or -dx12  : Use DirectX 12 renderer (default)\n\n"
                "Runtime controls:\n"
                "  G : Switch to GDI renderer\n"
                "  D : Switch to DirectX 12 renderer\n"
                "  ESC : Exit application\n\n"
                "The random number updates every 5 seconds.",
                "Graphics Engine Help",
                MB_OK | MB_ICONINFORMATION);
        }
    }

    // Default to DirectX 12
    return RendererType::DirectX12;
}
