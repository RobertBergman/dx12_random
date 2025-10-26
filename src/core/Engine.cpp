#include "Engine.h"
#include <string>

Engine::Engine(UINT width, UINT height)
    : m_hwnd(nullptr)
    , m_width(width)
    , m_height(height)
    , m_randomNumber(0)
{
    std::random_device rd;
    m_rng.seed(rd());
    UpdateRandomNumber();
    m_lastUpdateTime = std::chrono::steady_clock::now();
}

Engine::~Engine()
{
}

void Engine::Initialize(HWND hwnd, std::unique_ptr<IRenderer> renderer)
{
    m_hwnd = hwnd;
    m_renderer = std::move(renderer);
    m_renderer->Initialize(hwnd, m_width, m_height);
}

void Engine::Update()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastUpdateTime);

    if (elapsed.count() >= 5)
    {
        UpdateRandomNumber();
        m_lastUpdateTime = now;
    }
}

void Engine::Render()
{
    if (!m_renderer)
        return;

    RenderScene();
}

void Engine::OnDestroy()
{
    if (m_renderer)
        m_renderer->OnDestroy();
}

const char* Engine::GetRendererName() const
{
    if (m_renderer)
        return m_renderer->GetName();
    return "No Renderer";
}

void Engine::SwitchRenderer(std::unique_ptr<IRenderer> newRenderer)
{
    if (m_renderer)
        m_renderer->OnDestroy();

    m_renderer = std::move(newRenderer);
    m_renderer->Initialize(m_hwnd, m_width, m_height);

    // Force immediate redraw
    InvalidateRect(m_hwnd, nullptr, TRUE);
    UpdateWindow(m_hwnd);
}

void Engine::UpdateRandomNumber()
{
    std::uniform_int_distribution<int> dist(0, 9999);
    m_randomNumber = dist(m_rng);

    // Update window title with the random number
    if (m_hwnd)
    {
        std::string rendererName = GetRendererName();
        std::wstring title = L"Graphics Engine - ";
        title += std::wstring(rendererName.begin(), rendererName.end());
        title += L" - Random Number: " + std::to_wstring(m_randomNumber);
        SetWindowTextW(m_hwnd, title.c_str());
    }
}

void Engine::RenderScene()
{
    // Calculate color based on random number
    float r = 0.3f + (m_randomNumber % 100) / 300.0f;
    float g = 0.4f + ((m_randomNumber / 10) % 100) / 300.0f;
    float b = 0.6f + ((m_randomNumber / 100) % 100) / 300.0f;

    m_renderer->BeginFrame();
    m_renderer->Clear(r, g, b);

    // Draw engine name (top left)
    std::wstring rendererName(GetRendererName(), GetRendererName() + strlen(GetRendererName()));
    m_renderer->DrawText(rendererName.c_str(), 40.0f, 30.0f, 24.0f, 1.0f, 1.0f, 1.0f);

    // Draw title (centered at top)
    float titleWidth, titleHeight;
    m_renderer->MeasureText(L"Random Number Generator", 24.0f, titleWidth, titleHeight);
    float titleX = (m_width - titleWidth) / 2.0f;
    m_renderer->DrawText(L"Random Number Generator", titleX, 80.0f, 24.0f, 1.0f, 1.0f, 1.0f);

    // Draw large number (centered)
    std::wstring numberText = std::to_wstring(m_randomNumber);
    float numberWidth, numberHeight;
    m_renderer->MeasureText(numberText.c_str(), 120.0f, numberWidth, numberHeight);
    float numberX = (m_width - numberWidth) / 2.0f;
    float numberY = (m_height - numberHeight) / 2.0f;
    m_renderer->DrawText(numberText.c_str(), numberX, numberY, 120.0f, 1.0f, 1.0f, 0.39f, true); // Yellow, bold

    // Draw update message (bottom center)
    float messageWidth, messageHeight;
    m_renderer->MeasureText(L"Updates every 5 seconds", 20.0f, messageWidth, messageHeight);
    float messageX = (m_width - messageWidth) / 2.0f;
    m_renderer->DrawText(L"Updates every 5 seconds", messageX, m_height - 100.0f, 20.0f, 0.78f, 0.78f, 0.78f);

    m_renderer->EndFrame();
}
