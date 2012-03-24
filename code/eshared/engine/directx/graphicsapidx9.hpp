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

#ifndef GRAPHICS_API_DX9_HPP
#define GRAPHICS_API_DX9_HPP

// Render states and other rendering flags and constants.

enum eClearMode
{
    eCLEAR_COLORBUFFER   = 0x01,
    eCLEAR_DEPTHBUFFER   = 0x02,
    eCLEAR_STENCILBUFFER = 0x04,
    eCLEAR_ALLBUFFERS    = eCLEAR_COLORBUFFER | eCLEAR_DEPTHBUFFER | eCLEAR_STENCILBUFFER,
    eCLEAR_DEPTHCOLOR    = eCLEAR_COLORBUFFER | eCLEAR_DEPTHBUFFER
};

enum eCullingMode
{
    eCULLING_NONE,
    eCULLING_FRONT,
    eCULLING_BACK
};

enum eZFunction
{
    eZFUNC_NEVER,
    eZFUNC_LESS,
    eZFUNC_EQUAL,
    eZFUNC_LESSEQUAL,
    eZFUNC_GREATER,
    eZFUNC_NOTEQUAL,
    eZFUNC_GREATEREQUAL,
    eZFUNC_ALWAYS
};

enum eBlendMode
{
    eBLEND_ZERO,
    eBLEND_ONE,
    eBLEND_SRCCOLOR,
    eBLEND_INVSRCCOLOR,
    eBLEND_SRCALPHA,
    eBLEND_INVSRCALPHA,
    eBLEND_DSTALPHA,
    eBLEND_INVDSTALPHA,
    eBLEND_DSTCOLOR,
    eBLEND_INVDSTCOLOR,
};

enum eBlendOp
{
    eBLENDOP_ADD,
    eBLENDOP_SUB,
    eBLENDOP_INVSUB,
    eBLENDOP_MIN,
    eBLENDOP_MAX
};

enum ePolygonMode
{
    ePOLYMODE_FILLED,
    ePOLYMODE_LINES,
    ePOLYMODE_POINTS
};

enum eRenderCap
{
    eCAP_BLENDING,
    eCAP_ZBUFFER,
    eCAP_ZWRITE,
    eCAP_COLORWRITE,
    eCAP_SCISSORTEST,

    eMAX_CAP_COUNT
};

enum eTextureType
{
    eTEXTURE_2D,
    eTEXTURE_3D,
    eTEXTURE_CUBE
};

enum eTextureFilter
{
    eTEXFILTER_NEAREST,
    eTEXFILTER_BILINEAR,
    eTEXFILTER_TRILINEAR,
};

enum eTextureAddressMode
{
    eTEXADDRMODE_WRAP,
    eTEXADDRMODE_CLAMP,
    eTEXADDRMODE_MIRROR,
};

enum eFormat
{
    eFORMAT_ARGB8,
    eFORMAT_ARGB16,
    eFORMAT_ARGB16F,
    eFORMAT_DEPTH16,
    eFORMAT_DEPTH24X8,
    eFORMAT_R16F,
    eFORMAT_R32F,
    eFORMAT_GR16F,
    eFORMAT_GR32F
};

enum ePrimitiveType
{
    ePRIMTYPE_TRIANGLELIST,
    ePRIMTYPE_TRIANGLESTRIPS,
    ePRIMTYPE_LINESTRIPS,
    ePRIMTYPE_LINELIST,
};

enum eMessage
{
    eMSG_BUSY,
    eMSG_IDLE,
    eMSG_QUIT
};

enum eShaderConst
{
    // Vertex shader constants
    eVSCONST_LIGHT_VIEWPOS      = 0,
    eVSCONST_LIGHT_WORLDPOS     = 1,
    eVSCONST_LIGHT_INVRANGE     = 2,

    eVSCONST_CAMERA_WORLDPOS    = 3,

    eVSCONST_VIEW_MATRIX        = 4,
    eVSCONST_PROJ_MATRIX        = 8,
    eVSCONST_MVP_MATRIX         = 12,

