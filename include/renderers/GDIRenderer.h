#pragma once
#include "IRenderer.h"

// GDI-based renderer implementation
class GDIRenderer : public IRenderer
{
public:
    GDIRenderer();
    ~GDIRenderer() override;

    void Initialize(HWND hwnd, UINT width, UINT height) override;
    void BeginFrame() override;
    void Clear(float r, float g, float b) override;
    void DrawText(const wchar_t* text, float x, float y, float fontSize,
                 float r, float g, float b, bool bold = false) override;
    void MeasureText(const wchar_t* text, float fontSize,
                    float& outWidth, float& outHeight) override;
    void EndFrame() override;
    void OnDestroy() override;
    const char* GetName() const override { return "GDI Renderer"; }

private:
    HWND m_hwnd;
    UINT m_width;
    UINT m_height;

    // Double buffering
    HDC m_windowDC;
    HDC m_memoryDC;
    HBITMAP m_memoryBitmap;
    HBITMAP m_oldBitmap;
};
