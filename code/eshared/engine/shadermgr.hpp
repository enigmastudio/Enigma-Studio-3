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

#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

// In editor load shaders from file (for live editing),
// in player load shaders from arrays (for size).
#ifdef eDEBUG
    #define ePS(name) #name
    #define eVS(name) #name
#else
    #define ePS(name) ps_##name
    #define eVS(name) vs_##name
#endif

class eShaderManager
{
private:
    struct ShaderEntry
    {
        enum Type
        {
            TYPE_VS,
            TYPE_PS
        };

        Type                    type;
        eIShader *              shader;
        eU32                    hash;

#ifdef eDEBUG
        eString                 filePath;
        eS64                    lastTime;
#endif
    };

    typedef eArray<ShaderEntry *> ShaderEntryPtrArray;

public:
    static void                 initialize(eGraphicsApiDx9 *gfx);
    static void                 update();
    static void                 shutdown();

#ifdef eDEBUG
    static void                 setShaderFolder(const eString &shaderFolder);

    static eIPixelShader *      loadPixelShader(const eString &fileName);
    static eIVertexShader *     loadVertexShader(const eString &fileName);
#else
    static eIPixelShader *      loadPixelShader(eConstPtr data);
    static eIVertexShader *     loadVertexShader(eConstPtr data);
#endif

private:
    static eIShader *           _findShader(eU32 hash);

#ifdef eDEBUG
    static void                 _reloadIfShaderChanged(ShaderEntry &se);
    static eS64                 _getFileChangedTime(const eString &fileName);
#endif

private:
    static ShaderEntryPtrArray  m_shaders;
    static eGraphicsApiDx9 *      m_gfx;

#ifdef eDEBUG
    static eString              m_shaderFolder;
#endif
};

#endif // SHADER_MANAGER_HPP