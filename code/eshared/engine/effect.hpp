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

#ifndef EFFECT_HPP
#define EFFECT_HPP

// Base class for all post-processing effects.
typedef eArray<class eIEffect *> eIEffectPtrArray;

class eIEffect
{
public:
    eIEffect();
    virtual ~eIEffect();

public:
    void                run(eF32 time, eITexture2d *target, eIRenderer *renderer);

    void                addInput(eITexture2d *tex);
    void                addInput(eScene &scene, const eCamera &cam);
    void                addInput(eIEffect *fx);
    void                clearInputs();

public:
    static void         shutdown();

protected:
    struct Target
    {
        eITexture2d *   tex;
        eBool           inUse;
        eU32            wasUsed;
        eIEffect *      createdBy;
    };

    typedef eArray<Target *> TargetPtrArray;
    typedef eArray<Target> TargetArray;

protected:
    virtual Target *    _run(TargetPtrArray &srcs) = 0;
    Target *            _getTarget(const eSize &size);
    Target *            _renderSimple(Target *src, eIPixelShader *ps);

private:
    eIEffect::Target *  _runHierarchy();
    void                _garbageCollectTargets();

protected:
    eF32                m_time;
    eIRenderer *        m_renderer;
    eGraphicsApiDx9 *     m_gfx;
    eIPixelShader *     m_ps;

private:
    eIEffectPtrArray    m_inputFx;
    eIVertexShader *    m_vsQuad;

private:
    static TargetArray  m_targetPool;
};

// Base class for iteratable effects.
class eIIterableEffect : public eIEffect
{
public:
    eIIterableEffect(eU32 iterations);

    void                setIterations(eU32 iterations);
    eU32                getIterations() const;

protected:
    Target *            _renderPingPong(Target *src, Target *dst) const;

protected:
    eU32                m_iterations;
};

// Brings data from the renderer into the effect stack.
class eInputEffect : public eIEffect
{
private:
    struct SceneCacheEntry
    {
        eITexture2d *   tex;
        eBool           inUse;
        const eScene *  scene;
    };

    typedef eArray<SceneCacheEntry> SceneCache;

public:
    eInputEffect(eScene &scene, const eCamera &cam);
    eInputEffect(eITexture2d *tex);
    virtual ~eInputEffect();

    static void         clearCache();

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    Target *            _runForScene();
    Target *            _runForTexture();

    SceneCacheEntry &   _getSceneCacheEntry();

private:
    eScene *            m_scene;
    eCamera             m_cam;
    eITexture2d *       m_tex;

private:
    static SceneCache   m_sceneCache;
};

// Merges multiple effects.
class eMergeEffect : public eIEffect
{
public:
    enum BlendMode
    {
        MODE_ADD,
        MODE_SUB,
        MODE_MUL,
        MODE_BRIGHTER,
        MODE_DARKER,
        MODE_NONE
    };

public:
    eMergeEffect(BlendMode blendMode=MODE_ADD, const eVector2 &blendRatios=eVector2(0.5f, 0.5f));

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    BlendMode           m_blendMode;
    eVector2            m_blendRatios;
};

// Blurs the its input horizontally or vertically.
class eBlurEffect : public eIIterableEffect
{
public:
    enum Direction
    {
        DIR_HORZ,
        DIR_VERT
    };

public:
    eBlurEffect(Direction dir=DIR_HORZ, eU32 distance=1);

    void                setDirection(Direction dir);
    void                setDistance(eU32 dist);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    Direction           m_dir;
    eVector2            m_dist;
};

// Applies a depth-of-field effect using a depth-map.
class eDofEffect : public eIEffect
{
public:
    eDofEffect(eF32 focusDepth=1.0f, eF32 focusRange=10.0f);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eF32                m_focusDepth;
    eF32                m_focusRange;
};

// Generates ambient occlusion in screen space.
class eSsaoEffect : public eIEffect
{
public:
    eSsaoEffect(eF32 scale=0.5f, eF32 intensity=2.5f, eF32 bias=0.25f, eF32 radius=0.5f, eITexture2d *noiseMap=eNULL);

