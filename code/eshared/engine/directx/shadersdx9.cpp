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

#include <d3d9.h>
#include <d3dx9.h>

#include "../../eshared.hpp"
#include "shadersdx9.hpp"

ePixelShaderDx9::ePixelShaderDx9()
{
}

ePixelShaderDx9::ePixelShaderDx9(const ePixelShaderDx9 &ps)
{
}

ePixelShaderDx9 & ePixelShaderDx9::operator = (const ePixelShaderDx9 &ps)
{
    return *this;
}

#ifdef eDEBUG
ePixelShaderDx9::ePixelShaderDx9(IDirect3DDevice9 *d3dDev, const eString &fileName) :
    m_d3dDev(d3dDev),
    m_d3dPs(eNULL)
{
    eASSERT(d3dDev != eNULL);
    load(fileName);
}
#else
ePixelShaderDx9::ePixelShaderDx9(IDirect3DDevice9 *d3dDev, eConstPtr data) :
    m_d3dDev(d3dDev),
    m_d3dPs(eNULL)
{
    eASSERT(d3dDev != eNULL);
    eASSERT(data != eNULL);

    load(data);
}
#endif

ePixelShaderDx9::~ePixelShaderDx9()
{
    eSAFE_RELEASE_COM(m_d3dPs);
}

#ifdef eDEBUG
eBool ePixelShaderDx9::load(const eString &fileName)
{
    eSAFE_RELEASE_COM(m_d3dPs);

    LPD3DXBUFFER errMsg, binary;
    HRESULT res = D3DXCompileShaderFromFile(fileName, eNULL, eNULL, "main", "ps_3_0",
                                            D3DXSHADER_PACKMATRIX_ROWMAJOR, &binary, &errMsg, eNULL);

    if (FAILED(res))
    {
        if (res == E_FAIL)
        {
            eShowError(eString("Couldn't load pixel shader \"")+fileName+"\"!");
        }
        else if (errMsg)
        {
            eShowError((const eChar *)errMsg->GetBufferPointer());
        }

        eASSERT(eFALSE);
        return eFALSE;
    }

    eByteArray output(binary->GetBufferSize());
    eMemCopy(&output[0], binary->GetBufferPointer(), binary->GetBufferSize());
    res = m_d3dDev->CreatePixelShader((const DWORD *)&output[0], &m_d3dPs);
    eASSERT(!FAILED(res));

    return eTRUE;
}
#else
eBool ePixelShaderDx9::load(eConstPtr data)
{
    eASSERT(data != eNULL);

    eSAFE_RELEASE_COM(m_d3dPs);
    const HRESULT res = m_d3dDev->CreatePixelShader((const DWORD *)data, &m_d3dPs);
    eASSERT(!FAILED(res));

    return eTRUE;
}
#endif

eBool ePixelShaderDx9::bind()
{
    eASSERT(m_d3dPs != eNULL);

    const HRESULT res = m_d3dDev->SetPixelShader(m_d3dPs);
    eASSERT(!FAILED(res));

    return eTRUE;
}

eBool ePixelShaderDx9::upload()
{
    // Nothing todo for shaders.
    return eTRUE;
}

eBool ePixelShaderDx9::unload()
{
    // Nothing todo for shaders.
    return eTRUE;
}

eVertexShaderDx9::eVertexShaderDx9()
{
}

eVertexShaderDx9::eVertexShaderDx9(const eVertexShaderDx9 &ps)
{
}

eVertexShaderDx9 & eVertexShaderDx9::operator = (const eVertexShaderDx9 &ps)
{
    return *this;
}

#ifdef eDEBUG
eVertexShaderDx9::eVertexShaderDx9(IDirect3DDevice9 *d3dDev, const eString &fileName) :
    m_d3dDev(d3dDev),
    m_d3dVs(eNULL)
{
    eASSERT(d3dDev != eNULL);
    load(fileName);
}
#else
eVertexShaderDx9::eVertexShaderDx9(IDirect3DDevice9 *d3dDev, eConstPtr data) :
    m_d3dDev(d3dDev),
    m_d3dVs(eNULL)
{
    eASSERT(d3dDev != eNULL);
    eASSERT(data != eNULL);

    load(data);
}
#endif

eVertexShaderDx9::~eVertexShaderDx9()
{
    eSAFE_RELEASE_COM(m_d3dVs);
}

#ifdef eDEBUG
eBool eVertexShaderDx9::load(const eString &fileName)
{
    eSAFE_RELEASE_COM(m_d3dVs);

    LPD3DXBUFFER errMsg, binary;
    HRESULT res = D3DXCompileShaderFromFile(fileName, eNULL, eNULL, "main", "vs_3_0",
                                            D3DXSHADER_PACKMATRIX_ROWMAJOR, &binary, &errMsg, eNULL);

    if (FAILED(res))
    {
        if (res == E_FAIL)
        {
            eShowError(eString("Couldn't load vertex shader \"")+fileName+"\"!");
        }
        else if (errMsg)
        {
            eShowError((const eChar *)errMsg->GetBufferPointer());
        }

        eASSERT(eFALSE);
        return eFALSE;
    }

    eByteArray output(binary->GetBufferSize());
    eMemCopy(&output[0], binary->GetBufferPointer(), binary->GetBufferSize());
    res = m_d3dDev->CreateVertexShader((const DWORD *)&output[0], &m_d3dVs);
    eASSERT(!FAILED(res));

    return eTRUE;
}
#else
eBool eVertexShaderDx9::load(eConstPtr data)
{
    eASSERT(data != eNULL);

    eSAFE_RELEASE_COM(m_d3dVs);
    const HRESULT res = m_d3dDev->CreateVertexShader((const DWORD *)data, &m_d3dVs);
    eASSERT(!FAILED(res));

    return eTRUE;
}
#endif

eBool eVertexShaderDx9::bind()
{
    eASSERT(m_d3dVs != eNULL);

    const HRESULT res = m_d3dDev->SetVertexShader(m_d3dVs);
    eASSERT(!FAILED(res));

    return eTRUE;
}

eBool eVertexShaderDx9::upload()
{
    // Nothing todo for shaders.
    return eTRUE;
}

eBool eVertexShaderDx9::unload()
{
    // Nothing todo for shaders.
    return eTRUE;
}