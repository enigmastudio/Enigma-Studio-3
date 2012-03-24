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

ePROFILER_DEFINE(profFrustumIntersect, "Frustum intersections");

eCamera::eCamera(eF32 fov, eF32 aspect, eF32 near, eF32 far) :
    m_type(TYPE_PERSPECTIVE),
    m_fov(fov),
    m_aspect(aspect),
    m_near(near),
    m_far(far),
    m_left(0.0f),
    m_right(0.0f),
    m_top(0.0f),
    m_bottom(0.0f)
{
    _extractFrustumPlanes();
    m_projMtx.perspective(m_fov, m_aspect, m_near, m_far);
}

eCamera::eCamera(eF32 left, eF32 right, eF32 top, eF32 bottom, eF32 near, eF32 far) :
    m_type(TYPE_ORTHO),
    m_left(left),
    m_right(right),
    m_top(top),
    m_bottom(bottom),
    m_near(near),
    m_far(far),
    m_fov(0.0f),
    m_aspect(0.0f)
{
    m_projMtx.ortho(m_left, m_right, m_top, m_bottom, m_near, m_far);
}

void eCamera::activate(eGraphicsApiDx9 *gfx, const eMatrix4x4 &modelMtx) const
{
    eASSERT(gfx != eNULL);

    const eVector3 camWorldPos = m_invViewMtx.getTranslation();

    gfx->setActiveMatrices(modelMtx, m_viewMtx, m_projMtx);
    gfx->setVsConst(eVSCONST_CAMERA_WORLDPOS, camWorldPos);
}

eBool eCamera::intersectsFrustum(const eVector3 &v, eF32 radius) const
{
    ePROFILER_SCOPE(profFrustumIntersect);
    eASSERT(m_type == TYPE_PERSPECTIVE);

    const eVector3 pt = v*m_viewMtx;

    for (eU32 i=0; i<FRUSTUM_PLANES_COUNT; i++)
    {
        if (m_frustumPlanes[i].getDistance(pt) < -radius)
        {
            return eFALSE;
        }
    }

    return eTRUE;
}

// Checks the AABB against the frustum in world-space.
eBool eCamera::intersectsFrustumCountHits(const eAABB &aabb, eU32& insideHitCounter) const
{
    ePROFILER_SCOPE(profFrustumIntersect);
    eASSERT(m_type == TYPE_PERSPECTIVE);

    insideHitCounter = 0;
    for (eU32 i=0; i<6; i++)
    {
        const ePlane &p = m_frustumPlanes[i];

        const eF32 a = aabb.getCenter()*p.getNormal();
        const eF32 b = aabb.getSize()*p.getAbsNormal();

        if (a+b < -p.getCoeffD())
        {
            return eFALSE;
        }
        if(a-b >= -p.getCoeffD())
            insideHitCounter++;
    }

    return eTRUE;

}


void eCamera::setViewMatrix(const eMatrix4x4 &viewMtx)
{
    m_viewMtx = viewMtx;
    m_invViewMtx = viewMtx.inverse();

    _extractFrustumPlanes();
}

eMatrix4x4 eCamera::getViewMatrix() const
{
    return m_viewMtx;
}

eMatrix4x4 eCamera::getProjectionMatrix() const
{
    return m_projMtx;
}

eCamera::Type eCamera::getType() const
{
    return m_type;
}

eF32 eCamera::getFieldOfView() const
{
    return m_fov;
}

eF32 eCamera::getAspectRatio() const
{
    return m_aspect;
}

eF32 eCamera::getOrthoLeft() const
{
    return m_left;
}

eF32 eCamera::getOrthoRight() const
{
    return m_right;
}

eF32 eCamera::getOrthoTop() const
{
    return m_top;
}

eF32 eCamera::getOrthoBottom() const
{
    return m_bottom;
}

eF32 eCamera::getNear() const
{
    return m_near;
}

eF32 eCamera::getFar() const
{
    return m_far;
}

void eCamera::_extractFrustumPlanes()
{
    eASSERT(m_type == TYPE_PERSPECTIVE);

    // Frustum corners after perspective projection.
    eVector3 corners[8] =
    {
        eVector3(-1.0f, -1.0f,  0.0f), 
        eVector3( 1.0f, -1.0f,  0.0f), 
        eVector3(-1.0f,  1.0f,  0.0f), 
        eVector3( 1.0f,  1.0f,  0.0f), 
        eVector3(-1.0f, -1.0f,  1.0f), 
        eVector3( 1.0f, -1.0f,  1.0f), 
        eVector3(-1.0f,  1.0f,  1.0f), 
        eVector3( 1.0f,  1.0f,  1.0f), 
    };

    // Get corners before perspective projection by
    // multiplying with inverse projection matrix.
    eMatrix4x4 invViewProjMtx = m_viewMtx*m_projMtx;
    invViewProjMtx.invert();

    for (eInt i=0; i<8; i++)
    {
        eVector4 v(corners[i], 1.0f);

        v *= invViewProjMtx;
        v /= v.w;
        corners[i].set(v.x, v.y, v.z);
    }

    // Setup frustum planes.
    m_frustumPlanes[0] = ePlane(corners[0], corners[1], corners[2]);
    m_frustumPlanes[1] = ePlane(corners[6], corners[7], corners[5]);
    m_frustumPlanes[2] = ePlane(corners[2], corners[6], corners[4]);
    m_frustumPlanes[3] = ePlane(corners[7], corners[3], corners[5]);
    m_frustumPlanes[4] = ePlane(corners[2], corners[3], corners[6]);
    m_frustumPlanes[5] = ePlane(corners[1], corners[0], corners[4]);
}