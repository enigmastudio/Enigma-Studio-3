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

#include <QtGui/QApplication>
#include <QtGui/QColorDialog>
#include <QtGui/QResizeEvent>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

#include "renderview.hpp"

const eF32 eRenderView::CAMERA_MOUSE_SPEED          = 1.0f;
const eF32 eRenderView::CAMERA_MOVE_SPEED_SLOW      = 1.0f;
const eF32 eRenderView::CAMERA_MOVE_SPEED_FAST      = 10.0f;
const eF32 eRenderView::CAMERA_ZOOM_SPEED_SLOWDOWN  = 50.0f;
const eF32 eRenderView::SELECTED_VERTEX_SIZE        = 0.01f;
const eF32 eRenderView::NORMAL_LENGTH               = 0.05f;

eRenderView::eRenderView(QWidget *parent) : QWidget(parent),
    m_opId(eNOID),
    m_lastCalcMs(0),
    m_opHasChanged(eFALSE),
    m_texture(eNULL),
    m_time(0.0f),
    m_target(eNULL),
    m_engine(eFALSE, eSize(300, 200), winId()),
    m_gfx(m_engine.getGraphicsApi()),
    m_renderer(m_engine.getRenderer()),
    m_solidMesh(eMesh::TYPE_DYNAMIC),
    m_bboxMesh(eMesh::TYPE_DYNAMIC),
    m_normalsMesh(eMesh::TYPE_DYNAMIC),
    m_wireframeMesh(eMesh::TYPE_DYNAMIC),
    m_matMesh(eMesh::TYPE_DYNAMIC),
    m_gridMesh(eMesh::TYPE_STATIC),
    m_kdMesh(eMesh::TYPE_DYNAMIC),
    m_kdMeshInstance(eNULL),
    m_wireframeMeshInstance(eNULL)
{
    eOpStacking::initialize();

    _createInvalidLabel();
    _createActions();
    _resetMeshMode();
    _resetBitmapMode();
    _initMaterialMode();

    m_selVertsMat.setLighted(eFALSE);
    m_selVertsMat.setZFunction(eZFUNC_ALWAYS);
    m_selVertsMat.setPointSize(0.05f);

    _createGridMesh(eVector3::XAXIS, eVector3::ZAXIS, 10, 4, 3.0f);
    m_texAlpha = m_gfx->createChessTexture(64, 64, 32, eColor::BLACK, eColor::WHITE);

    setAttribute(Qt::WA_PaintOnScreen, true);
    m_timerId = startTimer(0);
}

eRenderView::~eRenderView()
{
    killTimer(m_timerId);

    eSAFE_DELETE(m_target);
    eSAFE_DELETE(m_texture);
    eSAFE_DELETE(m_texAlpha);

    eOpStacking::shutdown();
}

void eRenderView::_setupTarget()
{
    if (m_target == eNULL || m_target->getSize() != m_gfx->getWindowSize())
    {
        const eU32 width = m_gfx->getWindowWidth();
        const eU32 height = m_gfx->getWindowHeight();

        eSAFE_DELETE(m_target);
        m_target = m_gfx->createTexture2d(width, height, eTRUE, eFALSE, eFALSE, eFORMAT_ARGB8);
    }
}

void eRenderView::_copyToScreen()
{
    eStateManager::push();
    eStateManager::bindRenderTarget(0, eGraphicsApiDx9::TARGET_SCREEN);
    eStateManager::setTextureAddressMode(0, eTEXADDRMODE_CLAMP);
    eStateManager::setTextureFilter(0, eTEXFILTER_NEAREST);
    m_renderer->renderTexturedQuad(eRect(0, 0, m_target->getWidth(), m_target->getHeight()), m_target->getSize(), m_target);
    eStateManager::pop();
}

void eRenderView::setOperator(eID opId)
{
    if (opId == m_opId)
    {
        return;
    }

    // Set operator ID and indicate that
    // an new operator was set.
    m_opId          = opId;
    m_opHasChanged  = eTRUE;

    // Update widget actions for context-menu.
    for (eInt i=actions().size()-1; i>=0; i--)
    {
        removeAction(actions().at(i));
    }

    m_menu.clear();

    // Add default menu if operator is invalid.
    if (_getShownOperator() == eNULL)
    {
        addActions(m_defMenu.actions());
        m_menu.addActions(m_defMenu.actions());
        return;
    }

    // Add menu for operator category.
    const eString &category = _getShownOperator()->getCategory();

    if (category == "Bitmap")
    {
        addActions(m_bmpMenu.actions());
        m_menu.addActions(m_bmpMenu.actions());
    }
    else if (category == "Mesh")
    {
        addActions(m_meshMenu.actions());
        m_menu.addActions(m_meshMenu.actions());
    }
    else if (category == "Model")
    {
        addActions(m_modelMenu.actions());
        m_menu.addActions(m_modelMenu.actions());
    }
    else
    {
        addActions(m_defMenu.actions());
        m_menu.addActions(m_defMenu.actions());
    }
}

void eRenderView::setTime(eF32 time)
{
    eASSERT(time >= 0.0f);
    m_time = time;
}

eID eRenderView::getOperator() const
{
    return m_opId;
}

eBool eRenderView::getShowGrid() const
{
    return m_showGrid;
}

eBool eRenderView::getWireframe() const
{
    return m_wireframe;
}

