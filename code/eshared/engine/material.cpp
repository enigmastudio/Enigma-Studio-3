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

#include "../system/system.hpp"
#include "../math/math.hpp"
#include "engine.hpp"

eITexture2d * eMaterial::m_whiteTex = eNULL;
eITexture2d * eMaterial::m_normalMap = eNULL;
eMaterial     eMaterial::m_defaultMat;
eMaterial     eMaterial::m_wireframeMat;

eMaterial::eMaterial() :
    m_cullingMode(eCULLING_NONE),
    m_blendSrc(eBLEND_ONE),
    m_blendDst(eBLEND_ONE),
    m_blendOp(eBLENDOP_ADD),
    m_useBlending(eFALSE),
    m_flat(eFALSE),
    m_zFunc(eZFUNC_LESS),
    m_zBuffer(eTRUE),
    m_zMask(eTRUE),
    m_lighted(eTRUE),
    m_renderPass(0),
    m_useRefraction(eFALSE),
    m_refractionCol(eColor::WHITE),
    m_diffuseCol(eColor::WHITE),
    m_specularCol(eColor::WHITE),
    m_shininess(0.25f),
    m_refracIntensity(0.1f),
    m_pointLineSize(0.1f),
    m_assignment_pass_id(0)
{
    for (eU32 i=0; i<UNIT_COUNT; i++)
    {
        m_textures[i] = eNULL;
        m_texFilters[i] = eTEXFILTER_BILINEAR;
        m_texAddrModes[i] = eTEXADDRMODE_WRAP;
    }
}

void eMaterial::activate(eGraphicsApiDx9 *gfx, eBool allowBlending) const
{
    eASSERT(gfx != eNULL);

    for (eU32 i=0; i<UNIT_COUNT; i++)
    {
        if (m_textures[i])
        {
            eStateManager::setTextureFilter(i, m_texFilters[i]);
            eStateManager::setTextureAddressMode(i, m_texAddrModes[i]);
            eStateManager::bindTexture(i, m_textures[i]);
        }
        else
        {
            eStateManager::bindTexture(i, eNULL);
        }
    }

    // Set default textures if no textures bound.
    if (m_textures[UNIT_DIFFUSE] == eNULL)
    {
        _createWhiteTexture(gfx);
        eStateManager::bindTexture(UNIT_DIFFUSE, m_whiteTex);
    }

    if (m_textures[UNIT_NORMAL] == eNULL)
    {
        _createNormalMap(gfx);
        eStateManager::bindTexture(UNIT_NORMAL, m_normalMap);
    }

    // Apply all other state changes.
    eStateManager::setCullingMode(m_cullingMode);
    eStateManager::setBlendModes(m_blendSrc, m_blendDst, m_blendOp);
    eStateManager::setCap(eCAP_BLENDING, m_useBlending && allowBlending);
    eStateManager::setCap(eCAP_ZBUFFER, m_zBuffer);
    eStateManager::setCap(eCAP_ZWRITE, m_zMask);
    eStateManager::setAlphaTest(m_useBlending && allowBlending);

    // Set shader constants.
    gfx->setPsConst(ePSCONST_MAT_REFRACTION, m_refractionCol);
    gfx->setPsConst(ePSCONST_MAT_REFRINTENSITY, m_refracIntensity);
    gfx->setPsConst(ePSCONST_MAT_DIFFUSE, m_diffuseCol);
    gfx->setPsConst(ePSCONST_MAT_SPECULAR, m_specularCol);
    gfx->setPsConst(ePSCONST_MAT_SHININESS, m_shininess);
}

eU32 eMaterial::getSortKey() const
{
    eU32 key = 0;

    key |= (m_renderPass<<24);
    key |= ((eU32)m_useBlending<<16);
    key |= (eU16)((eU32)m_textures[0]^(eU32)m_textures[1]);

    return key;
}

// The render pass is not allowed to be greater than
// 255, because it may only occupy one byte in the
// sort key.
void eMaterial::setRenderPass(eU32 renderPass)
{
    eASSERT(renderPass < 256);
    m_renderPass = renderPass;
}

void eMaterial::setTexture(TextureUnit unit, eITexture *tex)
{
    eASSERT(unit < UNIT_COUNT);
    m_textures[unit] = tex;
}

void eMaterial::setTextureFilter(TextureUnit unit, eTextureFilter texFilter)
{
    eASSERT(unit < UNIT_COUNT);
    m_texFilters[unit] = texFilter;
}

void eMaterial::setTextureAddressMode(TextureUnit unit, eTextureAddressMode texAddrMode)
{
    m_texAddrModes[unit] = texAddrMode;
}

void eMaterial::setCullingMode(eCullingMode cullingMode)
{
    m_cullingMode = cullingMode;
}

void eMaterial::setBlending(eBlendMode src, eBlendMode dst, eBlendOp op)
{
    m_blendSrc = src;
    m_blendDst = dst;
    m_blendOp  = op;
}

