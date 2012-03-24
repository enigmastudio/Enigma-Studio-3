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

#ifndef SCENE_DATA_HPP
#define SCENE_DATA_HPP

class eMesh;
class eSceneData
{
public:
    __declspec(align(16)) struct Entry
    {
        eMatrix4x4              matrix;
        eAABB                   aabb;
        const eIRenderable *    renderableObject;
        eSceneData         *    renderableList;
        eU32                    renderableCount;
    };

public:
    eSceneData();

    void                    merge(eSceneData &sg, const eMatrix4x4 &mtx=eMatrix4x4());
    void                    transform(const eMatrix4x4 &mtx);
    void                    clear();

    void                    addLight(const eLight *light);
    void                    addRenderable(const eIRenderable *ra, const eMatrix4x4 &mtx=eMatrix4x4());

    eU32                    getEntryCount() const;
    const Entry &           getEntry(eU32 index) const;
    const eAABB &           getBoundingBox() const;
    eU32                    getLightCount() const;
    const eLight &          getLight(eU32 index) const;
    eU32                    getRenderableTotal() const;
    void                    convertToMeshOrCount(eU32& verticeCount, eU32& faceCount, eMatrix4x4& mtx = eMatrix4x4(), eMesh* tagetMesh = eNULL) const;

private:

private:
    typedef eArray<Entry> EntryArray;

private:
    eAABB                   m_aabb;
    EntryArray              m_entries;
    eConstLightPtrArray     m_lights;
    eU32                    m_renderableTotal;
};

#endif // SCENE_DATA_HPP