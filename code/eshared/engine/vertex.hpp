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

#ifndef VERTEX_HPP
#define VERTEX_HPP

// Standard vertex used for most rendering purposes.
struct eVertex
{
    eVertex(const eVector3 &pos, const eVector3 &nrm, const eVector2 &uv, const eColor &col) :
        position(pos),
        normal(nrm),
        texCoord(uv),
        color(col.toArgb())
    {
    }

    void set(const eVector3 &pos, const eVector3 &nrm, const eVector2 &uv, const eColor &col)
    {
        position = pos;
        normal = nrm;
        texCoord = uv;
        color = col;
    }

    eFXYZ		position;
    eFXYZ		normal;
    eVector2    texCoord;
    eColor      color;
};

// Vertex used for particle rendering.
struct eParticleVertex
{
    eParticleVertex(const eVector3 &pos, const eVector2 &uv, const eColor &col) :
        position(pos),
        texCoord(uv),
        color(col.toArgb())
    {
    }

    void set(const eVector3 &pos, const eVector2 &uv, const eColor &col)
    {
        position = pos;
        texCoord = uv;
        color = col.toArgb();
    }

    eFXYZ		position;
    eVector2    texCoord;
    eColor      color;
};

// Vertex used for instancing geometry.
struct eInstanceVertex
{
    eMatrix4x4          modelMtx;
    eMatrix4x4          normalMtx;
};

struct eLinkedInstanceVertex
{
    eInstanceVertex     vtx;
    eInt                next;
};

typedef eArray<eInstanceVertex> eInstanceVtxArray;

// Array holding all vertex type sizes.
const eU32 eVERTEX_SIZES[eVTXTYPE_COUNT] =
{
    sizeof(eVertex),
    sizeof(eParticleVertex),
    sizeof(eInstanceVertex)
};

#endif // VERTEX_HPP