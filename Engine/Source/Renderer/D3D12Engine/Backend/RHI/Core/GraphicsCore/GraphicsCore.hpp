#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

namespace D3D12Engine {
  namespace GraphicsCore {
    static const UINT FrameCount = 3;
    const DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    const DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D32_FLOAT;
  }
}