eVector3 eRenderView::getCameraPos() const
{
    return m_camEyePos;
}

eVector3 eRenderView::getCameraLookAt() const
{
    return m_camLookAt;
}

eVector3 eRenderView::getCameraUp() const
{
    return m_camUpVec;
}

eF32 eRenderView::getZoomFactor() const
{
    return m_bmpZoom;
}

eBool eRenderView::getTiling() const
{
    return m_tiling;
}

eBool eRenderView::getvisualizeAlpha() const
{
    return m_visualizeAlpha;
}

eU32 eRenderView::getLastCalcMs() const
{
    return m_lastCalcMs;
}

eIRenderer * eRenderView::getRenderer() const
{
    return m_renderer;
}

QSize eRenderView::sizeHint() const
{
    return QSize(120, 500);
}

// Needed because we're painting with Direct3D 
// on a normally by QT controlled widget.
QPaintEngine * eRenderView::paintEngine() const
{
    return eNULL;
}

// Returns weather or not an operator was rendered.
eBool eRenderView::_renderOperator()
{
    eASSERT(m_gfx != eNULL);

    eIOperator *op = _getShownOperator();

    if (op == eNULL)
    {
        m_gfx->clear(eCLEAR_ALLBUFFERS, eColor::GRAY);
        return eFALSE;
    }


    // Process active operator.
    eTimer timer;

    if (op->process(m_renderer, m_time))
    {
        m_lastCalcMs    = timer.getElapsedMs();
        m_opHasChanged  = eTRUE;
    }

    if (op->getValid())
    {
        m_lblInvalid->hide();
    }
    else
    {
        m_lblInvalid->show();
        return eFALSE;
    }

    // Call function depending on operator category.
    const eString &category = op->getCategory();

    if (category == "Bitmap")
    {
        _renderBitmapOp((eIBitmapOp *)op);
    }
    else if (category == "Mesh")
    {
        _renderMeshOp((eIMeshOp *)op);
    }
    else if (category == "Model")
    {
        _renderModelOp((eIModelOp *)op);
    }
    else if (category == "Sequencer")
    {
        _renderSequencerOp((eISequencerOp *)op);
    }
    else if (category == "Effect")
    {
        _renderEffectOp((eIEffectOp *)op);
    }
    else if (op->getType() == "Misc : Demo")
    {
        _renderDemoOp((eIDemoOp *)op);
    }
    else if (op->getType() == "Misc : Material")
    {
        _renderMaterialOp((eIMaterialOp *)op);
    }
    else
    {
        m_gfx->clear(eCLEAR_ALLBUFFERS, eColor::BLACK);
    }

    m_opHasChanged = eFALSE;
    return eTRUE;
}

void eRenderView::_renderMaterialOp(eIMaterialOp *op)
{
    eASSERT(op != eNULL);

    if (m_opHasChanged)
    {
        m_matCube.setMaterial(&op->getResult().material);
        m_matMesh.fromEditMesh(m_matCube);
        m_matMesh.finishLoading(eMesh::TYPE_DYNAMIC);
    }

    const eCamera cam = _createCamera();
    const eLight light = _createDefaultLight(cam);

    eMesh::Instance mi(m_matMesh);
    eSceneData sd;

    sd.addRenderable(&mi);
    sd.addLight(&light);

    _renderScene(eScene(sd), cam, 0.0f);
    _doCameraMovement();
}

void eRenderView::_renderBitmapOp(eIBitmapOp *op)
{
    eASSERT(op != eNULL);

    // Operator has changed, or another operator
    // was selected to be shown in render frame
    // => update texture which holds bitmap.
    const eIBitmapOp::Result &res = op->getResult();

    if (m_opHasChanged)
    {
        eSAFE_DELETE(m_texture);
        m_texture = m_gfx->createTexture2d(res.width, res.height, eFALSE, eFALSE, eFALSE, eFORMAT_ARGB8);
        ePtr data = m_texture->lock();
        eMemCopy(data, res.bitmap, res.size*sizeof(eColor));
        m_texture->unlock();
    }

    eASSERT(m_texture != eNULL);

    m_gfx->clear(eCLEAR_ALLBUFFERS, eColor::GRAY);

    eStateManager::push();
    eStateManager::setTextureFilter(0, eTEXFILTER_NEAREST);
    eStateManager::setBlendModes(eBLEND_SRCALPHA, eBLEND_INVSRCALPHA, eBLENDOP_ADD);
    eStateManager::setCap(eCAP_BLENDING, eTRUE);

    if (m_visualizeAlpha)
    {
        // Draw checkerboard background for alpha visualization.
        for (eU32 x=0; x<m_gfx->getWindowWidth(); x+=m_texAlpha->getWidth())
        {
            for (eU32 y=0; y<m_gfx->getWindowHeight(); y+=m_texAlpha->getHeight())
            {
                const eRect r(x, y, x+m_texAlpha->getWidth(), y+m_texAlpha->getHeight());
                m_renderer->renderTexturedQuad(r, m_gfx->getWindowSize(), m_texAlpha);
            }
        }
    }

    // Calculate display position of texture.
    const ePoint upLeft(m_bmpOffset.x, m_gfx->getWindowHeight()-res.height-m_bmpOffset.y);
    const ePoint downRight(upLeft.x+res.width*m_bmpZoom, upLeft.y+res.height*m_bmpZoom);
    const eRect  bmpRect(upLeft, downRight);

    if (m_tiling)
    {
        // Bitmap tiling enabled, so surround
        // bitmap with 8 instances of it-self.
        for (eInt x=-1; x<=1; x++)
        {
            for (eInt y=-1; y<=1; y++)
            {
                eRect r = bmpRect;

                r.translate(x*bmpRect.getWidth(), y*bmpRect.getHeight());
                m_renderer->renderTexturedQuad(r, m_gfx->getWindowSize(), m_texture);
            }
        }
    }
    else
    {
        // Render the bitmap.
        m_renderer->renderTexturedQuad(bmpRect, m_gfx->getWindowSize(), m_texture);
    }

    eStateManager::pop();
}

