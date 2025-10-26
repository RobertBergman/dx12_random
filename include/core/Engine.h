#pragma once
#include "IRenderer.h"
#include <memory>
#include <random>
#include <chrono>

// Application engine - handles logic only, delegates rendering to IRenderer
class Engine
{
public:
    Engine(UINT width, UINT height);
    ~Engine();

    // Initialize engine with a renderer
    void Initialize(HWND hwnd, std::unique_ptr<IRenderer> renderer);

    // Update application state
    void Update();

    // Render the scene
    void Render();

    // Cleanup
    void OnDestroy();

    // Switch to a different renderer at runtime
    void SwitchRenderer(std::unique_ptr<IRenderer> newRenderer);

    // Get current random number
    int GetRandomNumber() const { return m_randomNumber; }

    // Get current renderer name
    const char* GetRendererName() const;

private:
    void UpdateRandomNumber();
    void RenderScene();

    HWND m_hwnd;
    UINT m_width;
    UINT m_height;

    std::unique_ptr<IRenderer> m_renderer;

    // Application state
    int m_randomNumber;
    std::mt19937 m_rng;
    std::chrono::steady_clock::time_point m_lastUpdateTime;
};
