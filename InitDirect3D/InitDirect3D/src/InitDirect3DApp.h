#pragma once

#include "D3DApp.h"


class InitDirect3DApp : public D3DApp
{
public:
    InitDirect3DApp(HINSTANCE hInstance);
    ~InitDirect3DApp();

    bool Initialize();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();
};