void eRenderView::_renderScene(eScene &scene, const eCamera &cam, eF32 time)
{
    eASSERT(time >= 0.0f);

    _setupTarget();
    m_renderer->renderScene(scene, cam, m_target, time);
    _copyToScreen();
}

void eRenderView::_renderMeshOp(eIMeshOp *op)
{
    eASSERT(op != eNULL);

    eEditMesh &em = op->getResult().mesh;

    if (m_opHasChanged)
    {
        // Store triangulated version of mesh for
        // rendering of selected faces.
        m_triMesh = em;
        m_triMesh.triangulate();
        m_solidMesh.fromEditMesh(m_triMesh);
        m_solidMesh.finishLoading(eMesh::TYPE_DYNAMIC);

        _updateWireframeMesh(em, SELECTED_VERTEX_SIZE);
        _updateBoundingBoxMesh(em.getBoundingBox());
        _updateNormalsMesh(em, NORMAL_LENGTH);
    }

    // Render mesh and desired information.
    eSceneData sd;

    eMesh::Instance bboxMi(m_bboxMesh);
    eMesh::Instance normalsMi(m_normalsMesh);
    eMesh::Instance solidMi(m_solidMesh);
    eMesh::Instance gridMi(m_gridMesh);
    eMesh::Instance wireMi(m_wireframeMesh);

    sd.addRenderable(m_wireframe ? &wireMi : &solidMi);

    if (m_showBBoxes)
    {
        sd.addRenderable(&bboxMi);
    }

    if (m_showNormals)
    {
        sd.addRenderable(&normalsMi);
    }

    if (m_showGrid)
    {
        sd.addRenderable(&gridMi);
    }

    const eCamera cam = _createCamera();
    const eLight light = _createDefaultLight(cam);

    sd.addLight(&light);

    op->doEditorInteraction(m_gfx, sd);
    _renderScene(eScene(sd), cam, m_time);
    _doCameraMovement();
}

void eRenderView::_renderEffectOp(eIEffectOp *op)
{
    eASSERT(op != eNULL);

    _setupTarget();
    op->getResult().effect->run(m_time, m_target, m_renderer);
    _copyToScreen();
}

void eRenderView::_renderSequencerOp(eISequencerOp *op)
{
    eASSERT(op != eNULL);

    _setupTarget();
    op->getResult().sequencer.run(m_time, m_target, m_renderer);
    _copyToScreen();
}

void eRenderView::_renderDemoOp(eIDemoOp *op)
{
    eASSERT(op != eNULL);
    op->getResult().demo.render(m_time, m_renderer);
}

void eRenderView::_renderModelOp(eIModelOp *op)
{
    eASSERT(op != eNULL);

    eMesh::Instance mi(m_gridMesh);
    eSceneData sd;

    if(!m_wireframe)
        sd.merge(op->getResult().sceneData);

    const eCamera &cam = _createCamera();

    if(m_showBBoxes)
    {
        _updateKDMesh(op->getResult().sceneData, cam);
        eSAFE_DELETE(this->m_kdMeshInstance);
        this->m_kdMeshInstance = new eMesh::Instance(m_kdMesh);
        sd.addRenderable(m_kdMeshInstance);
    }

    if(m_wireframe) {
        _updateWireframeMeshFromModel(op->getResult().sceneData);
        eSAFE_DELETE(this->m_wireframeMeshInstance);
        this->m_wireframeMeshInstance = new eMesh::Instance(m_wireframeMesh);
        sd.addRenderable(m_wireframeMeshInstance);
    }

    if (m_showGrid)
    {
        sd.addRenderable(&mi);
    }

    const eLight &light = _createDefaultLight(cam);

    if (sd.getLightCount() == 0)
    {
        sd.addLight(&light);
    }

    op->doEditorInteraction(m_gfx, sd);

    _renderScene(eScene(sd), cam, m_time);
    _doCameraMovement();
}

// Creates a label, which is shown, if the
// currently displayed operator is invalid.
void eRenderView::_createInvalidLabel()
{
    m_lblInvalid = new QLabel("<b><font color=white>Operator is invalid!</font></b>", this);
    eASSERT(m_lblInvalid != eNULL);
    m_lblInvalid->setStyleSheet("background-color: #808080");
    m_lblInvalid->setAlignment(Qt::AlignCenter);
    m_lblInvalid->hide();

    QHBoxLayout *hbl = new QHBoxLayout(this);
    eASSERT(hbl != eNULL);
    hbl->setContentsMargins(0, 0, 0, 0);
    hbl->addWidget(m_lblInvalid);
}