void eMaterial::setDiffuseColor(const eColor &diffuse)
{
    m_diffuseCol = diffuse;
}

void eMaterial::setSpecularColor(const eColor &specular)
{
    m_specularCol = specular;
}

void eMaterial::setShininess(eF32 shininess)
{
    m_shininess = shininess;
}

void eMaterial::setRefractionColor(const eColor &refraction)
{
    m_refractionCol = refraction;
}

void eMaterial::setRefractionIntensity(eF32 intensity)
{
    m_refracIntensity = intensity;
}

void eMaterial::setUseBlending(eBool useBlending)
{
    m_useBlending = useBlending;
}

void eMaterial::setUseRefraction(eBool useRefraction)
{
    m_useRefraction = useRefraction;
}

void eMaterial::setFlatShaded(eBool flatShaded)
{
    m_flat = flatShaded;
}

void eMaterial::setZBuffer(eBool zBuffer)
{
    m_zBuffer = zBuffer;
}

void eMaterial::setZFunction(eZFunction zFunc)
{
    m_zFunc = zFunc;
}

void eMaterial::setZMask(eBool zMask)
{
    m_zMask = zMask;
}

void eMaterial::setLighted(eBool lighted)
{
    m_lighted = lighted;
}

void eMaterial::setPointSize(eF32 pointLineSize)
{
    eASSERT(pointLineSize > 0.0f);
    m_pointLineSize = pointLineSize;
}

eU32 eMaterial::getRenderPass() const
{
    return m_renderPass;
}

eITexture * eMaterial::getTexture(TextureUnit unit) const
{
    eASSERT(unit < UNIT_COUNT);
    return m_textures[unit];
}

eTextureFilter eMaterial::getTextureFilter(TextureUnit unit) const
{
    eASSERT(unit < UNIT_COUNT);
    return m_texFilters[unit];
}

eTextureAddressMode eMaterial::getTextureAddressMode(TextureUnit unit) const
{
    return m_texAddrModes[unit];
}

eCullingMode eMaterial::getCullingMode() const
{
    return m_cullingMode;
}

void eMaterial::getBlending(eBlendMode &src, eBlendMode &dst, eBlendOp &op) const
{
    src = m_blendSrc;
    dst = m_blendDst;
    op  = m_blendOp;
}

const eColor & eMaterial::getDiffuseColor() const
{
    return m_diffuseCol;
}

const eColor & eMaterial::getSpecularColor() const
{
    return m_specularCol;
}

eF32 eMaterial::getShininess() const
{
    return m_shininess;
}

const eColor & eMaterial::getRefractionColor() const
{
    return m_refractionCol;
}

eF32 eMaterial::getRefractionIntensity() const
{
    return m_refracIntensity;
}

eBool eMaterial::getUseBlending() const
{
    return m_useBlending;
}

eBool eMaterial::getUseRefraction() const
{
    return m_useRefraction;
}

eBool eMaterial::getFlatShaded() const
{
    return m_flat;
}

eBool eMaterial::getZBuffer() const
{
    return m_zBuffer;
}

eBool eMaterial::getZMask() const
{
    return m_zMask;
}

eZFunction eMaterial::getZFunction() const
{
    return m_zFunc;
}

eBool eMaterial::getLighted() const
{
    return m_lighted;
}

eF32 eMaterial::getPointSize() const
{
    return m_pointLineSize;
}

void eMaterial::initialize()
{
    m_wireframeMat.setLighted(eFALSE);
}

void eMaterial::shutdown()
{
    eSAFE_DELETE(m_whiteTex);
    eSAFE_DELETE(m_normalMap);
}

void eMaterial::setDefault(const eMaterial &mat)
{
    m_defaultMat = mat;
}

const eMaterial * eMaterial::getDefault()
{
    return &m_defaultMat;
}

const eMaterial * eMaterial::getWireframe()
{
    return &m_wireframeMat;
}

void eMaterial::_createWhiteTexture(eGraphicsApiDx9 *gfx)
{
    eASSERT(gfx != eNULL);

    if (m_whiteTex == eNULL)
    {
        m_whiteTex = gfx->createTexture2d(1, 1, eFALSE, eFALSE, eFALSE, eFORMAT_ARGB8);
        ePtr buffer = m_whiteTex->lock();
        eMemSet(buffer, 255, 4);
        m_whiteTex->unlock();
    }
}

void eMaterial::_createNormalMap(eGraphicsApiDx9 *gfx)
{
    eASSERT(gfx != eNULL);

    if (m_normalMap == eNULL)
    {
        m_normalMap = gfx->createTexture2d(1, 1, eFALSE, eFALSE, eFALSE, eFORMAT_ARGB8);
        eColor *buffer = (eColor *)m_normalMap->lock();
        buffer[0] = eColor(128, 128, 255); // Up-pointing normal vector <0.0f, 0.0f, 1.0f>.
        m_normalMap->unlock();
    }
}