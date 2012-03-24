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

#ifndef IRENDERABLE_HPP
#define IRENDERABLE_HPP

class eIRenderable
{
public:
    enum Type
    {
        TYPE_TRANSFORM,
        TYPE_MESH,
        TYPE_PARTICLE_SYSTEM,
    };

public:
    virtual ~eIRenderable()
    {
    }

    virtual void            update(eF32 time) = 0;
    virtual void            getRenderJobs(const eMatrix4x4 &matrix, const eMatrix4x4 &normalMtx, eRenderJobPtrArray &jobs, eU32 passID) const = 0;
    virtual const eAABB &   getBoundingBox() const = 0;
    virtual Type            getType() const = 0;
};

#endif // IRENDERABLE_HPP