void eRenderView::_createActions()
{
    // Create default actions.
    QAction *toggleFsAct = m_defMenu.addAction("Toggle fullscreen", this, SIGNAL(onToggleFullscreenMode()));
    eASSERT(toggleFsAct != eNULL);
    toggleFsAct->setCheckable(true);
    toggleFsAct->setShortcut(QKeySequence("tab"));
    toggleFsAct->setShortcutContext(Qt::WidgetShortcut);

    // Create bitmap actions.
    m_bmpMenu.addAction(toggleFsAct);

    QAction *resetVpAct = m_bmpMenu.addAction("Reset viewport", this, SLOT(_onResetViewport()));
    eASSERT(resetVpAct != eNULL);
    resetVpAct->setShortcut(QKeySequence("r"));
    resetVpAct->setShortcutContext(Qt::WidgetShortcut);

    QAction *act = m_bmpMenu.addAction("Show tiled", this, SLOT(_onBmpTiling()));
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("t"));
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setCheckable(true);

    act = m_bmpMenu.addAction("Visualize alpha", this, SLOT(_onBmpVisualizeAlpha()));
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("a"));
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setCheckable(true);
    act->setChecked(true);

    act = m_bmpMenu.addAction("Zoom in", this, SLOT(_onBmpZoomIn()));
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("+"));
    act->setShortcutContext(Qt::WidgetShortcut);

    act = m_bmpMenu.addAction("Zoom out", this, SLOT(_onBmpZoomOut()));
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("-"));
    act->setShortcutContext(Qt::WidgetShortcut);

    // Create mesh actions.
    m_meshMenu.addAction(toggleFsAct);
    m_meshMenu.addAction(resetVpAct);
    m_meshMenu.addSeparator();

    QAction *showGridAct = m_meshMenu.addAction("Show grid", this, SLOT(_onToggleShowGrid()));
    eASSERT(showGridAct != eNULL);
    showGridAct->setShortcut(QKeySequence("g"));
    showGridAct->setShortcutContext(Qt::WidgetShortcut);
    showGridAct->setCheckable(true);
    showGridAct->setChecked(true);

    act = m_meshMenu.addAction("Render wireframe", this, SLOT(_onToggleWireframe()));
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("f"));
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setCheckable(true);
    m_modelMenu.addAction(act);

    act = m_meshMenu.addAction("Show normals", this, SLOT(_onToggleShowNormals()));
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("n"));
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setCheckable(true);

    QAction *showBboxAct = m_meshMenu.addAction("Show bounding boxes", this, SLOT(_onToggleBoundingBoxes()));
    eASSERT(showBboxAct != eNULL);
    showBboxAct->setShortcut(QKeySequence("b"));
    showBboxAct->setShortcutContext(Qt::WidgetShortcut);
    showBboxAct->setCheckable(true);

    // Create model actions.
    m_modelMenu.addAction(toggleFsAct);
    m_modelMenu.addAction(resetVpAct);
    m_modelMenu.addAction(showBboxAct);
    m_modelMenu.addSeparator();
    m_modelMenu.addAction(showGridAct);

    addActions(m_defMenu.actions());
}

void eRenderView::_resetMeshMode()
{
    m_showGrid      = eTRUE;
    m_showNormals   = eFALSE;
    m_wireframe     = eFALSE;
    m_showBBoxes    = eFALSE;

    m_meshMenu.actions().at(3)->setChecked(true);  // Reset "show grid".
    m_meshMenu.actions().at(4)->setChecked(false); // Reset "render wireframe".
    m_meshMenu.actions().at(5)->setChecked(false); // Reset "show normals".
    m_meshMenu.actions().at(6)->setChecked(false); // Reset "show bounding boxes".

    // Reset the camera.
    eIMeshOp *meshOp = (eIMeshOp *)_getShownOperator();

    if (meshOp)
    {
        const eAABB &bbox = meshOp->getResult().mesh.getBoundingBox();

        m_camLookAt = bbox.getCenter();
        m_camEyePos = bbox.getMaximum()+(bbox.getMaximum()-m_camLookAt).normalized()*5.0f;
        m_camUpVec = eVector3::YAXIS;
    }
    else
    {
        _resetCamera();
    }
}

void eRenderView::_resetModelMode()
{
    m_showGrid = eTRUE;
    m_meshMenu.actions().at(3)->setChecked(true);  // Reset "show grid".

    // Reset the camera.
    eIModelOp *modelOp = (eIModelOp *)_getShownOperator();

    if (modelOp)
    {
        const eAABB &bbox = modelOp->getResult().sceneData.getBoundingBox();

        m_camLookAt = bbox.getCenter();
        m_camEyePos = bbox.getMaximum()+(bbox.getMaximum()-m_camLookAt).normalized()*5.0f;
        m_camUpVec = eVector3::YAXIS;
    }
    else
    {
        _resetCamera();
    }
}

void eRenderView::_resetBitmapMode()
{
    m_tiling         = eFALSE;
    m_bmpOffset      = ePoint(10, 10);
    m_bmpZoom        = 1.0f;
    m_visualizeAlpha = eTRUE;

    m_bmpMenu.actions().at(3)->setChecked(false); // Reset "show tiled".
}

