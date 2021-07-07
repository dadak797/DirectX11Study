#pragma once

#include "D3DApp.h"


class InitDirect3DApp : public D3DApp
{
public:
    InitDirect3DApp(HINSTANCE hInstance);
    ~InitDirect3DApp();

    bool Initialize() override;
    void OnResize() override;
    void UpdateScene(float dt) override;
    void DrawScene() override;
};