    void                setNoiseMap(eITexture2d *noiseMap);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eF32                m_scale;
    eF32                m_intensity;
    eF32                m_bias;
    eF32                m_radius;
    eITexture2d *       m_noiseMap;
};

// Generates ambient occlusion in screen space.
class eDistortEffect : public eIEffect
{
public:
	eDistortEffect(eVector2 intensity = eVector2(0.0f, 0.0f), eVector2 offset = eVector2(0.0f, 0.0f), eITexture2d *distortMap=eNULL);

	void                setDistortMap(eITexture2d *distortMap);

protected:
	virtual Target *    _run(TargetPtrArray &srcs);

private:
	eVector2            m_intensity;
	eVector2            m_offset;
	eITexture2d *       m_distortMap;
};

// Performs screen space anti-aliasing.
class eFxaaEffect : public eIEffect
{
public:
    eFxaaEffect();

protected:
    virtual Target *    _run(TargetPtrArray &srcs);
};

// Applies a color grading based on a volume texture.
class eColorGradingEffect : public eIEffect
{
public:
    eColorGradingEffect(eITexture3d *lookupMap=eNULL);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eITexture3d *       m_lookupMap;
};

// Adjusts the colors.
class eAdjustEffect : public eIIterableEffect
{
public:
    eAdjustEffect(eF32 brightness=1.0f, eF32 contrast=1.0f, const eColor &adjCol=eColor::WHITE, const eColor &subCol=eColor::BLACK);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eF32                m_brightness;
    eF32                m_contrast;
    eColor              m_adjCol;
    eColor              m_subCol;
};

// Applies a radial blur effect.
class eRadialBlurEffect : public eIEffect
{
public:
    eRadialBlurEffect(const eVector2 &origin=eVector2(0.5f, 0.5f), eF32 distance=1.0f, eF32 strength=2.0f);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eVector2            m_origin;
    eF32                m_distance;
    eF32                m_strength;
};

// Applies a sine wave based ripple effect.
class eRippleEffect : public eIIterableEffect
{
public:
    enum Mode
    {
        MODE_STANDARD,
        MODE_CONCENTRIC
    };

public:
    eRippleEffect(eF32 ampli=0.5f, eF32 length=5.0f, eF32 speed=5.0f, eF32 time=0.0f,
                  const eVector2 &offset=eVector2(0.5f, 0.5f), Mode mode=MODE_STANDARD);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eF32                m_ampli;
    eF32                m_length;
    eF32                m_speed;
    eF32                m_time;
    eVector2            m_offset;
    Mode                m_mode;
};

// Applies fog in screen space (warning: this
// is incompatible with alpha blending).
class eFogEffect : public eIEffect
{
public:
    enum Type
    {
        TYPE_LINEAR,
        TYPE_EXP,
        TYPE_EXPSQR
    };

public:
    eFogEffect(Type type=TYPE_LINEAR, eF32 start=1.0f, eF32 end=10.0f, eF32 density=0.1f, const eColor &color=eColor::WHITE);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    Type                m_type;
    eF32                m_start;
    eF32                m_end;
    eF32                m_density;
    eColor              m_color;
};

// Copies the effect into an external render
// target (used for render-to-texture).
class eSaveEffect : public eIEffect
{
public:
    eSaveEffect(eITexture2d *renderTarget=eNULL, eITexture2d *depthTarget=eNULL);

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eITexture2d *       m_renderTarget;
    eITexture2d *       m_depthTarget;
};

// Performs downsampling on a target.
class eDownsampleEffect : public eIIterableEffect
{
public:
    eDownsampleEffect(const eVector2 &amount=eVector2(0.5f, 0.5f));

protected:
    virtual Target *    _run(TargetPtrArray &srcs);

private:
    eVector2            m_amount;
};

#endif // EFFECT_HPP