#include "InitDirect3DApp.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int iCmdshow)
{
    InitDirect3DApp theApp(hInstance);

    if (!theApp.Initialize())
        return 0;

    return theApp.Run();
}