void eRenderView::_initMaterialMode()
{
    eIMeshOp *op = (eIMeshOp *)eOpFactory::get().newInstance("Mesh : Cube");
    eASSERT(op != eNULL);
    op->process(m_renderer, 0.0f);
    m_matCube = op->getResult().mesh;
    eSAFE_DELETE(op);
    m_matCube.triangulate();
}

eCamera eRenderView::_createCamera() const
{
    eCamera cam(45.0f, (eF32)size().width()/(eF32)size().height(), 0.1f, 1000.0f);
    eMatrix4x4 mtx;

    mtx.lookAt(m_camEyePos, m_camLookAt, m_camUpVec);
    cam.setViewMatrix(mtx);

    return cam;
}

eLight eRenderView::_createDefaultLight(const eCamera &cam)
{
    eLight light(eColor::WHITE, eColor(32, 32, 32), eColor::BLACK, 500.0f, eFALSE);

    light.setPosition(eVector3::ORIGIN*cam.getViewMatrix().inverse());
    return light;
}

void eRenderView::_resetCamera()
{
    m_camUpVec  = eVector3::YAXIS;
    m_camLookAt = eVector3::ORIGIN;
    m_camEyePos = eVector3(-10.0f, 10.0f, -10.0f);
}

void eRenderView::_doCameraMovement()
{
    // Calculate adjusting values for frame
    // rate independent movement.
    static eTimer timer;
    static eU32 oldTime = timer.getElapsedMs();
    const eU32 curTime = timer.getElapsedMs();
    const eF32 moveRate = (eF32)(curTime-oldTime)/100.0f+eALMOST_ZERO;
    oldTime = curTime;

    // Exit if no keys are pressed. Above values
    // have to be updated each time, because
    // old time needs to be altered.
    if (m_keysDown.isEmpty())
    {
        return;
    }

    // Calculate view direction and
    // right vector.
    const eF32 speed = (_isKeyDown(Qt::Key_Shift) ? CAMERA_MOVE_SPEED_FAST : CAMERA_MOVE_SPEED_SLOW);

    const eVector3 viewDir = (m_camLookAt-m_camEyePos).normalized()*moveRate*speed;
    const eVector3 right = (viewDir^m_camUpVec).normalized()*moveRate*speed;

    // Check pressed keys and do movement.
    if (_isKeyDown(Qt::Key_W))
    {
        m_camEyePos += viewDir;
        m_camLookAt += viewDir;
    }
    else if (_isKeyDown(Qt::Key_S))
    {
        m_camEyePos -= viewDir;
        m_camLookAt -= viewDir;
    }

    if (_isKeyDown(Qt::Key_A))
    {
        m_camEyePos += right;
        m_camLookAt += right;
    }
    else if (_isKeyDown(Qt::Key_D))
    {
        m_camEyePos -= right;
        m_camLookAt -= right;
    }
}

// Performs the camera rotation, based
// on mouse movement.
void eRenderView::_doCameraRotation(const QPoint &move, eCameraMode mode)
{
    const eVector2 delta(-(eF32)move.x()/180.0f*CAMERA_MOUSE_SPEED, (eF32)move.y()/180.0f*CAMERA_MOUSE_SPEED);

    const eVector3 viewDir = (m_camLookAt-m_camEyePos).normalized();
    const eVector3 right = (viewDir^m_camUpVec).normalized();

    if (mode == CAM_FIRSTPERSON)
    {
        const eMatrix4x4 rotRight(eQuat(right, -delta.y));
        const eMatrix4x4 rotY(eQuat(eVector3::YAXIS, -delta.x));
        const eMatrix4x4 rot = rotRight*rotY;

        m_camUpVec *= rot;

        // Rotate look-at vector around eye position.
        m_camLookAt -= m_camEyePos;
        m_camLookAt *= rot;
        m_camLookAt += m_camEyePos;
    }
    else
    {
        eVector3 newEyePos = m_camEyePos;
        newEyePos.rotate(delta.y*right+delta.x*m_camUpVec);
        m_camLookAt.null();

        const eVector3 newViewDir = (m_camLookAt-newEyePos).normalized();

        // Make sure that view direction and camera's
        // up vector stay linearly independant.
        if (eAbs(newViewDir*m_camUpVec) < 0.99f)
        {
            m_camUpVec = eVector3::YAXIS;
            m_camEyePos = newEyePos;
        }
    }
}

// Returns whether or not the given key
// is currently pressed down.
eBool eRenderView::_isKeyDown(Qt::Key key) const
{
    if (m_keysDown.contains(key))
    {
        return eTRUE;
    }

    return eFALSE;
}

// Returns the category of the currently
// shown operator. If no operator is shown
// an assertion is raised.
eIOperator * eRenderView::_getShownOperator() const
{
    if (m_opId == eNOID)
    {
        return eNULL;
    }

    return eDemoData::findOperator(m_opId);
}

