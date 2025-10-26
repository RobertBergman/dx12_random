#include "GDIRenderer.h"
#include <wingdi.h>
#include <algorithm>

GDIRenderer::GDIRenderer()
    : m_hwnd(nullptr)
    , m_width(0)
    , m_height(0)
    , m_windowDC(nullptr)
    , m_memoryDC(nullptr)
    , m_memoryBitmap(nullptr)
    , m_oldBitmap(nullptr)
{
}

GDIRenderer::~GDIRenderer()
{
    OnDestroy();
}

void GDIRenderer::Initialize(HWND hwnd, UINT width, UINT height)
{
    m_hwnd = hwnd;
    m_width = width;
    m_height = height;

    // Create memory DC for double buffering
    m_windowDC = GetDC(hwnd);
    m_memoryDC = CreateCompatibleDC(m_windowDC);
    m_memoryBitmap = CreateCompatibleBitmap(m_windowDC, width, height);
    m_oldBitmap = (HBITMAP)SelectObject(m_memoryDC, m_memoryBitmap);
}

void GDIRenderer::BeginFrame()
{
    // Nothing needed - we draw directly to memory DC
}

void GDIRenderer::Clear(float r, float g, float b)
{
    // Create gradient background from color to lighter version
    TRIVERTEX vertices[2];
    vertices[0].x = 0;
    vertices[0].y = 0;
    vertices[0].Red = (COLOR16)(r * 65535.0f);
    vertices[0].Green = (COLOR16)(g * 65535.0f);
    vertices[0].Blue = (COLOR16)(b * 65535.0f);
    vertices[0].Alpha = 0xffff;

    vertices[1].x = m_width;
    vertices[1].y = m_height;
    vertices[1].Red = (COLOR16)(std::min(1.0f, r * 1.3f) * 65535.0f);
    vertices[1].Green = (COLOR16)(std::min(1.0f, g * 1.3f) * 65535.0f);
    vertices[1].Blue = (COLOR16)(std::min(1.0f, b * 1.3f) * 65535.0f);
    vertices[1].Alpha = 0xffff;

    GRADIENT_RECT gRect;
    gRect.UpperLeft = 0;
    gRect.LowerRight = 1;

    GradientFill(m_memoryDC, vertices, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void GDIRenderer::DrawText(const wchar_t* text, float x, float y, float fontSize,
                           float r, float g, float b, bool bold)
{
    SetBkMode(m_memoryDC, TRANSPARENT);
    SetTextColor(m_memoryDC, RGB((BYTE)(r * 255), (BYTE)(g * 255), (BYTE)(b * 255)));

    HFONT hFont = CreateFontW(
        (int)fontSize, 0, 0, 0,
        bold ? FW_BOLD : FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

    HFONT hOldFont = (HFONT)SelectObject(m_memoryDC, hFont);

    RECT textRect = { (LONG)x, (LONG)y, (LONG)m_width, (LONG)m_height };
    ::DrawTextW(m_memoryDC, text, -1, &textRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP);

    SelectObject(m_memoryDC, hOldFont);
    DeleteObject(hFont);
}

void GDIRenderer::MeasureText(const wchar_t* text, float fontSize,
                              float& outWidth, float& outHeight)
{
    HFONT hFont = CreateFontW(
        (int)fontSize, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

    HFONT hOldFont = (HFONT)SelectObject(m_memoryDC, hFont);

    SIZE size;
    GetTextExtentPoint32W(m_memoryDC, text, (int)wcslen(text), &size);

    outWidth = (float)size.cx;
    outHeight = (float)size.cy;

    SelectObject(m_memoryDC, hOldFont);
    DeleteObject(hFont);
}

void GDIRenderer::EndFrame()
{
    // Copy from memory DC to window DC
    BitBlt(m_windowDC, 0, 0, m_width, m_height, m_memoryDC, 0, 0, SRCCOPY);
}

void GDIRenderer::OnDestroy()
{
    if (m_memoryDC)
    {
        SelectObject(m_memoryDC, m_oldBitmap);
        DeleteObject(m_memoryBitmap);
        DeleteDC(m_memoryDC);
        m_memoryDC = nullptr;
    }

    if (m_windowDC)
    {
        ReleaseDC(m_hwnd, m_windowDC);
        m_windowDC = nullptr;
    }
}
