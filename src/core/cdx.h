#pragma once

#pragma warning(disable:4201) // Nonstandard extension used : nameless struct/union.
#pragma warning(disable:4238) // Nonstandard extension used : class rvalue used as lvalue.
#pragma warning(disable:4239) // A non-const reference may only be bound to an lvalue; assignment operator takes a reference to non-const.
#pragma warning(disable:4324) // Structure was padded due to __declspec(align()).

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wrl/client.h>
#include <wrl/event.h>

// DirectX 12 includes.
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)
#define MY_IID_PPV_ARGS                     IID_PPV_ARGS

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <memory>
#include <string>
#include <cwctype>
#include <exception>
#include <ppltasks.h>
#include <functional>