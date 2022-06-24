#ifndef __HEADERS_H__
#define __HEADERS_H__

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <comdef.h>
#include <wrl/client.h> // comptr
#include <shellapi.h> //cmdline

#include <dwmapi.h> // dark title bar
#pragma comment(lib, "Dwmapi.lib") // dark title bar

#include <dxgi1_6.h> // swapchain
#pragma comment(lib, "dxgi.lib") // dxgi

#include <directx/d3d12.h> // directX
#pragma comment(lib, "D3D12.lib") // directX

// Directx
#include <directx/d3dx12.h> // helper
#include <DirectXMath.h>
#include <d3dcompiler.h> // hlsl
#pragma comment(lib, "d3dcompiler.lib") // hlsl

// stl
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>
#include <queue>

// loader
#include <tiny_obj_loader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#endif