void eRenderView::_createGridMesh(const eVector3 &xBase, const eVector3 &zBase, eU32 segmentCount, eU32 majorSegCount, eF32 spacing)
{
    m_gridMesh.clear();

    // Calculate number of lines.
    const eInt segLines = segmentCount/2*majorSegCount;
    const eF32 size = (eF32)segLines*spacing;
    const eU32 lineCount = segLines*4+6;

    // Setup coordiante system frame.
    const eVector3 xAxis = xBase*size;
    const eVector3 zAxis = zBase*size;
    const eVector3 yAxis = (zBase^xBase)*size;

    // Add lines for coordinate system's origin.
    m_gridMesh.reserveSpace(lineCount, lineCount*2);

    const eMaterial *mat = eMaterial::getWireframe();

    m_gridMesh.addLine( xAxis, eVector3::ORIGIN, eColor::RED, mat);
    m_gridMesh.addLine(-xAxis, eVector3::ORIGIN, eColor::LIGHTGRAY, mat);
    m_gridMesh.addLine( zAxis, eVector3::ORIGIN, eColor::BLUE, mat);
    m_gridMesh.addLine(-zAxis, eVector3::ORIGIN, eColor::LIGHTGRAY, mat);
    m_gridMesh.addLine( yAxis, eVector3::ORIGIN, eColor::GREEN, mat);
    m_gridMesh.addLine(-yAxis, eVector3::ORIGIN, eColor::LIGHTGRAY, mat);

    // Render minor and major spacing lines.
    for (eInt i=-segLines; i<=segLines; i++)
    {
        // Don't draw line in the middle of
        // coordinate system (grid).
        if (i != 0)
        {
            // Minor spacing lines are drawn in dark
            // gray, major spacing lines in light gray.
            const eColor &color = ((i%majorSegCount)%2 == 0 ? eColor::LIGHTGRAY : eColor::GRAY);

            m_gridMesh.addLine(-xAxis+zBase*i*spacing, xAxis+zBase*i*spacing, color, mat);
            m_gridMesh.addLine(-zAxis+xBase*i*spacing, zAxis+xBase*i*spacing, color, mat);
        }
    }

    m_gridMesh.finishLoading(eMesh::TYPE_DYNAMIC);
}

void eRenderView::_updateNormalsMesh(const eEditMesh &mesh, eF32 length)
{
    eASSERT(length > 0.0f);

    const eU32 normalCount = mesh.getFaceCount()+mesh.getVertexCount();

    m_normalsMesh.clear();
    m_normalsMesh.reserveSpace(normalCount, normalCount*2);

    // Add face normals.
    eU32 index = 0;

    for (eU32 i=0; i<mesh.getFaceCount(); i++)
    {
        const eVector3 &normal = mesh.getFace(i)->normal;
        const eVector3 &pos = mesh.getFace(i)->getCenter().position;

        m_normalsMesh.addLine(pos, pos+normal*length, eColor::RED, eMaterial::getWireframe());
    }

    // Add vertex normals.
    for (eU32 i=0; i<mesh.getVertexCount(); i++)
    {
        const eEditMesh::Vertex *vtx = mesh.getVertex(i);
        eASSERT(vtx != eNULL);

        m_normalsMesh.addLine(vtx->position, vtx->position+vtx->normal*length, eColor::GREEN, eMaterial::getWireframe());
    }

    m_normalsMesh.finishLoading(eMesh::TYPE_DYNAMIC);
}

// Renders mesh in wireframe. Selected primitives
// are rendered in orange.
void eRenderView::_updateWireframeMesh(const eEditMesh &mesh, eF32 selVertexSize)
{
    m_wireframeMesh.clear();

    // Add selected triangles of mesh.
    eU32 index = 0;

    for (eU32 i=0; i<m_triMesh.getFaceCount(); i++)
    {
        const eEditMesh::Face *face = m_triMesh.getFace(i);
        eASSERT(face != eNULL);

        if (face->selected)
        {
            eEditMesh::HalfEdge *he = face->he;

            do
            {
                m_wireframeMesh.addVertex(he->origin->position, eColor::ORANGE);
                he = he->next;
            }
            while (he != face->he);

            m_wireframeMesh.addTriangle(index++, index++, index++, eMaterial::getWireframe());
        }
    }

    // Add edges of mesh.
    static const eColor colors[2] =
    {
        eColor::GRAY,
        eColor::ORANGE,
    };

    for (eU32 i=0; i<mesh.getEdgeCount(); i++)
    {
        const eEditMesh::Edge *edge = mesh.getEdge(i);
        eASSERT(edge != eNULL);

        const eVector3 &pos0 = edge->he0->origin->position;
        const eVector3 &pos1 = edge->he1->origin->position;

        m_wireframeMesh.addLine(pos0, pos1, colors[edge->selected], eMaterial::getWireframe());
    }

    // Add selected vertices of mesh.
    for (eU32 i=0; i<mesh.getVertexCount(); i++)
    {
        const eEditMesh::Vertex *vtx = mesh.getVertex(i);
        eASSERT(vtx != eNULL);

        if (vtx->selected)
        {
            m_wireframeMesh.addPoint(vtx->position, eColor::ORANGE, eMaterial::getWireframe());
        }
    }

    m_wireframeMesh.finishLoading(eMesh::TYPE_DYNAMIC);
}



