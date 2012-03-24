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

#define D3D_DEBUG_INFO
#include <d3d9.h>
#include <d3dx9.h>

#include "../../eshared.hpp"

#include "texturesdx9.hpp"
#include "buffersdx9.hpp"
#include "shadersdx9.hpp"

#define ACTIVATE_PERFHUD

static const D3DVERTEXELEMENT9 VERTEX_DECLS[eVTXTYPE_COUNT-1][13] =
{
    // Default vertex declaration.
    {
        { 0,   0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0,  12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0,  24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0,  32, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        { 1,   0, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
        { 1,  16, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
        { 1,  32, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
        { 1,  48, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
        { 1,  64, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5 },
        { 1,  80, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6 },
        { 1,  96, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 7 },
        { 1, 112, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 8 },
        D3DDECL_END()
    },

    // Vertex declaration for particles.
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        D3DDECL_END()
    }
};

// Additional formats (aka driver hacks).
const D3DFORMAT FOURCC_NULL = ((D3DFORMAT)(MAKEFOURCC('N', 'U', 'L', 'L')));

// Initialize static members.
const eChar eGraphicsApiDx9::WINDOW_TITLE[] = "Enigma Player 3";

eGraphicsApiDx9::eGraphicsApiDx9() :
    m_d3dMain(eNULL),
    m_d3dDevice(eNULL),
    m_d3dPp(new D3DPRESENT_PARAMETERS),
    m_d3dCaps(new D3DCAPS9),
    m_screenRt(eNULL),
    m_screenDt(eNULL),
    m_deviceLost(eFALSE),
    m_hwnd(eNULL),
    m_ownWindow(eFALSE),
    m_fullScreen(eFALSE),
    m_vsync(eFALSE),
    m_wndWidth(800),
    m_wndHeight(600),
    m_frameQuery(0),
    m_frameCollect(-1)
{
    eResourceManager::setGraphics(this);
    eStateManager::setGraphics(this);
    eStateManager::reset();


    eASSERT(m_d3dPp != eNULL);
    eASSERT(m_d3dCaps != eNULL);

    eMemSet(m_d3dVDecls, 0, sizeof(m_d3dVDecls));
    eMemSet(m_d3dQueriesTsBegin, 0, sizeof(m_d3dQueriesTsBegin));
    eMemSet(m_d3dQueriesTsEnd, 0, sizeof(m_d3dQueriesTsEnd));
    eMemSet(m_d3dQueriesTsFreq, 0, sizeof(m_d3dQueriesTsFreq));
    eMemSet(m_d3dQueriesTsDisjoint, 0, sizeof(m_d3dQueriesTsDisjoint));

    eMemSet(m_occlQuery, 0, sizeof(m_occlQuery));
    m_startPull = 0;
    m_frame = 0;
}

#ifdef eEDITOR

eGraphicsApiDx9::~eGraphicsApiDx9()
{
    shutdown();

    eSAFE_DELETE(m_d3dCaps);
    eSAFE_DELETE(m_d3dPp);
}

#endif

eBool eGraphicsApiDx9::initialize()
{
    m_d3dMain = Direct3DCreate9(D3D_SDK_VERSION);
    eASSERT(m_d3dMain != eNULL);

    if (m_d3dMain)
    {
        HRESULT res = m_d3dMain->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_d3dCaps);
        eASSERT(!FAILED(res));

        // Check for depth-stencil texture support.
        const HRESULT resDst = m_d3dMain->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
                                                            D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, D3DFMT_D16);

        // Check for null render-target support.
        const HRESULT resNullRt = m_d3dMain->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
                                                               D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, FOURCC_NULL);

        // Check for floating-point render-targets (HDR).
        const HRESULT resFpRt = m_d3dMain->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
                                                             D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING | D3DUSAGE_RENDERTARGET,
                                                             D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F);

        if (FAILED(resDst))
        {
            eShowError("No depth-stencil textures support!");
        }
        else if (FAILED(resNullRt))
        {
            eShowError("No null render-targets supported!");
        }
        else if (FAILED(resFpRt))
        {
            eShowError("No floating-point render-targets supported!");
        }
        else if (m_d3dCaps->PixelShaderVersion < D3DPS_VERSION(3, 0))
        {
            eShowError("Shader model 3.0 compatible graphics card needed!");
        }
        else if (m_d3dCaps->TextureCaps & D3DPTEXTURECAPS_POW2 || m_d3dCaps->TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
        {
            eShowError("Graphics card does not support non-power-of-2 textures!");
        }
        else if (m_d3dCaps->MaxVertexIndex <= 65535)
        {
            eShowError("Graphics card does not support 32-bit index buffers!");
        }
        else if (m_d3dCaps->NumSimultaneousRTs < MAX_TARGETS)
        {
            eShowError("Not enough simultaneous render-targets supported!");
        }
        else
        {
            _enumerateResolutions();
            return eTRUE;
        }
    }
    else
    {
        eShowError("Couldn't initialize Direct3D9!");
    }

#ifdef eEDITOR
    shutdown();
#endif
    return eFALSE;
}

#ifdef eEDITOR
void eGraphicsApiDx9::shutdown()
{
    _releaseVertexDecls();
    _releaseNullTarget();
    _releaseScreenTargets();
    _releaseQueries();

    eSAFE_RELEASE_COM(m_d3dMain);
    eSAFE_RELEASE_COM(m_d3dDevice);

    if (m_ownWindow)
    {
        DestroyWindow((HWND)m_hwnd);
        m_hwnd = eNULL;
    }
}
#endif

eBool eGraphicsApiDx9::getInitialized() const
{
    return (m_d3dMain ? eTRUE : eFALSE);
}

eU32 eGraphicsApiDx9::getResolutionCount() const
{
    return m_resolutions.size();
}

const eSize & eGraphicsApiDx9::getResolution(eU32 index) const
{
    return m_resolutions[index];
}

eBool eGraphicsApiDx9::openWindow(eU32 width, eU32 height, eBool fullScreen, eBool vsync, ePtr hwnd)
{
    m_wndWidth   = width;
    m_wndHeight  = height;
    m_fullScreen = fullScreen;
    m_vsync      = vsync;

    if (hwnd)
    {
        m_hwnd = (HWND)hwnd;
        m_ownWindow = eFALSE;
    }
    else
    {
        m_hwnd = _createWindow(width, height, fullScreen);
        eASSERT(m_hwnd != eNULL);
        m_ownWindow = eTRUE;
    }

    // Initialize Direct3D9 API.
    eMemSet(m_d3dPp, 0, sizeof(D3DPRESENT_PARAMETERS));

    m_d3dPp->SwapEffect             = D3DSWAPEFFECT_DISCARD;
    m_d3dPp->BackBufferFormat       = D3DFMT_A8R8G8B8;
    m_d3dPp->BackBufferCount        = 1;
    m_d3dPp->EnableAutoDepthStencil = TRUE;
    m_d3dPp->AutoDepthStencilFormat = D3DFMT_D24S8;
    m_d3dPp->hDeviceWindow          = (HWND)m_hwnd;
    m_d3dPp->PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
    m_d3dPp->MultiSampleType        = D3DMULTISAMPLE_NONE;

    if (fullScreen)
    {
        m_d3dPp->BackBufferWidth = width;
        m_d3dPp->BackBufferHeight = height;

        // V-sync can only be activated in fullscreen mode.
        if (vsync)
        {
            m_d3dPp->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        }
    }
    else
    {
        m_d3dPp->Windowed = TRUE;
    }

    // Create Direct3D device.
    eU32 adapter = D3DADAPTER_DEFAULT;
    D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

#ifdef ACTIVATE_PERFHUD
    for (eU32 i=0; i<m_d3dMain->GetAdapterCount(); i++)
    {
        D3DADAPTER_IDENTIFIER9 identifier;

        const HRESULT res = m_d3dMain->GetAdapterIdentifier(i, 0, &identifier);
        eASSERT(!FAILED(res));

        if (eStrCompare(identifier.Description, "NVIDIA PerfHUD") == 0)
        {
            adapter = i;
            deviceType = D3DDEVTYPE_REF;
            break;
        }
    }
#endif

    if (FAILED(m_d3dMain->CreateDevice(adapter,
                                       deviceType,
                                       (HWND)m_hwnd,
                                       D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                       m_d3dPp,
                                       &m_d3dDevice)))
    {
        eShowError("Couldn't create Direct3D 9 device!");
        closeWindow();
        return eFALSE;
    }

    _createQueries();
    _createNullTarget();
    _getScreenTargets();
     
    if (fullScreen)
    {
        ShowCursor(FALSE);
    }
    
    _createVertexDecls();
    eStateManager::setViewport(0, 0, width, height);
    eStateManager::forceApply();

    return eTRUE;
}

void eGraphicsApiDx9::setWindowTitle(const eString &title)
{
    SetWindowText((HWND)m_hwnd, title);
}

void eGraphicsApiDx9::closeWindow()
{
    if (m_hwnd)
    {
        eSAFE_RELEASE_COM(m_d3dDevice);
        eSAFE_RELEASE_COM(m_d3dMain);

        DestroyWindow((HWND)m_hwnd);
        m_hwnd = eNULL;
    }

    if (m_fullScreen)
    {
        ShowCursor(TRUE);
    }
}

void eGraphicsApiDx9::handleMessages(eMessage &msg)
{
    MSG winMsg;

    if (PeekMessage(&winMsg, eNULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&winMsg);
        DispatchMessage(&winMsg);

        msg = (winMsg.message == WM_QUIT ? eMSG_QUIT : eMSG_BUSY);
    }
    else
    {          
        msg = eMSG_IDLE;
    }
}

void eGraphicsApiDx9::clear(eClearMode mode, const eColor &color) const
{
    eU32 clear = 0;

    if (mode & eCLEAR_COLORBUFFER)
    {
        clear |= D3DCLEAR_TARGET;
    }

    if (mode & eCLEAR_DEPTHBUFFER)
    {
        clear |= D3DCLEAR_ZBUFFER;
    }

    if (mode & eCLEAR_STENCILBUFFER)
    {
        clear |= D3DCLEAR_STENCIL;
    }

    const HRESULT res = m_d3dDevice->Clear(0, eNULL, clear, color.toArgb(), 1.0f, 0);
    eASSERT(!FAILED(res));
}

eRenderStats eGraphicsApiDx9::getRenderStats() const
{
    return m_renderStats;
}

eBool eGraphicsApiDx9::getFullScreen() const
{
    return m_fullScreen;
}

eU32 eGraphicsApiDx9::getWindowWidth() const
{
    return m_wndWidth;
}

eU32 eGraphicsApiDx9::getWindowHeight() const
{
    return m_wndHeight;
}

eSize eGraphicsApiDx9::getWindowSize() const
{
    return eSize(m_wndWidth, m_wndHeight);
}

void eGraphicsApiDx9::setCap(eRenderCap cap, eBool enabled)
{
    static const struct CapMapping
    {
        eRenderCap          cap;
        D3DRENDERSTATETYPE  d3dState;
        eBool               values[2];
    }
    mappings[] =
    {
        {eCAP_BLENDING,     D3DRS_ALPHABLENDENABLE,  {D3DZB_FALSE, D3DZB_TRUE}},
        {eCAP_ZBUFFER,      D3DRS_ZENABLE,           {D3DZB_FALSE, D3DZB_TRUE}},
        {eCAP_ZWRITE,       D3DRS_ZWRITEENABLE,      {D3DZB_FALSE, D3DZB_TRUE}},
        {eCAP_COLORWRITE,   D3DRS_COLORWRITEENABLE,  {0,           0x0000000f}},
        {eCAP_SCISSORTEST,  D3DRS_SCISSORTESTENABLE, {D3DZB_FALSE, D3DZB_TRUE}}
    };

    const CapMapping &m = mappings[cap];
    eASSERT(m.cap == cap);
    const HRESULT res = m_d3dDevice->SetRenderState(m.d3dState, m.values[enabled]);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setBlendModes(eBlendMode srcMode, eBlendMode dstMode, eBlendOp blendOp)
{
    static const eU32 d3dBlendModeMap[] =
    {
        D3DBLEND_ZERO,
        D3DBLEND_ONE,
        D3DBLEND_SRCCOLOR,
        D3DBLEND_INVSRCCOLOR,
        D3DBLEND_SRCALPHA,
        D3DBLEND_INVSRCALPHA,
        D3DBLEND_DESTALPHA,
        D3DBLEND_INVDESTALPHA,
        D3DBLEND_DESTCOLOR,
        D3DBLEND_INVDESTCOLOR,
    };

    static const eU32 d3dBlendOpMap[] =
    {
        D3DBLENDOP_ADD,
        D3DBLENDOP_SUBTRACT,
        D3DBLENDOP_REVSUBTRACT,
        D3DBLENDOP_MIN,
        D3DBLENDOP_MAX
    };

    m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, d3dBlendModeMap[srcMode]);
    m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, d3dBlendModeMap[dstMode]);
    m_d3dDevice->SetRenderState(D3DRS_BLENDOP, d3dBlendOpMap[blendOp]);
}

