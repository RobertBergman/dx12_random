#include "DX12Renderer.h"
#include "Logger.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Helper macro for checking HRESULT
#define CHECK_HR(hr, msg) if (FAILED(hr)) { Logger::LogError(msg, hr); throw std::runtime_error(msg); }

DX12Renderer::DX12Renderer()
    : m_hwnd(nullptr)
    , m_frameIndex(0)
    , m_viewport()
    , m_scissorRect()
    , m_rtvDescriptorSize(0)
    , m_width(0)
    , m_height(0)
    , m_fenceEvent(nullptr)
    , m_fenceValue(0)
{
}

DX12Renderer::~DX12Renderer()
{
    OnDestroy();
}

void DX12Renderer::Initialize(HWND hwnd, UINT width, UINT height)
{
    Logger::Log("DX12Renderer::Initialize - Starting");
    try
    {
        m_hwnd = hwnd;
        m_width = width;
        m_height = height;

        m_viewport.Width = static_cast<float>(width);
        m_viewport.Height = static_cast<float>(height);
        m_viewport.MaxDepth = 1.0f;

        m_scissorRect.right = static_cast<LONG>(width);
        m_scissorRect.bottom = static_cast<LONG>(height);

        Logger::Log("Loading pipeline...");
        LoadPipeline(hwnd);
        Logger::Log("Loading assets...");
        LoadAssets();
        Logger::Log("Initializing SpriteBatch...");
        InitializeSpriteBatch();
        Logger::Log("DX12Renderer::Initialize - Complete");
    }
    catch (const std::exception& e)
    {
        Logger::LogError(std::string("DX12Renderer::Initialize failed: ") + e.what());
        throw;
    }
}
void DX12Renderer::LoadPipeline(HWND hwnd)
{
    HRESULT hr;
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            Logger::Log("Debug layer enabled");
        }
    }
#endif

    Logger::Log("Creating DXGI factory...");
    ComPtr<IDXGIFactory4> factory;
    hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    CHECK_HR(hr, "Failed to create DXGI factory");

    Logger::Log("Enumerating adapters...");
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    hr = factory->EnumAdapters1(0, &hardwareAdapter);
    CHECK_HR(hr, "Failed to enumerate adapters");

    Logger::Log("Creating D3D12 device...");
    hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
    CHECK_HR(hr, "Failed to create D3D12 device");

    Logger::Log("Creating command queue...");
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    CHECK_HR(hr, "Failed to create command queue");

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    swapChain.As(&m_swapChain);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT n = 0; n < FRAME_COUNT; n++)
    {
        m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }

    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
}


void DX12Renderer::LoadAssets()
{
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
    m_commandList->Close();

    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    m_fenceValue = 1;

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}


void DX12Renderer::InitializeSpriteBatch()
{
    using namespace DirectX;

    Logger::Log("Initializing DirectXTK12 SpriteBatch and SpriteFont...");

    // Create descriptor heap for sprite fonts
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 2; // One for each font
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_fontHeap));
    CHECK_HR(hr, "Failed to create font descriptor heap");

    // Initialize Graphics Memory
    m_graphicsMemory = std::make_unique<GraphicsMemory>(m_device.Get());

    // Create Resource Upload Batch for loading fonts
    ResourceUploadBatch resourceUpload(m_device.Get());
    resourceUpload.Begin();

    // Get descriptor handles
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_fontHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_fontHeap->GetGPUDescriptorHandleForHeapStart());

    // Load sprite fonts
    try
    {
        // Small font (24pt)
        m_font = std::make_unique<SpriteFont>(
            m_device.Get(),
            resourceUpload,
            L"arial24.spritefont",
            cpuHandle,
            gpuHandle
        );

        // Move to next descriptor
        UINT descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        cpuHandle.Offset(1, descriptorSize);
        gpuHandle.Offset(1, descriptorSize);

        // Large font (120pt bold)
        m_largeFont = std::make_unique<SpriteFont>(
            m_device.Get(),
            resourceUpload,
            L"arial120.spritefont",
            cpuHandle,
            gpuHandle
        );

        Logger::Log("Sprite fonts loaded successfully");
    }
    catch (const std::exception& e)
    {
        Logger::LogError(std::string("Failed to load sprite fonts: ") + e.what());
        throw;
    }

    // Create SpriteBatch (BEFORE ending resource upload)
    DirectX::RenderTargetState rtState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    DirectX::SpriteBatchPipelineStateDescription pd(rtState);
    m_spriteBatch = std::make_unique<SpriteBatch>(m_device.Get(), resourceUpload, pd, &m_viewport);

    // Upload font resources
    auto uploadResourcesFinished = resourceUpload.End(m_commandQueue.Get());
    uploadResourcesFinished.wait();

    Logger::Log("SpriteBatch initialized successfully");
}


void DX12Renderer::BeginFrame()
{
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), nullptr);

    // Transition to render target
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void DX12Renderer::Clear(float r, float g, float b)
{
    const float clearColor[] = { r, g, b, 1.0f };
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Set viewport and scissor rect for sprite rendering
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Set descriptor heap for sprite rendering
    ID3D12DescriptorHeap* heaps[] = { m_fontHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

    // Begin sprite batch
    m_spriteBatch->Begin(m_commandList.Get());
}

void DX12Renderer::DrawText(const wchar_t* text, float x, float y, float fontSize,
                            float r, float g, float b, bool bold)
{
    using namespace DirectX;

    XMVECTOR color = XMVectorSet(r, g, b, 1.0f);

    // Choose font based on size (threshold at 60pt)
    DirectX::SpriteFont* font = (fontSize > 60.0f) ? m_largeFont.get() : m_font.get();

    font->DrawString(m_spriteBatch.get(), text, XMFLOAT2(x, y), color);
}

void DX12Renderer::MeasureText(const wchar_t* text, float fontSize,
                               float& outWidth, float& outHeight)
{
    using namespace DirectX;

    // Choose font based on size
    DirectX::SpriteFont* font = (fontSize > 60.0f) ? m_largeFont.get() : m_font.get();

    XMVECTOR size = font->MeasureString(text);
    outWidth = XMVectorGetX(size);
    outHeight = XMVectorGetY(size);
}

void DX12Renderer::EndFrame()
{
    // End sprite batch
    m_spriteBatch->End();

    // Transition back to present
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_commandList->ResourceBarrier(1, &barrier);
    m_commandList->Close();

    // Execute command list
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present
    m_swapChain->Present(1, 0);

    // Wait for frame completion
    WaitForPreviousFrame();

    // Update graphics memory (DirectXTK12 requirement)
    m_graphicsMemory->Commit(m_commandQueue.Get());
}

void DX12Renderer::WaitForPreviousFrame()
{
    const UINT64 fence = m_fenceValue;
    m_commandQueue->Signal(m_fence.Get(), fence);
    m_fenceValue++;

    if (m_fence->GetCompletedValue() < fence)
    {
        m_fence->SetEventOnCompletion(fence, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DX12Renderer::OnDestroy()
{
    if (m_device)
    {
        WaitForPreviousFrame();
    }

    if (m_swapChain)
    {
        m_swapChain->SetFullscreenState(FALSE, nullptr);
    }

    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    m_spriteBatch.reset();
    m_font.reset();
    m_largeFont.reset();
    m_graphicsMemory.reset();
}