    // Pixel shader constants
    ePSCONST_LIGHT_VIEWPOS      = 0,
    ePSCONST_LIGHT_WORLDPOS     = 1,
    ePSCONST_LIGHT_INVRANGE     = 2,
    ePSCONST_LIGHT_PENUMBRA     = 3,
    ePSCONST_LIGHT_SHADOWBIAS   = 4,

    ePSCONST_LIGHT_DIFFUSE      = 5,
    ePSCONST_LIGHT_TOTALAMBIENT = 6,
    ePSCONST_LIGHT_SPECULAR     = 7,

    ePSCONST_MAT_REFRACTION     = 8,
    ePSCONST_MAT_REFRINTENSITY  = 9, 
    ePSCONST_MAT_DIFFUSE        = 10,
    ePSCONST_MAT_SPECULAR       = 11,
    ePSCONST_MAT_SHININESS      = 12,
    ePSCONST_MAT_INDEX          = 13,

    ePSCONST_SHADOW_PROJZ       = 14,
    ePSCONST_SHADOW_MAP_SIZE    = 15,
};

enum eVertexType
{
    eVTXTYPE_DEFAULT,
    eVTXTYPE_PARTICLE,
    eVTXTYPE_INSTANCE,

    eVTXTYPE_COUNT
};

enum eBufferLock
{
    eLOCK_DEFAULT,
    eLOCK_DISCARD,
    eLOCK_NOOVERWRITE,

    eBUFFERLOCK_COUNT
};

enum eCubeMapFace
{
    eCMFACE_POSX,
    eCMFACE_NEGX,
    eCMFACE_POSY,
    eCMFACE_NEGY,
    eCMFACE_POSZ,
    eCMFACE_NEGZ,
    eCMFACE_COUNT
};

// Render API related structures.

struct eViewport
{
    eU32                        x;
    eU32                        y;
    eU32                        width;
    eU32                        height;
};

struct eRenderStats
{
    eU32                        batches;
    eU32                        triangles;
    eU32                        vertices;
    eU32                        lines;
    eF32                        fps;
    eF32                        gpuTimeMs;
};

// Virtual class declarations of different resource types.

class eITexture : public eINonVolatileResource
{
public:
    virtual eBool               unload();
    virtual eBool               bind(eU32 unit, eCubeMapFace face, eBool targetAsTexture=eFALSE) = 0;

    virtual eBool               unlock() = 0;

    virtual eU32                getWidth() const = 0;
    virtual eU32                getHeight() const = 0;
    virtual eFormat             getFormat() const = 0;
    virtual eBool               isDynamic() const = 0;
    virtual eBool               isMipMapped() const = 0;
};

class eITexture2d : public eITexture
{
public:
    virtual ePtr                lock() = 0;

    virtual eSize               getSize() const = 0;
    virtual eBool               isRenderTarget() const = 0;

#ifndef eINTRO
    virtual void                saveToFile(const eChar *fileName) = 0;
#endif
};

class eITexture3d : public eITexture
{
public:
    virtual ePtr                lock() = 0;

    virtual eU32                getDepth() const = 0;
};

class eITextureCube : public eITexture
{
public:
    virtual ePtr                lock(eCubeMapFace face) = 0;

    virtual eSize               getSize() const = 0;
    virtual eBool               isRenderTarget() const = 0;

#ifndef eINTRO
    virtual void                saveToFile(const eChar *fileName) = 0;
#endif
};

class eIIndexBuffer : public eIVolatileResource
{
public:
    virtual eBool               unload();
    virtual eBool               bind() = 0;
    virtual eU32 *              lock(eU32 offset, eU32 count, eBufferLock lockMode) = 0;
    virtual eBool               unlock() = 0;

    virtual eU32                getCount() const = 0;
    virtual eBool               isDynamic() const = 0;
};

class eIVertexBuffer : public eIVolatileResource
{
public:
    virtual eBool               unload();
    virtual eBool               bind(eU32 index, eVertexType vertexType, eU32 byteOffset, eU32 instanceCount) = 0;
    virtual ePtr                lock(eU32 byteOffset, eU32 byteCount, eBufferLock lockMode) = 0;
    virtual eBool               unlock() = 0;

    virtual eU32                getByteSize() const = 0;
    virtual eBool               isDynamic() const = 0;
};

