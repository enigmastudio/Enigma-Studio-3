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

#ifndef RENDER_VIEW_HPP
#define RENDER_VIEW_HPP

#include <QtGui/QMenu>

#include "../../eshared/eshared.hpp"

class QLabel;

// Viewport for rendering. All kinds of
// operators are displayed in this widget.
class eRenderView : public QWidget
{
    Q_OBJECT

private:
    enum eCameraMode
    {
        CAM_FIRSTPERSON,
        CAM_ROTORIGIN,
    };

public:
    eRenderView(QWidget *parent);
    virtual ~eRenderView();

    void                    setOperator(eID opId);
    void                    setTime(eF32 time);

    eID                     getOperator() const;
    eBool                   getShowGrid() const;
    eBool                   getWireframe() const;
    eVector3                getCameraPos() const;
    eVector3                getCameraLookAt() const;
    eVector3                getCameraUp() const;
    eF32                    getZoomFactor() const;
    eBool                   getTiling() const;
    eBool                   getvisualizeAlpha() const;
    eU32                    getLastCalcMs() const;
    eIRenderer *            getRenderer() const;

public:
    virtual QSize           sizeHint() const;
    virtual QPaintEngine *  paintEngine() const;

Q_SIGNALS:
    void                    onToggleFullscreenMode();

private:
    void                    _setupTarget();
    void                    _copyToScreen();
    void                    _renderScene(eScene &scene, const eCamera &cam, eF32 time);

    eBool                   _renderOperator();
    void                    _renderMaterialOp(eIMaterialOp *op);
    void                    _renderBitmapOp(eIBitmapOp *op);
    void                    _renderMeshOp(eIMeshOp *op);
    void                    _renderModelOp(eIModelOp *op);
    void                    _renderEffectOp(eIEffectOp *op);
    void                    _renderSequencerOp(eISequencerOp *op);
    void                    _renderDemoOp(eIDemoOp *op);

    void                    _createInvalidLabel();
    void                    _createActions();
    void                    _resetMeshMode();
    void                    _resetModelMode();
    void                    _resetBitmapMode();
    void                    _initMaterialMode();

    eCamera                 _createCamera() const;
    eLight                  _createDefaultLight(const eCamera &cam);
    void                    _resetCamera();
    void                    _doCameraMovement();
    void                    _doCameraRotation(const QPoint &move, eCameraMode mode);

    eBool                   _isKeyDown(Qt::Key key) const;
    eIOperator *            _getShownOperator() const;

    void                    _createGridMesh(const eVector3 &xBase, const eVector3 &zBase, eU32 segmentCount, eU32 majorSegCount, eF32 spacing);
    void                    _updateNormalsMesh(const eEditMesh &mesh, eF32 length);
    void                    _updateWireframeMesh(const eEditMesh &mesh, eF32 selVertexSize);
    void                    _updateWireframeMeshFromModel(const eSceneData &sd);
    void                    _updateBoundingBoxMesh(const eAABB &bbox);
    void                    _updateKDMesh(eSceneData& sceneData, const eCamera &cam);

private Q_SLOTS:
    void                    _onBmpTiling();
    void                    _onBmpZoomIn();
    void                    _onBmpZoomOut();
    void                    _onBmpVisualizeAlpha();

    void                    _onResetViewport();
    void                    _onToggleShowGrid();
    void                    _onToggleShowNormals();
    void                    _onToggleBoundingBoxes();
    void                    _onToggleWireframe();

private:
    virtual void            contextMenuEvent(QContextMenuEvent *ce);
    virtual void            mousePressEvent(QMouseEvent *me);
    virtual void            mouseMoveEvent(QMouseEvent *me);
    virtual void            keyReleaseEvent(QKeyEvent *ke);
    virtual void            keyPressEvent(QKeyEvent *ke);
    virtual void            wheelEvent(QWheelEvent *we);
    virtual void            resizeEvent(QResizeEvent *re);
    virtual void            timerEvent(QTimerEvent *te);
    virtual void            paintEvent(QPaintEvent *pe);
    virtual void            leaveEvent(QEvent *ev);

private:
    static const eF32       CAMERA_MOUSE_SPEED;
    static const eF32       CAMERA_MOVE_SPEED_SLOW;
    static const eF32       CAMERA_MOVE_SPEED_FAST;
    static const eF32       CAMERA_ZOOM_SPEED_SLOWDOWN;

    static const eF32       SELECTED_VERTEX_SIZE;
    static const eF32       NORMAL_LENGTH;

private:
    eEngine                 m_engine;
    eGraphicsApiDx9 *         m_gfx;
    eIRenderer *            m_renderer;
    eITexture2d *           m_target;

    eID                     m_opId;
    eInt                    m_timerId;
    eU32                    m_lastCalcMs;
    eBool                   m_opHasChanged;
    QPoint                  m_lastMousePos;
    QPoint                  m_mouseDownPos;
    eF32                    m_time;

    // Bitmap mode variables.
    eITexture2d *           m_texture;
    eITexture2d *           m_texAlpha;
    eBool                   m_tiling;
    eBool                   m_visualizeAlpha;
    eF32                    m_bmpZoom;
    ePoint                  m_bmpOffset;

    // Mesh mode variables.
    eVector3                m_camUpVec;
    eVector3                m_camEyePos;
    eVector3                m_camLookAt;
    eBool                   m_showNormals;
    eBool                   m_wireframe;
    eMesh                   m_solidMesh;
    eMesh                   m_bboxMesh;
    eMesh                   m_normalsMesh;
    eMesh                   m_wireframeMesh;
    eEditMesh               m_triMesh;

    // Mesh and model mode variables.
    eBool                   m_showGrid;
    eBool                   m_showBBoxes;
    eMesh                   m_gridMesh;
    eMaterial               m_selVertsMat;
    eMesh                   m_kdMesh;
    eKDTree                 m_kdTree;
    eMesh::Instance*        m_kdMeshInstance;
    eMesh::Instance*        m_wireframeMeshInstance;

    // Material mode variables.
    eEditMesh               m_matCube;
    eMesh                   m_matMesh;

    // Stuff used by GUI.
    QList<eInt>             m_keysDown;
    QMenu                   m_menu;
    QMenu                   m_defMenu;
    QMenu                   m_bmpMenu;
    QMenu                   m_meshMenu;
    QMenu                   m_modelMenu;
    QLabel *                m_lblInvalid;
};

#endif // RENDER_VIEW_HPP