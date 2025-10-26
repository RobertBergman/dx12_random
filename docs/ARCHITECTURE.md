# Graphics Engine Architecture

## Current Structure

The current architecture already has some separation:

### Interfaces
- **IGraphicsEngine**: Abstract interface for graphics engines
  - Handles both rendering AND application logic
  - Methods: Initialize, Update, Render, OnDestroy, GetRandomNumber, GetEngineName

### Implementations
- **GDIEngine**: GDI-based implementation
  - Rendering: GDI double-buffered rendering with gradient fills
  - Logic: Random number generation, timing

- **DX12Engine**: DirectX 12 implementation
  - Rendering: DirectX 12 with DirectXTK12 sprite fonts
  - Logic: Random number generation, timing

## Proposed Improved Architecture

To separate engine logic from graphics rendering:

### Layer 1: Application Logic (Engine)
```
Engine
├── State management (random number, timing)
├── Update logic (every 5 seconds)
└── Uses IRenderer for all drawing
```

### Layer 2: Graphics Rendering (IRenderer)
```
IRenderer (interface)
├── Initialize(hwnd, width, height)
├── BeginFrame()
├── Clear(r, g, b)
├── DrawText(text, x, y, size, color, bold)
├── MeasureText(text, size) -> width, height
├── EndFrame()
└── OnDestroy()

Implementations:
- GDIRenderer (uses GDI)
- DX12Renderer (uses DirectX 12 + DirectXTK12)
```

## Benefits of Separation

1. **Single Responsibility**: Engine handles logic, Renderer handles drawing
2. **Reusability**: Same engine can work with different renderers
3. **Testability**: Can test logic without graphics, test rendering separately
4. **Flexibility**: Easy to add new renderers (Vulkan, Metal, etc.)

## Current vs Proposed

### Current:
```cpp
GDIEngine engine(1280, 720);
engine.Initialize(hwnd);
engine.Update();  // Updates random number
engine.Render();  // Draws everything
```

### Proposed:
```cpp
Engine engine(1280, 720);
auto renderer = std::make_unique<GDIRenderer>();
engine.Initialize(hwnd, std::move(renderer));

engine.Update();  // Updates random number
engine.Render();  // Calls renderer->BeginFrame(), DrawText(), EndFrame()

// Switch renderer at runtime
engine.SwitchRenderer(std::make_unique<DX12Renderer>());
```

## Implementation Notes

The current system works well! The refactoring would require:
1. Creating IRenderer interface with rendering-only methods
2. Extracting rendering code from GDIEngine/DX12Engine into GDIRenderer/DX12Renderer
3. Creating new Engine class that uses IRenderer
4. Updating main.cpp to use new architecture

This is a significant refactoring that would take time but improve code quality.
