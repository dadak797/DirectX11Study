#include "BoxApp.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int iCmdshow)
{
    BoxApp theApp(hInstance);

    if (!theApp.Initialize())
        return 0;

    return theApp.Run();
}