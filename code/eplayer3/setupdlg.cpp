/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../eshared/eshared.hpp"
#include "setupdlg.hpp"

static const eChar WINDOW_TITLE[]  = "Enigma 3";
static const eInt BTN_WIDTH = 135;
static const eInt BTN_HEIGHT = 25;

// Struct for passing more than just one
// parameter to window callback.
struct WndProcParams
{
    const eGraphicsApiDx9 * gfx;
    eSetup *              setup;
};

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static eArray<HWND> resBtns;
    static HWND windowedBtn;
    static WndProcParams *pp = eNULL;

    switch (msg)
    {
        case WM_CREATE:
        {
            pp = (WndProcParams *)((CREATESTRUCT *)lparam)->lpCreateParams;
            eASSERT(pp != eNULL);

            for (eU32 i=0; i<pp->gfx->getResolutionCount(); i++)
            {
                const eSize &res = pp->gfx->getResolution(i);
                eChar buffer[32];

                eStrCopy(buffer, eIntToStr(res.width));
                eStrAppend(buffer, "  x  ");
                eStrAppend(buffer, eIntToStr(res.height));
                
                HWND btn = CreateWindow("button", buffer, WS_CHILD | WS_VISIBLE, 0, i*BTN_HEIGHT,
                                        BTN_WIDTH, BTN_HEIGHT, hwnd, NULL, eNULL, eNULL);
                eASSERT(btn != eNULL);
                resBtns.append(btn);
            }

            windowedBtn = CreateWindow("button", "Windowed", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 22,
                                       pp->gfx->getResolutionCount()*BTN_HEIGHT,
                                       BTN_WIDTH, BTN_HEIGHT, hwnd, eNULL, eNULL, eNULL);

            return 0;
        }

        case WM_COMMAND:
        {
            if (HIWORD(wparam) == BN_CLICKED)
            {
                for (eU32 i=0; i<resBtns.size(); i++)
                {
                    if (lparam == (LPARAM)resBtns[i])
                    {
                        pp->setup->fullScreen = (SendMessage(windowedBtn, BM_GETCHECK, 0, 0) != BST_CHECKED);
                        pp->setup->res = pp->gfx->getResolution(i);
                        DestroyWindow(hwnd);
                        PostQuitMessage(IDOK);
                    }
                }
            }

            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(IDCANCEL);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

eBool eShowSetupDialog(eSetup &setup, const eEngine &engine)
{
    const eGraphicsApiDx9 *gfx = engine.getGraphicsApi();
    eASSERT(gfx != eNULL);

    // Prepare parameters for callback.
    WndProcParams pp;

    pp.gfx = gfx;
    pp.setup = &setup;

    // Create setup window.
    WNDCLASS wc;

    eMemSet(&wc, 0, sizeof(wc));
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpfnWndProc   = wndProc;
    wc.lpszClassName = WINDOW_TITLE;
    wc.style         = CS_HREDRAW | CS_VREDRAW;

    const ATOM res = RegisterClass(&wc);
    eASSERT(res != eNULL);
    const HWND hwnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_SYSMENU | WS_VISIBLE, CW_USEDEFAULT,
                                   CW_USEDEFAULT, BTN_WIDTH+5, (gfx->getResolutionCount()+1)*BTN_HEIGHT+2+25,
                                   NULL, NULL, NULL, (ePtr)&pp);
    eASSERT(hwnd != eNULL);

    // Window's main loop.
    MSG msg;
    eMemSet(&msg, 0, sizeof(msg));

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, eNULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Has user clicked a resolution button or not?
    return (msg.wParam == IDOK ? eTRUE : eFALSE);
}