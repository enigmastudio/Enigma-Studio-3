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
#include "../configinfo.hpp"

#include "setupdlg.hpp"
#include "production.hpp"

//#define SHOW_STATS
//#define BENCHMARK

static eBool quit = eFALSE;

static eBool loadingCallback(eIRenderer *renderer, eU32 processed, eU32 total, ePtr param)
{
    eASSERT(renderer != eNULL);
    eASSERT(processed <= total);

    eGraphicsApiDx9 *gfx = renderer->getGraphicsApi();
    eASSERT(gfx != eNULL);

    eMessage msg;
    renderer->getGraphicsApi()->handleMessages(msg);

    if (msg == eMSG_IDLE && processed%5 == 0)
    {
        if (gfx->renderStart())
        {
            eIDemoOp *loadingOp = (eIDemoOp *)param;

            if (loadingOp)
            {
//                eASSERT(loadingOp->getType() == "Misc : Demo");

                const eF32 time = (eF32)processed/(eF32)total;

                loadingOp->process(renderer, time);
                loadingOp->getResult().demo.render(time, renderer);
            }
            else
            {
                gfx->clear(eCLEAR_ALLBUFFERS, eColor::BLACK);
            }

            gfx->renderEnd();
        }
    }

	quit = (msg == eMSG_QUIT);
    return !quit;
}

#ifdef eDEBUG
eInt WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, eChar *cmdLine, eInt showCmd)
#else
void WinMainCRTStartup()
#endif
{
    eMemTrackerStart();
    eInitGlobalsStatics();

    if (eVerifyInstructionSets())
    {
        eSetup setup(800, 600, eFALSE);
        eEngine engine;

#ifdef eRELEASE
		if (eShowSetupDialog(setup, engine))
#endif
        {
            engine.openWindow(setup.fullScreen, setup.res, eNULL);

            eGraphicsApiDx9 *gfx = engine.getGraphicsApi();
            eASSERT(gfx != eNULL);
            eIRenderer *renderer = engine.getRenderer();
            eASSERT(renderer != eNULL);

            // Load and generate demo.
            eOpStacking::initialize();

            eDemoScript script(data, sizeof(data));
            eDemoData::load(script);

            eIDemoOp *demoOp = eDemoData::getMainDemoOperator();
            eASSERT(demoOp != eNULL);
            demoOp->setProcessAll(eTRUE);
            gfx->setWindowTitle(demoOp->getParameter(1).getValue().string);
            eIDemoOp *loadingOp = (eIDemoOp *)eDemoData::findOperator(demoOp->getParameter(2).getValue().linkedOpId);

            if (loadingOp)
            {
                loadingOp->process(renderer, 0.0f);
            }

            // Process returns whether or not the generation
            // of the demo content was interrupted by user.
            if (demoOp->process(renderer, 0.0f, loadingCallback, loadingOp) && !quit)
            {
                demoOp->setProcessAll(eFALSE);

                eDemo &demo = demoOp->getResult().demo;
                demo.getSynth().play(0.0f);

                // Start the demo.
                eTimer timer;
                eMessage msg;
                eBool ended = eFALSE;

#ifdef BENCHMARK
                eU32 totalFrames = 0;
#endif

                eF32 oldTime = 0.0f;

                do
                {
                    gfx->handleMessages(msg);

                    if (msg == eMSG_IDLE)
                    {
                        const eF32 time = (eF32)timer.getElapsedMs()/1000.0f;

                        if (gfx->renderStart())
                        {
                            demoOp->process(renderer, time);
                            ended = demo.render(time, renderer);
                            gfx->renderEnd();
#ifdef BENCHMARK
                            totalFrames++;
#endif
                        }
#ifdef SHOW_STATS
                        const eRenderStats &stats = gfx->getRenderStats();

                        eString buffer = "Vertices: ";
                        buffer += eIntToStr(stats.vertices);
                        buffer += ", Triangles: ";
                        buffer += eIntToStr(stats.triangles);
                        buffer += ", FPS: ";
                        buffer += eIntToStr(eFtoL(stats.fps));
                        buffer += ", Time: ";
                        buffer += eIntToStr(eFtoL(time));
                        buffer += ".";
                        buffer += eIntToStr(eFtoL(time*100)%100);

                        gfx->setWindowTitle(buffer);
#endif
                    }
                }
                while (!ended && msg != eMSG_QUIT);

#ifdef BENCHMARK
                const eU32 averageFps = eFtoL((eF32)totalFrames/timer.getElapsedMs()*1000.0f);
                eShowError(eIntToStr(averageFps));
#endif
                demo.getSynth().stop();
            }
#ifdef eEDITOR         
            eOpStacking::shutdown();
#endif
        }
    }

    eFreeGlobalsStatics();
    ExitProcess(0);

#ifdef eDEBUG
    return 0;
#endif
}