void eGraphicsApiDx9::setAlphaTest(eBool enable)
{
    if (enable)
    {
        m_d3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000008);
        m_d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, eTRUE); 
        m_d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    }
    else
    {
        m_d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }
}

void eGraphicsApiDx9::setPolygonMode(ePolygonMode mode)
{
    HRESULT res;

    switch (mode)
    {
        case ePOLYMODE_FILLED:
        {
            res = m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
            break;
        }

        case ePOLYMODE_LINES:
        {
            res = m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
            break;
        }

        case ePOLYMODE_POINTS:
        {
            res = m_d3dDevice->SetRenderState(D3DRS_POINTSIZE, eFtoDW(3.0f));
            eASSERT(!FAILED(res));
            res = m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
            break;
        }
    }

    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setViewport(eU32 x, eU32 y, eU32 width, eU32 height)
{
    if (width == 0)
    {
        width = m_wndWidth;
    }

    if (height == 0)
    {
        height = m_wndHeight;
    }

    const D3DVIEWPORT9 vp =
    {
        x, y, width, height, 0, 1
    };

    const HRESULT res = m_d3dDevice->SetViewport(&vp);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setCullingMode(eCullingMode cm)
{
    static const eU32 d3dCullMap[3] =
    {
        D3DCULL_NONE,
        D3DCULL_CW,
        D3DCULL_CCW
    };

    const HRESULT res = m_d3dDevice->SetRenderState(D3DRS_CULLMODE, d3dCullMap[cm]);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setZFunction(eZFunction zFunc)
{
    // +1, because Direct3D enum starts at 1, ours at 0.
    const HRESULT res = m_d3dDevice->SetRenderState(D3DRS_ZFUNC, (D3DCMPFUNC)(zFunc+1));
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setScissorRect(const eRect &rect)
{
    eASSERT(rect.left <= rect.right);
    eASSERT(rect.top <= rect.bottom);

    RECT sc;

    sc.left   = rect.left;
    sc.top    = rect.top;
    sc.right  = rect.right;
    sc.bottom = rect.bottom;

    const HRESULT res = m_d3dDevice->SetScissorRect(&sc);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setTextureFilter(eU32 unit, eTextureFilter texFilter)
{
    static const D3DTEXTUREFILTERTYPE filterModes[3][3] =
    {
        D3DTEXF_POINT,  D3DTEXF_POINT,  D3DTEXF_POINT,
        D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_POINT,
        D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_LINEAR,
    };

    m_d3dDevice->SetSamplerState(unit, D3DSAMP_MAGFILTER, filterModes[texFilter][0]);
    m_d3dDevice->SetSamplerState(unit, D3DSAMP_MINFILTER, filterModes[texFilter][1]);
    m_d3dDevice->SetSamplerState(unit, D3DSAMP_MIPFILTER, filterModes[texFilter][2]);
}

void eGraphicsApiDx9::setTextureAddressMode(eU32 unit, eTextureAddressMode tam)
{
    static const D3DTEXTUREADDRESS addrModes[3] =
    {
        D3DTADDRESS_WRAP,
        D3DTADDRESS_CLAMP,
        D3DTADDRESS_MIRROR,
    };

    m_d3dDevice->SetSamplerState(unit, D3DSAMP_ADDRESSU, addrModes[tam]);
    m_d3dDevice->SetSamplerState(unit, D3DSAMP_ADDRESSV, addrModes[tam]);
    m_d3dDevice->SetSamplerState(unit, D3DSAMP_ADDRESSW, addrModes[tam]);
}

void eGraphicsApiDx9::setPixelShader(eIPixelShader *ps)
{
    if (ps == eNULL)
    {
        const HRESULT res = m_d3dDevice->SetPixelShader(eNULL);
        eASSERT(!FAILED(res));
    }
    else
    {
        ps->bind();   
    }
}

void eGraphicsApiDx9::setVertexShader(eIVertexShader *vs)
{
    if (vs == eNULL)
    {
        const HRESULT res = m_d3dDevice->SetVertexShader(eNULL);
        eASSERT(!FAILED(res));
    }
    else
    {
        vs->bind();   
    }
}

void eGraphicsApiDx9::setPsConst(eU32 offset, eU32 count, const eF32 *data)
{
    eASSERT(count > 0);
    eASSERT(data != eNULL);

    const HRESULT res = m_d3dDevice->SetPixelShaderConstantF(offset, data, count);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setVsConst(eU32 offset, eU32 count, const eF32 *data)
{
    eASSERT(count > 0 );
    eASSERT(data != eNULL);


    const HRESULT res = m_d3dDevice->SetVertexShaderConstantF(offset, data, count);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::setRenderTarget(eU32 index, eITexture *tex, eCubeMapFace face)
{
    eASSERT(index < eGraphicsApiDx9::MAX_TARGETS);
    eASSERT(face < eCMFACE_COUNT);

    if (index == 0)
    {
        if (tex == TARGET_SCREEN)
        {      
            const HRESULT res = m_d3dDevice->SetRenderTarget(0, m_screenRt);
            eASSERT(!FAILED(res));
        }
        else if (tex == eNULL)
        {
            const HRESULT res = m_d3dDevice->SetRenderTarget(0, m_rtNull);
            eASSERT(!FAILED(res));
        }
        else
        {
            tex->bind(0, face);
        }
    }
    else
    {
        if (tex == eNULL)
        {
            const HRESULT res = m_d3dDevice->SetRenderTarget(index, eNULL);
            eASSERT(!FAILED(res));
        }
        else
        {
            tex->bind(index, face);
        }
    }
}

void eGraphicsApiDx9::setDepthTarget(eITexture2d *tex)
{
    eASSERT(tex != eNULL);

    if (tex == TARGET_SCREEN)
    {
        const HRESULT res = m_d3dDevice->SetDepthStencilSurface(m_screenDt);
        eASSERT(!FAILED(res));
    }
    else
    {
        tex->bind(0, eCMFACE_POSX);
    }
}

void eGraphicsApiDx9::setVertexBuffer(eU32 index, eIVertexBuffer *vb, eVertexType vertexType, eU32 byteOffset, eU32 instanceCount)
{
    if (vb == eNULL)
    {
        const HRESULT res = m_d3dDevice->SetStreamSource(index, eNULL, 0, 0);
        eASSERT(!FAILED(res));
    }
    else
    {
        vb->bind(index, vertexType, byteOffset, instanceCount);
    }
}

void eGraphicsApiDx9::setIndexBuffer(eIIndexBuffer *ib)
{
    if (ib == eNULL)
    {
        const HRESULT res = m_d3dDevice->SetIndices(eNULL);
        eASSERT(!FAILED(res));
    }
    else
    {
        ib->bind();
    }
}

void eGraphicsApiDx9::setTexture(eU32 unit, eITexture *tex)
{
    eASSERT(unit < MAX_TEX_UNITS);

    if (tex)
    {
        tex->bind(unit, eCMFACE_POSX, eTRUE);
    }
    else
    {
        const HRESULT res = m_d3dDevice->SetTexture(unit, eNULL);
        eASSERT(!FAILED(res));
    }
}

void eGraphicsApiDx9::drawPrimitives(ePrimitiveType type, eU32 startVertex, eU32 primitiveCount)
{
    eASSERT(primitiveCount > 0);

    D3DPRIMITIVETYPE d3dType;

    switch (type)
    {
        case ePRIMTYPE_TRIANGLELIST:
        {
            d3dType = D3DPT_TRIANGLELIST;
            m_renderStats.triangles += primitiveCount;
            m_renderStats.vertices += primitiveCount*3;
            break;
        }

        case ePRIMTYPE_TRIANGLESTRIPS:
        {
            d3dType = D3DPT_TRIANGLESTRIP;
            m_renderStats.triangles += primitiveCount;
            m_renderStats.vertices += primitiveCount+2;
            break;
        }

        case ePRIMTYPE_LINESTRIPS:
        {
            d3dType = D3DPT_LINESTRIP;
            m_renderStats.vertices += primitiveCount+1;
            m_renderStats.lines += primitiveCount;
            break;
        }

        case ePRIMTYPE_LINELIST:
        {
            d3dType = D3DPT_LINELIST;
            m_renderStats.vertices += primitiveCount*2;
            m_renderStats.lines += primitiveCount;
            break;
        }
    }

    m_renderStats.batches++;

    const HRESULT res = m_d3dDevice->DrawPrimitive(d3dType, startVertex, primitiveCount);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::drawIndexedPrimitives(ePrimitiveType type, eU32 startVertex, eU32 vertexCount, eU32 startIndex, eU32 primitiveCount, eU32 instanceCount)
{
    eASSERT(vertexCount > 0);
    eASSERT(primitiveCount > 0);

    const eU32 realInsts = (instanceCount == 0 ? 1 : instanceCount);

    D3DPRIMITIVETYPE d3dType;

    switch (type)
    {
        case ePRIMTYPE_TRIANGLELIST:
        {
            d3dType = D3DPT_TRIANGLELIST;
            m_renderStats.triangles += primitiveCount*realInsts;
            break;
        }

        case ePRIMTYPE_TRIANGLESTRIPS:
        {
            d3dType = D3DPT_TRIANGLESTRIP;
            m_renderStats.triangles += primitiveCount*realInsts;
            break;
        }

        case ePRIMTYPE_LINESTRIPS:
        {
            d3dType = D3DPT_LINESTRIP;
            m_renderStats.lines += primitiveCount*realInsts;
            break;
        }

        case ePRIMTYPE_LINELIST:
        {
            d3dType = D3DPT_LINELIST;
            m_renderStats.lines += primitiveCount*realInsts;
            break;
        }
    }

    m_renderStats.vertices += vertexCount*realInsts;
    m_renderStats.batches++;

    const HRESULT res = m_d3dDevice->DrawIndexedPrimitive(d3dType, startVertex, 0, vertexCount, startIndex, primitiveCount);
    eASSERT(!FAILED(res));
}

// Returns wether or not rendering the scene can be
// performed or not (because of possible lost device).
eBool eGraphicsApiDx9::renderStart()
{
    HRESULT res = m_d3dDevice->TestCooperativeLevel();

    if (res == D3DERR_DEVICELOST)
    {
        // Sleep in order to not consume time
        // while device is not resetable.
        eSleep(50);
        return eFALSE;
    }
    else if (res == D3DERR_DEVICENOTRESET)
    {
        // Device can be reset.
        eResourceManager::unloadAll();
        _resetDevice(m_wndWidth, m_wndHeight);
        eResourceManager::uploadAll();
        eStateManager::reset(); // Important, because old D3D resource handles are invalid now.
        eStateManager::forceApply();
    }

    // Reset render statistics.
    eMemSet(&m_renderStats, 0, sizeof(m_renderStats));

    // Put in marker into command buffer.
    /*
    res = m_d3dQueriesTsFreq[m_frameQuery]->Issue(D3DISSUE_END);
    eASSERT(!FAILED(res));
    res = m_d3dQueriesTsDisjoint[m_frameQuery]->Issue(D3DISSUE_BEGIN);
    eASSERT(!FAILED(res));
    res = m_d3dQueriesTsBegin[m_frameQuery]->Issue(D3DISSUE_END);
    eASSERT(!FAILED(res));
    */

    m_occlQuery[m_frame]->Issue(D3DISSUE_BEGIN);

    // Start rendering scene.
    res = m_d3dDevice->BeginScene();
    eASSERT(!FAILED(res));

    return eTRUE;
}

void eGraphicsApiDx9::renderEnd()
{
    HRESULT res = m_d3dDevice->EndScene();
    eASSERT(!FAILED(res));
    res = m_d3dDevice->Present(eNULL, eNULL, eNULL, eNULL);
    eASSERT(!FAILED(res));

    m_occlQuery[m_frame]->Issue(D3DISSUE_END);

    ++m_frame %= FRAMES;

    if (m_frame == 0)
    {
        m_startPull = eTRUE;
    }

    if (m_startPull)
    {
        eU32 dummy;
        while (m_occlQuery[m_frame]->GetData(&dummy, sizeof(dummy), D3DGETDATA_FLUSH) == S_FALSE)
        {
            Sleep(1);
        }
    }

    /*
    res = m_d3dQueriesTsEnd[m_frameQuery]->Issue(D3DISSUE_END);
    eASSERT(!FAILED(res));
    res = m_d3dQueriesTsDisjoint[m_frameQuery]->Issue(D3DISSUE_END);
    eASSERT(!FAILED(res));

    if (m_frameCollect >= 0)
    {
        while (m_d3dQueriesTsDisjoint[m_frameCollect]->GetData(eNULL, 0, 0) != S_OK)
        {
            Sleep(1);
        }

        eU64 startTimeStamp, endTimeStamp, timeStampFreq;
        BOOL isDisjoint;

        res = m_d3dQueriesTsDisjoint[m_frameCollect]->GetData(&isDisjoint, sizeof(BOOL), 0);
        eASSERT(!FAILED(res));
            
        if (!isDisjoint)
        {
            res = m_d3dQueriesTsBegin[m_frameCollect]->GetData((ePtr)&startTimeStamp, sizeof(startTimeStamp), 0);
            eASSERT(!FAILED(res));

            res = m_d3dQueriesTsEnd[m_frameCollect]->GetData((ePtr)&endTimeStamp, sizeof(endTimeStamp), 0);
            eASSERT(!FAILED(res));

            res = m_d3dQueriesTsFreq[m_frameCollect]->GetData((ePtr)&timeStampFreq, sizeof(timeStampFreq), 0);
            eASSERT(!FAILED(res));

            // Calculate time spend in GPU and frame rate.
            m_renderStats.gpuTimeMs = (eF32)((eF64)(endTimeStamp-startTimeStamp)/(eF64)timeStampFreq*1000.0);
        }
    }

    ++m_frameCollect %= 2;
    ++m_frameQuery %= 2;
    */

    m_renderStats.fps = _getFpsRate();
}

void eGraphicsApiDx9::flush()
{
}

#ifdef eDEBUG
eIVertexShader * eGraphicsApiDx9::createVertexShader(const eString &fileName) const
{
    return new eVertexShaderDx9(m_d3dDevice, fileName);
}

eIPixelShader * eGraphicsApiDx9::createPixelShader(const eString &fileName) const
{
    return new ePixelShaderDx9(m_d3dDevice, fileName);
}
#else
eIPixelShader * eGraphicsApiDx9::createPixelShader(eConstPtr data) const
{
    return new ePixelShaderDx9(m_d3dDevice, data);
}

eIVertexShader * eGraphicsApiDx9::createVertexShader(eConstPtr data) const
{
    return new eVertexShaderDx9(m_d3dDevice, data);
}
#endif

eIVertexBuffer * eGraphicsApiDx9::createVertexBuffer(eU32 byteSize, eBool dynamic) const
{
    return new eVertexBufferDx9(m_d3dDevice, m_d3dVDecls, byteSize, dynamic);
}

eIIndexBuffer *  eGraphicsApiDx9::createIndexBuffer(eU32 indexCount, eBool dynamic) const
{
    return new eIndexBufferDx9(m_d3dDevice, indexCount, dynamic);
}

eITexture2d * eGraphicsApiDx9::createTexture2d(eU32 width, eU32 height, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format) const
{
    return new eTexture2dDx9(m_d3dDevice, width, height, renderTarget, mipMapped, dynamic, format);
}

eITexture3d * eGraphicsApiDx9::createTexture3d(eU32 width, eU32 height, eU32 depth, eBool mipMapped, eBool dynamic, eFormat format)
{
    return new eTexture3dDx9(m_d3dDevice, width, height, depth, mipMapped, dynamic, format);
}

eITextureCube * eGraphicsApiDx9::createTextureCube(eU32 width, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format) const
{
    return new eTextureCubeDx9(m_d3dDevice, width, renderTarget, mipMapped, dynamic, format);
}

#ifndef eINTRO
eBool eGraphicsApiDx9::loadImage(const eByteArray &fileData, eColor *&image, eU32 &width, eU32 &height) const
{
    IDirect3DTexture9 *newTex = eNULL;

    if (FAILED(D3DXCreateTextureFromFileInMemoryEx(m_d3dDevice, &fileData[0], fileData.size(),
                                                   D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
                                                   D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
                                                   D3DX_DEFAULT, D3DX_DEFAULT, 0, eNULL, eNULL, &newTex)))
    {
        return eFALSE;
    }

    D3DSURFACE_DESC desc;

    newTex->GetLevelDesc(0, &desc);
    width = desc.Width;
    height = desc.Height;
    image = new eColor[width*height];
    eASSERT(image != eNULL);

    D3DLOCKED_RECT lr;

    newTex->LockRect(0, &lr, eNULL, D3DLOCK_READONLY);
    const eU8 *src = (eU8*)lr.pBits;

    for (eU32 i=0; i<width*height; i++)
    {
        const eU8 r = *src++;
        const eU8 g = *src++;
        const eU8 b = *src++;
        const eU8 a = *src++;

        image[i].set(b, g, r, a);
    }

    newTex->UnlockRect(0);
    eSAFE_RELEASE_COM(newTex);

    return eTRUE;
}
#endif

IDirect3DDevice9 * eGraphicsApiDx9::getDevice() const
{
    return m_d3dDevice;
}

eF32 eGraphicsApiDx9::_getFpsRate() const
{
    static const eU32 UPDATE_INTERVAL = 333;

    static eF32 fpsHolder = 0;
    static eU32 oldTime = 0;
    static eU32 frameCounter = 0;

    eU32 curTime = GetTickCount();
    frameCounter++;

    if (curTime-oldTime >= UPDATE_INTERVAL)
    {
        fpsHolder = (eF32)frameCounter*(1000.0f/(eF32)UPDATE_INTERVAL);
        frameCounter = 0;
        oldTime = curTime;
    }

    return fpsHolder;
}

eBool eGraphicsApiDx9::_enumerateResolutions()
{
    eU32 modeCount = m_d3dMain->GetAdapterModeCount(D3DADAPTER_DEFAULT,
                                                    D3DFMT_X8R8G8B8);

    if (!modeCount)
    {
        eShowError("Your graphics card doesn't support the X8R8G8B8 format!");
        return eFALSE;
    }

    for (eU32 i=0; i<modeCount; i++)
    {
        D3DDISPLAYMODE mode;
        m_d3dMain->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &mode);

        // Only list resolutions > 640x480.
        if (mode.Width >= 640 && mode.Height >= 480)
        {
            eBool found = eFALSE;
            
            for (eU32 i=0; i<m_resolutions.size(); i++)
            {
                const eSize &entry = m_resolutions[i];

                // Resolution was already added?
                if (entry.width == mode.Width && entry.height == mode.Height)
                {
                    found = eTRUE;
                    break;
                }
            }

            // There are many resolutions with same
            // width/height, but different HZ values,
            // so just add one auf them.
            if (!found)
            {
                eSize entry;

                entry.width = mode.Width;
                entry.height = mode.Height;

                m_resolutions.append(entry);
            }
        }
    }

    return eTRUE;
}

// Callback function for Direct3D window.
static LRESULT CALLBACK wndProc(HWND hwnd, eU32 msg, WPARAM wparam, LPARAM lparam)
{
    static eGraphicsApiDx9 *gfx = eNULL;

    switch (msg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT *cs = (CREATESTRUCT*)lparam;
            eASSERT(cs != eNULL);
            gfx = (eGraphicsApiDx9 *)cs->lpCreateParams;
            eASSERT(gfx != eNULL);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE:
        {
            const eU16 width = LOWORD(lparam);
            const eU16 height = HIWORD(lparam);

            gfx->resizeBackbuffer(width, height);
            return 0;
        }

        case WM_KEYDOWN:
        {
            switch(wparam)
            {
                case VK_ESCAPE:
                {
                    DestroyWindow(hwnd);
                    return 0;
                }
            }

            break;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

ePtr eGraphicsApiDx9::_createWindow(eU32 width, eU32 height, eBool fullScreen)
{
    // Register class and create window if needed.
    WNDCLASS wc;

    eMemSet(&wc, 0, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor       = LoadCursor(eNULL, IDC_ARROW);
    wc.lpfnWndProc   = wndProc;
    wc.lpszClassName = WINDOW_TITLE;
//    wc.hIcon    = LoadIcon(0 (LPCTSTR)IDI_ESTUDIO);
//    wc.hIconSm    = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    const ATOM res = RegisterClass(&wc);
    DWORD l = GetLastError();
    eASSERT(res != eNULL);

    // Create the window.
    if (m_fullScreen)
    {
        return CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_VISIBLE | WS_POPUP, 0, 0,
                            m_wndWidth, m_wndHeight, eNULL, eNULL, eNULL, this);
    }
    else
    {
        // In windowed mode adjust the window rect,
        // so that the client area of the window has
        // the size of the desired resolution.
        const eU32 style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

        RECT r;

        r.left = 0;
        r.top = 0;
        r.right = m_wndWidth;
        r.bottom = m_wndHeight;

        AdjustWindowRect(&r, style, FALSE);    

        return CreateWindow(WINDOW_TITLE, WINDOW_TITLE, style, CW_USEDEFAULT, CW_USEDEFAULT,
                            r.right-r.left, r.bottom-r.top, eNULL, eNULL, eNULL, this);
    }
}

void eGraphicsApiDx9::resizeBackbuffer(eU32 width, eU32 height)
{
    // Only resize back-buffer if size has changed.
    if (m_wndWidth != width || m_wndHeight != height)
    {
        eResourceManager::unloadAll();
        _resetDevice(width, height);
        eResourceManager::uploadAll();

        eStateManager::reset(); // Important, because old D3D resource handles are invalid now.
        eStateManager::setViewport(0, 0, width, height);
        eStateManager::forceApply();
    }
}

void eGraphicsApiDx9::_resetDevice(eU32 width, eU32 height)
{
    m_wndWidth = width;
    m_wndHeight = height;
    m_d3dPp->BackBufferWidth = width;
    m_d3dPp->BackBufferHeight = height;

    _releaseNullTarget();
    _releaseScreenTargets();
    _releaseQueries();

    if (SUCCEEDED(m_d3dDevice->Reset(m_d3dPp)))
    {
        _getScreenTargets();
        _createNullTarget();
        _createQueries();
    }
}

void eGraphicsApiDx9::_createVertexDecls()
{
    for (eInt i=0; i<eVTXTYPE_COUNT-1; i++)
    {
        const HRESULT res = m_d3dDevice->CreateVertexDeclaration(VERTEX_DECLS[i], &m_d3dVDecls[i]);
        eASSERT(!FAILED(res));
    }
}

void eGraphicsApiDx9::_releaseVertexDecls()
{
    for (eU32 i=0; i<eVTXTYPE_COUNT; i++)
    {
        eSAFE_RELEASE_COM(m_d3dVDecls[i]);
    }
}

void eGraphicsApiDx9::_createNullTarget()
{
    const HRESULT res = m_d3dDevice->CreateRenderTarget(m_d3dCaps->MaxTextureWidth, m_d3dCaps->MaxTextureHeight,
                                                        FOURCC_NULL, D3DMULTISAMPLE_NONE, 0, FALSE, &m_rtNull, eNULL);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::_releaseNullTarget()
{
    eSAFE_RELEASE_COM(m_rtNull);
}

void eGraphicsApiDx9::_getScreenTargets()
{
    HRESULT res = m_d3dDevice->GetRenderTarget(0, &m_screenRt);
    eASSERT(!FAILED(res));
    
    res = m_d3dDevice->GetDepthStencilSurface(&m_screenDt);
    eASSERT(!FAILED(res));
}

void eGraphicsApiDx9::_releaseScreenTargets()
{
    eSAFE_RELEASE_COM(m_screenRt);
    eSAFE_RELEASE_COM(m_screenDt);
}

void eGraphicsApiDx9::_createQueries()
{
    /*
    for (eU32 i=0; i<2; i++)
    {
        HRESULT res = m_d3dDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &m_d3dQueriesTsBegin[i]);
        eASSERT(!FAILED(res));

        res = m_d3dDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &m_d3dQueriesTsEnd[i]);
        eASSERT(!FAILED(res));

        res = m_d3dDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, &m_d3dQueriesTsFreq[i]);
        eASSERT(!FAILED(res));

        res = m_d3dDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, &m_d3dQueriesTsDisjoint[i]);
        eASSERT(!FAILED(res));
    }
    */

    for (eInt i=0; i<FRAMES; i++)
    {
        const HRESULT res = m_d3dDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_occlQuery[i]);
        eASSERT(!FAILED(res));
    }
}

void eGraphicsApiDx9::_releaseQueries()
{
    /*
    for (eU32 i=0; i<2; i++)
    {
        eSAFE_RELEASE_COM(m_d3dQueriesTsBegin[i]);
        eSAFE_RELEASE_COM(m_d3dQueriesTsEnd[i]);
        eSAFE_RELEASE_COM(m_d3dQueriesTsFreq[i]);
        eSAFE_RELEASE_COM(m_d3dQueriesTsDisjoint[i]);
    }
    */

    for (eInt i=0; i<FRAMES; i++)
    {
        eSAFE_RELEASE_COM(m_occlQuery[i]);
    }
}








/**********************************************************************************
 ****** Methods from eGraphicsApiDx9 ************************************************
 **********************************************************************************/

eITexture2d * eGraphicsApiDx9::TARGET_SCREEN = (eITexture2d *)0xdeadbeef;

eBool eITexture::unload()
{
    // Make sure this texture isn't bound to any state anymore.
    for (eU32 i=0; i<eGraphicsApiDx9::MAX_TARGETS; i++)
    {
        if (eStateManager::getNewStates().texture[i] == this)
        {
            eStateManager::getNewStates().texture[i] = eNULL;
        }

        if (eStateManager::getActiveStates().texture[i] == this)
        {
            eStateManager::getActiveStates().texture[i] = eNULL;
        }

        for (eU32 j=0; j<eStateManager::getStatesStackSize(); j++)
        {
            if (eStateManager::getStackStates(j).texture[i] == this)
            {
                eStateManager::getStackStates(j).texture[i] = eNULL;
            }
        }
    }

    return eTRUE;
}

eBool eIIndexBuffer::unload()
{
    // Make sure this index buffer isn't bound to any state anymore.
    if (eStateManager::getNewStates().indexBuffer == this)
    {
        eStateManager::getNewStates().indexBuffer = eNULL;
    }

    if (eStateManager::getActiveStates().indexBuffer == this)
    {
        eStateManager::getActiveStates().indexBuffer = eNULL;
    }

    for (eU32 j=0; j<eStateManager::getStatesStackSize(); j++)
    {
        if (eStateManager::getStackStates(j).indexBuffer == this)
        {
            eStateManager::getStackStates(j).indexBuffer = eNULL;
        }
    }

    return eTRUE;
}

eBool eIVertexBuffer::unload()
{
    // Make sure this vertex buffer isn't bound to any state anymore.
    for (eU32 i=0; i<2; i++)
    {
        eIVertexBuffer *& vbNew = eStateManager::getNewStates().vtxBufInfos[i].vertexBuffer;
        eIVertexBuffer *& vbActive = eStateManager::getNewStates().vtxBufInfos[i].vertexBuffer;

        if (vbNew == this)
        {
            vbNew = eNULL;
        }

        if (vbActive == this)
        {
            vbActive = eNULL;
        }

        for (eU32 j=0; j<eStateManager::getStatesStackSize(); j++)
        {
            eIVertexBuffer *& vb = eStateManager::getNewStates().vtxBufInfos[i].vertexBuffer;

            if (vb == this)
            {
                vb = eNULL;
            }
        
        }
    }

    return eTRUE;
}

eITexture2d * eGraphicsApiDx9::createChessTexture(eU32 width, eU32 height, eU32 step, const eColor &col0, const eColor &col1) const
{
    eASSERT(width > 2);
    eASSERT(height > 2);
    eASSERT(step < width);
    eASSERT(step < height);
    eASSERT(width%step == 0);
    eASSERT(height%step == 0);

    eITexture2d *tex = createTexture2d(width, height, eFALSE, eTRUE, eFALSE, eFORMAT_ARGB8);
    eColor *data = (eColor *)tex->lock();

    const eColor colors[2] =
    {
        col0,
        col1
    };

    for (eU32 y=0, index=0; y<height; y++)
    {
        const eU32 yds = y/step;

        for (eU32 x=0; x<width; x++)
        {
            const eU32 col = ((x/step)+yds)%2;
            data[index++] = colors[col];
        }
    }

    tex->unlock();
    return tex;
}

void eGraphicsApiDx9::setActiveMatrices(const eMatrix4x4 &modelMtx, const eMatrix4x4 &viewMtx, const eMatrix4x4 &projMtx)
{
    m_activeModelMtx = modelMtx;
    m_activeViewMtx = viewMtx;
    m_activeModelViewMtx = modelMtx*viewMtx;
    m_activeProjMtx = projMtx;

    const eMatrix4x4 mvpMtx = m_activeModelViewMtx*projMtx;

    setVsConst(eVSCONST_VIEW_MATRIX, viewMtx);
    setVsConst(eVSCONST_PROJ_MATRIX, projMtx);
    setVsConst(eVSCONST_MVP_MATRIX, mvpMtx);
}

void eGraphicsApiDx9::setPsConst(eU32 offset, eF32 f)
{
    setPsConst(offset, eVector2(f));
}

void eGraphicsApiDx9::setPsConst(eU32 offset, const eMatrix4x4 &m)
{
    setPsConst(offset, 4, m.m);
}

void eGraphicsApiDx9::setPsConst(eU32 offset, const eColor &v)
{
    setPsConst(offset, 1, v);
}

void eGraphicsApiDx9::setPsConst(eU32 offset, const eVector4 &v)
{
    setPsConst(offset, 1, v);
}

void eGraphicsApiDx9::setPsConst(eU32 offset, const eVector3 &v)
{
    const eF32 c[4] = {v.x, v.y, v.z, 0.0f};
    setPsConst(offset, 1, c);
}

void eGraphicsApiDx9::setPsConst(eU32 offset, const eVector2 &v)
{
    const eF32 c[4] = {v.x, v.y, 0.0f, 0.0f};
    setPsConst(offset, 1, c);
}

void eGraphicsApiDx9::setVsConst(eU32 offset, eF32 f)
{
    setVsConst(offset, eVector2(f));
}

void eGraphicsApiDx9::setVsConst(eU32 offset, const eMatrix4x4 &m)
{
    setVsConst(offset, 4, m);
}

void eGraphicsApiDx9::setVsConst(eU32 offset, const eVector4 &v)
{
    setVsConst(offset, 1, v);
}

void eGraphicsApiDx9::setVsConst(eU32 offset, const eVector3 &v)
{
    const eF32 c[4] = {v.x, v.y, v.z, 0.0f};
    setVsConst(offset, 1, c);
}

void eGraphicsApiDx9::setVsConst(eU32 offset, const eVector2 &v)
{
    const eF32 c[4] = {v.x, v.y, 0.0f, 0.0f};
    setVsConst(offset, 1, c);
}

eMatrix4x4 eGraphicsApiDx9::getActiveViewMatrix() const
{
    return m_activeViewMtx;
}

eMatrix4x4 eGraphicsApiDx9::getActiveModelMatrix() const
{
    return m_activeModelMtx;
}

eMatrix4x4 eGraphicsApiDx9::getActiveProjectionMatrix() const
{
    return m_activeProjMtx;
}

void eGraphicsApiDx9::getBillboardVectors(eVector3 &right, eVector3 &up, eVector3 *view) const
{
    right.set(m_activeViewMtx.m11, m_activeViewMtx.m21, m_activeViewMtx.m31);
    up.set(m_activeViewMtx.m12, m_activeViewMtx.m22, m_activeViewMtx.m32);

    right.normalize();
    up.normalize();

    if (view)
    {
        view->set(m_activeViewMtx.m13, m_activeViewMtx.m23, m_activeViewMtx.m33);
        view->normalize();
    }
}

/**********************************************************************************
 ****** /Methods from eGraphicsApiDx9 ***********************************************
 **********************************************************************************/
