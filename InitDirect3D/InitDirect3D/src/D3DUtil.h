#pragma once

#pragma comment(lib, "d3d11.lib")

#include <windows.h>
#include <d3d11.h>
#include <sstream>
#include <cassert>
#include <DirectXMath.h>

using namespace DirectX;


#define ReleaseCOM(x) { if(x){ x->Release(); x = nullptr; } }

#if defined(DEBUG) || defined(_DEBUG)
#ifndef HR
#define HR(x)                                                 \
{                                                             \
    HRESULT hr = (x);                                         \
    if(FAILED(hr))                                            \
    {                                                         \
        std::wostringstream ss;                               \
        ss << L"Function: " << L#x;                           \
        ss << L"\nFile: " << __FILEW__;                       \
        ss << L"\nLine: " << __LINE__;                        \
        MessageBox(NULL, ss.str().c_str(), L"Error", MB_OK);  \
    }                                                         \
}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif

namespace Colors
{
    const XMFLOAT4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
    const XMFLOAT4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    const XMFLOAT4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
    const XMFLOAT4 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
    const XMFLOAT4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    const XMFLOAT4 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
    const XMFLOAT4 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
    const XMFLOAT4 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
    const XMFLOAT4 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
    const XMFLOAT4 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
}