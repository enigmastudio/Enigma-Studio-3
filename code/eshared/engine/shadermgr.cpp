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

eGraphicsApiDx9 *                     eShaderManager::m_gfx = eNULL;
eShaderManager::ShaderEntryPtrArray eShaderManager::m_shaders;

#ifdef eDEBUG
eString                             eShaderManager::m_shaderFolder("../code/eshared/engine/shaders/");
#endif

void eShaderManager::initialize(eGraphicsApiDx9 *gfx)
{
    eASSERT(gfx != eNULL);
    m_gfx = gfx;
}

void eShaderManager::update()
{
#ifdef eDEBUG
    for (eU32 i=0; i<m_shaders.size(); i++)
    {
        _reloadIfShaderChanged(*m_shaders[i]);
    }
#endif
}

void eShaderManager::shutdown()
{
    for (eU32 i=0; i<m_shaders.size(); i++)
    {
        eSAFE_DELETE(m_shaders[i]->shader);
        eSAFE_DELETE(m_shaders[i]);
    }

    m_shaders.clear();
}

#ifdef eDEBUG
void eShaderManager::setShaderFolder(const eString &shaderFolder)
{
    m_shaderFolder = shaderFolder;
}

eIPixelShader * eShaderManager::loadPixelShader(const eString &name)
{
    eASSERT(m_gfx != eNULL);

    const eString fileName = name+".ps";
    const eU32 hash = eHashStr(fileName);

    eIPixelShader *shader = (eIPixelShader *)_findShader(hash);
    
    if (shader == eNULL)
    {
        const eString filePath = m_shaderFolder+fileName;
        shader = m_gfx->createPixelShader(filePath);

        if (shader)
        {
            ShaderEntry *se = new ShaderEntry;
            eASSERT(se != eNULL);

            se->type     = ShaderEntry::TYPE_PS;
            se->hash     = hash;
            se->filePath = filePath;
            se->shader   = shader;
            se->lastTime = _getFileChangedTime(filePath);

            m_shaders.append(se);
        }
    }

    return shader;
}

eIVertexShader * eShaderManager::loadVertexShader(const eString &name)
{
    eASSERT(m_gfx != eNULL);

    const eString fileName = name+".vs";
    const eU32 hash = eHashStr(fileName);

    eIVertexShader *shader = (eIVertexShader *)_findShader(hash);
    
    if (shader == eNULL)
    {
        const eString filePath = m_shaderFolder+fileName;
        shader = m_gfx->createVertexShader(filePath);

        if (shader)
        {
            ShaderEntry *se = new ShaderEntry;
            eASSERT(se != eNULL);

            se->type     = ShaderEntry::TYPE_VS;
            se->hash     = hash;
            se->filePath = filePath;
            se->shader   = shader;
            se->lastTime = _getFileChangedTime(se->filePath);

            m_shaders.append(se);
        }
    }

    return shader;
}
#else
eIPixelShader * eShaderManager::loadPixelShader(eConstPtr data)
{
    eASSERT(data != eNULL);
    eASSERT(m_gfx != eNULL);

    const eU32 hash = (eU32)data;
    eIPixelShader *shader = (eIPixelShader *)_findShader(hash);
    
    if (shader == eNULL)
    {
        shader = m_gfx->createPixelShader(data);

        if (shader)
        {
            ShaderEntry *se = new ShaderEntry;
            eASSERT(se != eNULL);

            se->type   = ShaderEntry::TYPE_PS;
            se->hash   = hash;
            se->shader = shader;

            m_shaders.append(se);
        }
    }

    return shader;
}

eIVertexShader * eShaderManager::loadVertexShader(eConstPtr data)
{
    eASSERT(data != eNULL);
    eASSERT(m_gfx != eNULL);

    const eU32 hash = (eU32)data;
    eIVertexShader *shader = (eIVertexShader *)_findShader(hash);

    if (shader == eNULL)
    {
        shader = m_gfx->createVertexShader(data);

        if (shader)
        {
            ShaderEntry *se = new ShaderEntry;
            eASSERT(se != eNULL);

            se->type   = ShaderEntry::TYPE_VS;
            se->hash   = hash;
            se->shader = shader;

            m_shaders.append(se);
        }
    }

    return shader;
}
#endif

eIShader * eShaderManager::_findShader(eU32 hash)
{
    for (eU32 i=0; i<m_shaders.size(); i++)
    {
        if (m_shaders[i]->hash == hash)
        {
            return m_shaders[i]->shader;
        }
    }

    return eNULL;
}

#ifdef eDEBUG
#include <sys/stat.h>

void eShaderManager::_reloadIfShaderChanged(ShaderEntry &se)
{
    const eS64 newTime = _getFileChangedTime(se.filePath);

    if (newTime != -1 && newTime > se.lastTime)
    {
        se.shader->load(se.filePath);
    }
}

eS64 eShaderManager::_getFileChangedTime(const eString &fileName)
{
    struct _stat stat;

    if (_stat(fileName, &stat) == 0)
    {
        return stat.st_mtime;
    }

    return -1;
}
#endif