void renderWireFrameSceneData(const eSceneData& sd, eMesh& mesh, eMatrix4x4 mat) {
    for(eU32 s = 0; s < sd.getEntryCount(); s++) {
        const eSceneData::Entry& e = sd.getEntry(s);
        if(e.renderableList != eNULL) 
            renderWireFrameSceneData(*e.renderableList, mesh, mat * e.matrix);
        else {
            if(e.renderableObject->getType() == eIRenderable::TYPE_MESH) {
                const eMesh::Instance& mi = (const eMesh::Instance&)*e.renderableObject;
                const eMesh& m = mi.getMesh();
                for(eU32 p = 0; p < m.getPrimitiveCount(); p++) {
                    const eMesh::Primitive& prim = m.getPrimitive(p);
                    const eVector3 v0 = mat * m.getVertex(prim.indices[0]).position;
                    const eVector3 v1 = mat * m.getVertex(prim.indices[1]).position;
                    const eVector3 v2 = mat * m.getVertex(prim.indices[2]).position;
                    const eColor& col = eColor::GRAY;

                    if(mesh.getVertexCount() + 6 <= 500000) {
                        mesh.addLine(v0, v1, col, eMaterial::getWireframe());
                        mesh.addLine(v1, v2, col, eMaterial::getWireframe());
                        mesh.addLine(v2, v0, col, eMaterial::getWireframe());
                    }
                }
            }
        }
    }
}

// Renders mesh in wireframe. Selected primitives
// are rendered in orange.
void eRenderView::_updateWireframeMeshFromModel(const eSceneData &sd)
{
    m_wireframeMesh.clear();

    renderWireFrameSceneData(sd, m_wireframeMesh, eMatrix4x4());

    m_wireframeMesh.finishLoading(eMesh::TYPE_DYNAMIC);
}


void renderKDNode(eKDTree::Node& node, eMesh& mesh, eU32 depth) {
    if(node.renderableCnt == 0)
        return;
    if(depth > 8)
        return;

    eVector3 corners[8];
    node.m_bbox.getCorners(corners);
    for(eU32 i = 0; i < 8; i++)
        mesh.addVertex(corners[i], eColor::ORANGE);

    const eMaterial *mat = eMaterial::getWireframe();

    eU32 c = eClamp((eS32)64, (eS32)(64 + depth * 8), (eS32)255);
    eColor color(0, c , 0, 32);

    if(node.children[0] != eNULL) {
        renderKDNode(*node.children[0], mesh, depth + 1);
        renderKDNode(*node.children[1], mesh, depth + 1);
        return;
    } else
        color = eColor(c, c , 0);

    mesh.addLine(corners[0], corners[1], color, mat);
    mesh.addLine(corners[1], corners[2], color, mat);
    mesh.addLine(corners[2], corners[3], color, mat);
    mesh.addLine(corners[3], corners[0], color, mat);
    mesh.addLine(corners[4], corners[5], color, mat);
    mesh.addLine(corners[5], corners[6], color, mat);
    mesh.addLine(corners[6], corners[7], color, mat);
    mesh.addLine(corners[7], corners[4], color, mat);
    mesh.addLine(corners[0], corners[4], color, mat);
    mesh.addLine(corners[1], corners[5], color, mat);
    mesh.addLine(corners[2], corners[6], color, mat);
    mesh.addLine(corners[3], corners[7], color, mat);

}

void eRenderView::_updateKDMesh(eSceneData& sceneData, const eCamera &cam)
{
    eRenderJobPtrArray tmp;
    m_kdTree.reconstruct(sceneData);
    m_kdTree.cull(0, cam, tmp);
    eRenderJob::reset();

    m_kdMesh.clear();
    renderKDNode(m_kdTree.m_root, m_kdMesh, 0);
    m_kdMesh.finishLoading(eMesh::TYPE_DYNAMIC);
/*
    // Add selected triangles of mesh.
    eU32 index = 0;

    for (eU32 i=0; i<m_triMesh.getFaceCount(); i++)
    {
        const eEditMesh::Face *face = m_triMesh.getFace(i);
        eASSERT(face != eNULL);

        if (face->selected)
        {
            eEditMesh::HalfEdge *he = face->he;

            do
            {
                m_wireframeMesh.addVertex(he->origin->position, eColor::ORANGE);
                he = he->next;
            }
            while (he != face->he);

            m_wireframeMesh.addTriangle(index++, index++, index++, eMaterial::getWireframe());
        }
    }

    // Add edges of mesh.
    static const eColor colors[2] =
    {
        eColor::GRAY,
        eColor::ORANGE,
    };

    for (eU32 i=0; i<mesh.getEdgeCount(); i++)
    {
        const eEditMesh::Edge *edge = mesh.getEdge(i);
        eASSERT(edge != eNULL);

        const eVector3 &pos0 = edge->he0->origin->position;
        const eVector3 &pos1 = edge->he1->origin->position;

        m_wireframeMesh.addLine(pos0, pos1, colors[edge->selected], eMaterial::getWireframe());
    }

    // Add selected vertices of mesh.
    for (eU32 i=0; i<mesh.getVertexCount(); i++)
    {
        const eEditMesh::Vertex *vtx = mesh.getVertex(i);
        eASSERT(vtx != eNULL);

        if (vtx->selected)
        {
            m_wireframeMesh.addPoint(vtx->position, eColor::ORANGE, eMaterial::getWireframe());
        }
    }

    m_wireframeMesh.finishLoading();
*/
}

void eRenderView::_updateBoundingBoxMesh(const eAABB &bbox)
{
    m_bboxMesh.clear();
    m_bboxMesh.reserveSpace(12, 8);

    eVector3 corners[8];
    bbox.getCorners(corners);

    const eMaterial *mat = eMaterial::getWireframe();

    m_bboxMesh.addLine(corners[0], corners[1], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[1], corners[2], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[2], corners[3], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[3], corners[0], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[4], corners[5], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[5], corners[6], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[6], corners[7], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[7], corners[4], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[0], corners[4], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[1], corners[5], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[2], corners[6], eColor::YELLOW, mat);
    m_bboxMesh.addLine(corners[3], corners[7], eColor::YELLOW, mat);

    m_bboxMesh.finishLoading(eMesh::TYPE_DYNAMIC);
}

