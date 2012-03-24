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

#ifndef CAMERA_HPP
#define CAMERA_HPP

class eCamera
{
public:
    enum Type
    {
        TYPE_ORTHO,
        TYPE_PERSPECTIVE
    };

public:
    eCamera(eF32 fov, eF32 aspect, eF32 near, eF32 far);
    eCamera(eF32 left, eF32 right, eF32 top, eF32 bottom, eF32 near, eF32 far);

    void                activate(eGraphicsApiDx9 *gfx, const eMatrix4x4 &modelMtx=eMatrix4x4()) const;

    eBool               intersectsFrustum(const eVector3 &v, eF32 radius) const;
    eBool               intersectsFrustumCountHits(const eAABB &aabb, eU32& insideHitCounter) const;

    void                setViewMatrix(const eMatrix4x4 &viewMtx);

    eMatrix4x4          getViewMatrix() const;
    eMatrix4x4          getProjectionMatrix() const;
    Type                getType() const;
    eF32                getFieldOfView() const;
    eF32                getAspectRatio() const;
    eF32                getOrthoLeft() const;
    eF32                getOrthoRight() const;
    eF32                getOrthoTop() const;
    eF32                getOrthoBottom() const;
    eF32                getNear() const;
    eF32                getFar() const;

private:
    void                _extractFrustumPlanes();

private:
    static const eU32   FRUSTUM_PLANES_COUNT = 6;

private:
    Type                m_type;
    eF32                m_near;
    eF32                m_far;
    eMatrix4x4          m_viewMtx;
    eMatrix4x4          m_invViewMtx;

    // Ortho mode parameters.
    eF32                m_left;
    eF32                m_right;
    eF32                m_top;
    eF32                m_bottom;

    // Perspective mode parameters.
    eF32                m_fov;
    eF32                m_aspect;
    eMatrix4x4          m_projMtx;
    ePlane              m_frustumPlanes[FRUSTUM_PLANES_COUNT];
};

#endif // CAMERA_HPP