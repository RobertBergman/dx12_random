#pragma once
#include "IRenderer.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>

// DirectXTK12 headers
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "ResourceUploadBatch.h"
#include "GraphicsMemory.h"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

// DirectX 12 renderer implementation
class DX12Renderer : public IRenderer
{
public:
    DX12Renderer();
    ~DX12Renderer() override;

    void Initialize(HWND hwnd, UINT width, UINT height) override;
    void BeginFrame() override;
    void Clear(float r, float g, float b) override;
    void DrawText(const wchar_t* text, float x, float y, float fontSize,
                 float r, float g, float b, bool bold = false) override;
    void MeasureText(const wchar_t* text, float fontSize,
                    float& outWidth, float& outHeight) override;
    void EndFrame() override;
    void OnDestroy() override;
    const char* GetName() const override { return "DirectX 12 Renderer"; }

private:
    void LoadPipeline(HWND hwnd);
    void LoadAssets();
    void InitializeSpriteBatch();
    void WaitForPreviousFrame();

    static const UINT FRAME_COUNT = 2;

    // D3D12 Pipeline objects
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // Synchronization
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    // Viewport
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
    UINT m_rtvDescriptorSize;

    // DirectXTK12 for text rendering
    std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont> m_font;          // 24pt
    std::unique_ptr<DirectX::SpriteFont> m_largeFont;     // 120pt
    ComPtr<ID3D12DescriptorHeap> m_fontHeap;

    // State
    HWND m_hwnd;
    UINT m_width;
    UINT m_height;
};