class eIShader : public eIVolatileResource
{
public:
    virtual eBool               bind() = 0;

#ifdef eDEBUG
    virtual eBool               load(const eString &fileName) = 0;
#else
    virtual eBool               load(eConstPtr data) = 0;
#endif
};

class eIVertexShader : public eIShader
{
};

class eIPixelShader : public eIShader
{
};

struct IDirect3DSurface9;
struct IDirect3DVertexDeclaration9;
struct IDirect3DQuery9;
struct IDirect3DDevice9;
struct IDirect3D9;

class eGraphicsApiDx9
{
public:
    enum
    {
        MAX_TEX_UNITS           = 10,
        MAX_TARGETS             = 4
    };

    static eITexture2d *        TARGET_SCREEN;

public:
    eGraphicsApiDx9();
#ifdef eEDITOR
    virtual                 ~eGraphicsApiDx9();
    void                    shutdown();
#endif
    eBool                   initialize();

    eBool                   openWindow(eU32 width, eU32 height, eBool fullScreen=eFALSE, eBool vsync=eTRUE, ePtr hwnd=eNULL);
    void                    closeWindow();
    void                    setWindowTitle(const eString &title);
    void                    handleMessages(eMessage &msg);
    void                    resizeBackbuffer(eU32 width, eU32 height);

    void                    clear(eClearMode mode, const eColor &color) const;
    eBool                   renderStart();
    void                    renderEnd();
    void                    flush();

    eBool                   getInitialized() const;
    eU32                    getResolutionCount() const;
    const eSize &           getResolution(eU32 index) const;

    eRenderStats            getRenderStats() const;
    eBool                   getFullScreen() const;
    eU32                    getWindowWidth() const;
    eU32                    getWindowHeight() const;
    eSize                   getWindowSize() const;

    void                    setCap(eRenderCap cap, eBool enabled);
    void                    setBlendModes(eBlendMode srcMode, eBlendMode dstMode, eBlendOp blendOp);
    void                    setAlphaTest(eBool enable);
    void                    setPolygonMode(ePolygonMode mode);
    void                    setViewport(eU32 x, eU32 y, eU32 width, eU32 height);
    void                    setCullingMode(eCullingMode cm);
    void                    setZFunction(eZFunction zFunc);
    void                    setScissorRect(const eRect &rect);
    void                    setTextureFilter(eU32 unit, eTextureFilter texFilter);
    void                    setTextureAddressMode(eU32 unit, eTextureAddressMode tam);
    void                    setPsConst(eU32 offset, eU32 count, const eF32 *data);
    void                    setVsConst(eU32 offset, eU32 count, const eF32 *data);
    void                    setDepthTarget(eITexture2d *tex);
    void                    setRenderTarget(eU32 index, eITexture *tex, eCubeMapFace face);
    void                    setVertexBuffer(eU32 index, eIVertexBuffer *vb, eVertexType vertexType, eU32 byteOffset, eU32 instanceCount);
    void                    setIndexBuffer(eIIndexBuffer *ib);
    void                    setVertexShader(eIVertexShader *vs);
    void                    setPixelShader(eIPixelShader *ps);
    void                    setTexture(eU32 unit, eITexture *tex);
    void                    setActiveMatrices(const eMatrix4x4 &modelMtx, const eMatrix4x4 &viewMtx, const eMatrix4x4 &projMtx);

    void                    setPsConst(eU32 offset, eF32 f);
    void                    setPsConst(eU32 offset, const eMatrix4x4 &m);
    void                    setPsConst(eU32 offset, const eColor &v);
    void                    setPsConst(eU32 offset, const eVector4 &v);
    void                    setPsConst(eU32 offset, const eVector3 &v);
    void                    setPsConst(eU32 offset, const eVector2 &v);
    void                    setVsConst(eU32 offset, eF32 f);
    void                    setVsConst(eU32 offset, const eMatrix4x4 &m);
    void                    setVsConst(eU32 offset, const eVector4 &v);
    void                    setVsConst(eU32 offset, const eVector3 &v);
    void                    setVsConst(eU32 offset, const eVector2 &v);

