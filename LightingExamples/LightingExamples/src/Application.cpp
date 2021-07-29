#include "DrawingApp.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int iCmdshow)
{
    DrawingApp theApp(hInstance);

    if (!theApp.Initialize())
        return 0;

    return theApp.Run();
}