void eRenderView::_onBmpTiling()
{
    m_tiling = !m_tiling;
}

void eRenderView::_onBmpZoomIn()
{
    m_bmpZoom *= 2.0f;
}

void eRenderView::_onBmpZoomOut()
{
    m_bmpZoom /= 2.0f;
}

void eRenderView::_onBmpVisualizeAlpha()
{
    m_visualizeAlpha = !m_visualizeAlpha;
}

void eRenderView::_onResetViewport()
{
    if (_getShownOperator() == eNULL)
    {
        return;
    }

    const eString &category = _getShownOperator()->getCategory();

    if (category == "Mesh")
    {
        _resetMeshMode();
    }
    else if (category == "Model")
    {
        _resetModelMode();
    }
    else if (category == "Bitmap")
    {
        _resetBitmapMode();
    }
}

void eRenderView::_onToggleShowGrid()
{
    m_showGrid = !m_showGrid;
}

void eRenderView::_onToggleShowNormals()
{
    m_showNormals = !m_showNormals;
}

void eRenderView::_onToggleBoundingBoxes()
{
    m_showBBoxes = !m_showBBoxes;
}

void eRenderView::_onToggleWireframe()
{
    m_wireframe = !m_wireframe;
}

void eRenderView::contextMenuEvent(QContextMenuEvent *ce)
{
    QWidget::contextMenuEvent(ce);

    // Only show context menu if mouse was not
    // moved too much.
    if ((m_mouseDownPos-ce->pos()).manhattanLength() < 4)
    {
        m_menu.exec(QCursor::pos());
    }
}

void eRenderView::mousePressEvent(QMouseEvent *me)
{
    QWidget::mousePressEvent(me);

    m_lastMousePos = me->pos();
    m_mouseDownPos = me->pos();
}

void eRenderView::mouseMoveEvent(QMouseEvent *me)
{
    QWidget::mouseMoveEvent(me);

    const QPoint move = m_lastMousePos-me->pos();
    m_lastMousePos = me->pos();

    const eIOperator *op = _getShownOperator();
    
    if (op == eNULL)
    {
        return;
    }

    const eString &category = op->getCategory();

    if (category == "Effect" || category == "Model" || category == "Mesh" || op->getType() == "Misc : Material")
    {
        const eBool leftBtn = (me->buttons() & Qt::LeftButton);
        _doCameraRotation(move, (leftBtn ? CAM_FIRSTPERSON : CAM_ROTORIGIN));
    }
    else if (category == "Bitmap")
    {
        m_bmpOffset.x -= move.x();
        m_bmpOffset.y -= move.y();
    }
}

void eRenderView::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget::keyReleaseEvent(ke);

    // Remove key from held keys list.
    const eInt index = m_keysDown.indexOf(ke->key());

    if (index != -1 && !ke->isAutoRepeat())
    {
        m_keysDown.removeAt(index);
    }
}

void eRenderView::keyPressEvent(QKeyEvent *ke)
{
    QWidget::keyPressEvent(ke);

    // Add key to held keys list.
    if (!m_keysDown.contains(ke->key()) && !ke->isAutoRepeat())
    {
        m_keysDown.append(ke->key());
    }
}

void eRenderView::wheelEvent(QWheelEvent *we)
{
    QWidget::wheelEvent(we);

	// Zoom in/out using mouse-wheel.
	eInt wheelDelta = we->delta();

	// In bitmap view:
    eIOperator *op = _getShownOperator();

	if (op && op->getCategory() == "Bitmap")
	{
		if (wheelDelta > 0)
		{
			_onBmpZoomIn();
		}
		else
		{
			_onBmpZoomOut();
		}
	}
	// On other views:
	else
	{
		const eVector3 viewDir = (m_camLookAt-m_camEyePos).normalized()*(eF32)wheelDelta/CAMERA_ZOOM_SPEED_SLOWDOWN;

		m_camEyePos += viewDir;
		m_camLookAt += viewDir;
	}
}

void eRenderView::resizeEvent(QResizeEvent *re)
{
    QWidget::resizeEvent(re);
    m_gfx->resizeBackbuffer(re->size().width(), re->size().height());
}

void eRenderView::timerEvent(QTimerEvent *te)
{
    QWidget::timerEvent(te);
    QApplication::processEvents();
    update();
}

#include <windows.h>

void eRenderView::paintEvent(QPaintEvent *pe)
{
    // Save call count, to avoid recursive paint events.
	static eU32 calls = 0;

    if (calls == 0)
    {
        calls++;

        eProfiler::beginFrame();
        {
            if (m_gfx->renderStart())
            {
                _renderOperator();
		        m_gfx->renderEnd();
            }
        }
        eProfiler::endFrame();

        QWidget::paintEvent(pe);
        calls--;
    }
}

void eRenderView::leaveEvent(QEvent *ev)
{
    QWidget::leaveEvent(ev);

    // Clear pressed keys, because render-frame
    // loses focus => no keyReleaseEvents will
    // be received anymore.
    m_keysDown.clear();
}