    eMatrix4x4              getActiveViewMatrix() const;
    eMatrix4x4              getActiveModelMatrix() const;
    eMatrix4x4              getActiveProjectionMatrix() const;
    void                    getBillboardVectors(eVector3 &right, eVector3 &up, eVector3 *view=eNULL) const;

    void                    drawPrimitives(ePrimitiveType type, eU32 startVertex, eU32 primitiveCount);
    void                    drawIndexedPrimitives(ePrimitiveType type, eU32 startVertex, eU32 vertexCount, eU32 startIndex, eU32 primitiveCount, eU32 instanceCount);

#ifdef eDEBUG
    eIVertexShader *        createVertexShader(const eString &fileName) const;
    eIPixelShader *         createPixelShader(const eString &fileName) const;
#else
    eIVertexShader *        createVertexShader(eConstPtr data) const;
    eIPixelShader *         createPixelShader(eConstPtr data) const;
#endif

    eIVertexBuffer *        createVertexBuffer(eU32 byteSize, eBool dynamic) const;
    eIIndexBuffer *         createIndexBuffer(eU32 indexCount, eBool dynamic) const;
    eITexture2d *           createTexture2d(eU32 width, eU32 height, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format) const;
    eITexture3d *           createTexture3d(eU32 width, eU32 height, eU32 depth, eBool mipMapped, eBool dynamic, eFormat format);
    eITextureCube *         createTextureCube(eU32 width, eBool renderTarget, eBool mipMapped, eBool dynamic, eFormat format) const;
    eITexture2d *           createChessTexture(eU32 width, eU32 height, eU32 step, const eColor &col0, const eColor &col1) const;


#ifndef eINTRO
    eBool                   loadImage(const eByteArray &fileData, eColor *&image, eU32 &width, eU32 &height) const;
#endif

public:
    IDirect3DDevice9 *              getDevice() const;

private:
    eF32                            _getFpsRate() const;
    eBool                           _enumerateResolutions();
    ePtr                            _createWindow(eU32 width, eU32 height, eBool fullScreen);
    void                            _resetDevice(eU32 width, eU32 height);
    void                            _createVertexDecls();
    void                            _releaseVertexDecls();
    void                            _createNullTarget();
    void                            _releaseNullTarget();
    void                            _getScreenTargets();
    void                            _releaseScreenTargets();
    void                            _createQueries();
    void                            _releaseQueries();

private:
    static const eChar              WINDOW_TITLE[];

private:
    eRenderStats                    m_renderStats;


    static const int FRAMES = 2;

    IDirect3DQuery9 *               m_occlQuery[FRAMES];
    eBool                           m_startPull;
    eU32                            m_frame;


    eU32                            m_frameQuery;
    eInt                            m_frameCollect;
    IDirect3DQuery9 *               m_d3dQueriesTsBegin[2];
    IDirect3DQuery9 *               m_d3dQueriesTsEnd[2];
    IDirect3DQuery9 *               m_d3dQueriesTsFreq[2];
    IDirect3DQuery9 *               m_d3dQueriesTsDisjoint[2];

    IDirect3D9 *                    m_d3dMain;
    IDirect3DDevice9 *              m_d3dDevice;
    IDirect3DVertexDeclaration9 *   m_d3dVDecls[eVTXTYPE_COUNT];
    IDirect3DSurface9 *             m_rtNull;
    IDirect3DSurface9 *             m_screenRt;
    IDirect3DSurface9 *             m_screenDt;
    struct _D3DPRESENT_PARAMETERS_* m_d3dPp;
    struct _D3DCAPS9 *              m_d3dCaps;

    eSizeArray                      m_resolutions;

    eBool                           m_deviceLost;
    ePtr                            m_hwnd;
    eBool                           m_ownWindow;
    eBool                           m_fullScreen;
    eBool                           m_vsync;
    eU32                            m_wndWidth;
    eU32                            m_wndHeight;

    eMatrix4x4                      m_activeModelViewMtx;
    eMatrix4x4                      m_activeProjMtx;
    eMatrix4x4                      m_activeModelMtx;
    eMatrix4x4                      m_activeViewMtx;
};

#endif // GRAPHICS_API_DX9_HPP