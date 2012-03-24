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

#ifndef STATE_MANAGER_HPP
#define STATE_MANAGER_HPP

// Manages all render states and tries to
// minimize redundant render state switches.
class eStateManager
{
public:
    // Represents one render state with all its
    // render states and stuff.
    struct RenderStates
    {
        struct VertexBufferInfos
        {
            eIVertexBuffer *    vertexBuffer;
            eVertexType         vertexType;
            eU32                byteOffset;
            eU32                instanceCount;
        };

        eIIndexBuffer *         indexBuffer;
        VertexBufferInfos       vtxBufInfos[2];
        eIVertexShader *        vertexShader;
        eIPixelShader *         pixelShader;
        eITexture *             texture[eGraphicsApiDx9::MAX_TEX_UNITS];
        eITexture *             targets[eGraphicsApiDx9::MAX_TARGETS];
        eCubeMapFace            targetFace[eGraphicsApiDx9::MAX_TARGETS];
        eTextureFilter          texFilters[eGraphicsApiDx9::MAX_TEX_UNITS];
        eTextureAddressMode     texAddrModes[eGraphicsApiDx9::MAX_TEX_UNITS];
        eITexture2d *           depthTarget;

        eViewport               viewport;
        eRect                   scissorRect;

        eBool                   alphaTest;
        eBool                   caps[eMAX_CAP_COUNT];

        eZFunction              zFunc;
        eCullingMode            cullingMode;
        ePolygonMode            polyMode;
        eBlendMode              blendSrc;
        eBlendMode              blendDst;
        eBlendOp                blendOp;
    };

public:
    static void                 setGraphics(eGraphicsApiDx9 *gfx);

    static void                 forceApply();
    static void                 apply(eBool force=eFALSE);
    static void                 reset();
    static void                 push();
    static void                 pop();

    static void                 setCap(eRenderCap cap, eBool enabled);
    static void                 setBlendModes(eBlendMode src, eBlendMode dst, eBlendOp op);
    static void                 setAlphaTest(eBool enabled);
    static void                 setPolygonMode(ePolygonMode mode);
    static void                 setViewport(eU32 x, eU32 y, eU32 width, eU32 height);
    static void                 setCullingMode(eCullingMode cm);
    static void                 setZFunction(eZFunction zFunc);
    static void                 setTextureFilter(eU32 unit, eTextureFilter texFilter);
    static void                 setTextureAddressMode(eU32 unit, eTextureAddressMode address);
    static void                 bindVertexBuffer(eU32 index, eIVertexBuffer *vb, eVertexType vertexType, eU32 byteOffset=0, eU32 instanceCount=0);
    static void                 bindIndexBuffer(eIIndexBuffer *ib);
    static void                 bindPixelShader(eIPixelShader *ps);
    static void                 bindVertexShader(eIVertexShader *vs);
    static void                 bindTexture(eU32 unit, eITexture *tex);
    static void                 bindRenderTarget(eU32 index, eITexture *tex, eCubeMapFace face=eCMFACE_POSX);
    static void                 bindDepthTarget(eITexture2d *tex);
    static void                 setScissorRect(const eRect &rect);

    static eBool                getCap(eRenderCap cap);
    static void                 getBlendModes(eBlendMode &src, eBlendMode &dst, eBlendOp &op);
    static eBool                getAlphaTest();
    static ePolygonMode         getPolygonMode();
    static eViewport            getViewport();
    static eCullingMode         getCullingMode();
    static eZFunction           getZFunction();
    static eTextureFilter       getTextureFilter(eU32 unit);
    static eTextureAddressMode  getTextureAddressMode(eU32 unit);
    static eIVertexBuffer *     getVertexBuffer(eU32 index);
    static eIIndexBuffer *      getIndexBuffer();
    static eIPixelShader *      getPixelShader();
    static eIVertexShader *     getVertexShader();
    static eITexture *          getTexture(eU32 unit);
    static eITexture *          getRenderTarget(eU32 index);
    static eITexture2d *        getDepthTarget();
    static eRect                getScissorRect();

    static RenderStates &       getActiveStates();
    static RenderStates &       getNewStates();
    static RenderStates &       getStackStates(eU32 index);
    static eU32                 getStatesStackSize();

private:
    static const eInt           STACK_SIZE = 16;
    static const eInt           STACK_EMPTY = -1;

private:
    static eGraphicsApiDx9 *      m_gfx;
    static RenderStates         m_activeStates;
    static RenderStates         m_newStates;
    static RenderStates         m_statesStack[STACK_SIZE];
    static eInt                 m_stackPos;
    static eBool                m_changed;
};

#endif // STATE_MANAGER_HPP