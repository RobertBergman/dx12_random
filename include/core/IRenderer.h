#pragma once
#include <Windows.h>

// Pure rendering interface - no application logic
class IRenderer
{
public:
    virtual ~IRenderer() = default;

    // Initialize renderer with window and dimensions
    virtual void Initialize(HWND hwnd, UINT width, UINT height) = 0;

    // Begin a new frame
    virtual void BeginFrame() = 0;

    // Clear the screen with a color (RGB values 0.0-1.0)
    virtual void Clear(float r, float g, float b) = 0;

    // Draw text at position (x, y) with given font size and color
    // fontSize: point size (e.g., 24, 120)
    // r, g, b: color components 0.0-1.0
    // bold: whether to use bold font
    virtual void DrawText(const wchar_t* text, float x, float y, float fontSize,
                         float r, float g, float b, bool bold = false) = 0;

    // Measure text dimensions for layout calculations
    virtual void MeasureText(const wchar_t* text, float fontSize,
                            float& outWidth, float& outHeight) = 0;

    // End frame and present to screen
    virtual void EndFrame() = 0;

    // Cleanup resources
    virtual void OnDestroy() = 0;

    // Get renderer name for display
    virtual const char* GetName() const = 0;
};
