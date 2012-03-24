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

#ifndef SHADERS_DX9_HPP
#define SHADERS_DX9_HPP

class ePixelShaderDx9 : public eIPixelShader
{
private:
    ePixelShaderDx9();
    ePixelShaderDx9(const ePixelShaderDx9 &ps);
    ePixelShaderDx9 & operator = (const ePixelShaderDx9 &ps);

public:
#ifdef eDEBUG
    ePixelShaderDx9(IDirect3DDevice9 *d3dDev, const eString &fileName);
#else
    ePixelShaderDx9(IDirect3DDevice9 *d3dDev, eConstPtr data);
#endif

    virtual ~ePixelShaderDx9();

#ifdef eDEBUG
    virtual eBool               load(const eString &fileName);
#else
    virtual eBool               load(eConstPtr data);
#endif

    virtual eBool               bind();
    virtual eBool               upload();
    virtual eBool               unload();

private:
    IDirect3DPixelShader9 *     m_d3dPs;
    IDirect3DDevice9 *          m_d3dDev;
};

class eVertexShaderDx9 : public eIVertexShader
{
private:
    eVertexShaderDx9();
    eVertexShaderDx9(const eVertexShaderDx9 &ps);
    eVertexShaderDx9 & operator = (const eVertexShaderDx9 &ps);

public:
#ifdef eDEBUG
    eVertexShaderDx9(IDirect3DDevice9 *d3dDev, const eString &fileName);
#else
    eVertexShaderDx9(IDirect3DDevice9 *d3dDev, eConstPtr data);
#endif

    virtual ~eVertexShaderDx9();

#ifdef eDEBUG
    virtual eBool               load(const eString &fileName);
#else
    virtual eBool               load(eConstPtr data);
#endif

    virtual eBool               bind();
    virtual eBool               upload();
    virtual eBool               unload();

private:
    IDirect3DVertexShader9 *    m_d3dVs;
    IDirect3DDevice9 *          m_d3dDev;
};

#endif // SHADERS_DX9_HPP