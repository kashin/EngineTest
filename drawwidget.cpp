#include "drawwidget.h"

#include <irrlicht/irrlicht.h>
#include <QDebug>
#include <QResizeEvent>

using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;

struct SIrrlichtKey
{
    irr::EKEY_CODE code;
    wchar_t ch;
};

SIrrlichtKey convertToIrrlichtKey( int key )
{
    SIrrlichtKey irrKey;
    irrKey.code = (irr::EKEY_CODE)(0);
    irrKey.ch = (wchar_t)(0);

    // Letters A..Z and numbers 0..9 are mapped directly
    if ( (key >= Qt::Key_A && key <= Qt::Key_Z) || (key >= Qt::Key_0 && key <= Qt::Key_9) )
    {
        irrKey.code = (irr::EKEY_CODE)( key );
        irrKey.ch = (wchar_t)( key );
    }
    else

    // Dang, map keys individually
    switch( key )
    {
    case Qt::Key_Up:
        irrKey.code = irr::KEY_UP;
        break;

    case Qt::Key_Down:
        irrKey.code = irr::KEY_DOWN;
        break;

    case Qt::Key_Left:
        irrKey.code = irr::KEY_LEFT;
        break;

    case Qt::Key_Right:
        irrKey.code = irr::KEY_RIGHT;
        break;
    case Qt::Key_Space:
        irrKey.code = irr::KEY_SPACE;
        break;
    default:
        break;
    }
    return irrKey;
}

DrawWidget::DrawWidget(QWidget *parent)
    : QGLWidget(parent)
{
    setAttribute( Qt::WA_PaintOnScreen, true );
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setMouseTracking( true );
    setFocusPolicy( Qt::ClickFocus );
    setFocus( Qt::OtherFocusReason );
}

DrawWidget::~DrawWidget()
{
    if( mDevice != 0)
    {
        mDevice->closeDevice();
        mDevice->drop();
        mDevice = 0;
    }
}

void DrawWidget::init()
{
    createIrrlichtDevice();
}

void DrawWidget::initializeGL()
{
    init();
}

void DrawWidget::resizeGL(int width, int height)
{
    if (mDevice != 0)
    {
        dimension2d<uint> size;
        size.Width = width;
        size.Height = height;
        mDriver->OnResize( size );
    }
}

void DrawWidget::paintGL()
{
    if (mDevice == 0)
        return;
    drawIrrlichtScene();
    update();
}

void DrawWidget::createIrrlichtDevice()
{

    if(mDevice != 0)
        return;

    SIrrlichtCreationParameters params;
    params.DriverType = EDT_OPENGL;
    params.WindowId = (void *) winId();
    params.WindowSize = irr::core::dimension2d<irr::u32>( size().width(), size().height() );
    params.Stencilbuffer = true;

    mDevice = createDeviceEx(params);
    if( mDevice == 0 )
        qDebug() << "failed to create irrlicht device";

    mDriver = mDevice->getVideoDriver();
    mScene = mDevice->getSceneManager();

    buildIrrlichtScene();
}

void DrawWidget::buildIrrlichtScene()
{
    mDevice->getFileSystem()->addZipFileArchive("./media/map-20kdm2.pk3");
    IAnimatedMesh* mapMesh = mScene->getMesh("./20kdm2.bsp");
    ISceneNode* mapNode =
            mScene->addOctreeSceneNode(mapMesh->getMesh(0), 0, -1, 1024);
    ITriangleSelector* selector = 0;

    if (mapNode)
    {
            mapNode->setPosition(vector3df(-1350,-130,-1400));

            selector = mScene->createOctreeTriangleSelector(
                            mapMesh->getMesh(0), mapNode, 128);
            mapNode->setTriangleSelector(selector);
            mScene->getMeshManipulator()->makePlanarTextureMapping(mapMesh->getMesh(0), 0.004f);
    }

    ICameraSceneNode* camera = mScene->addCameraSceneNodeFPS(0, 100.0f, .3f, 0, 0, 0, true, 3.f);
    if (camera)
    {
        camera->setPosition(vector3df(50,50,-60));
        camera->setTarget(vector3df(-70,30,-60));
    }

    if (selector)
    {
            ISceneNodeAnimator* anim = mScene->createCollisionResponseAnimator(
                    selector, camera, core::vector3df(30,50,30),
                    core::vector3df(0,-10,0), vector3df(0,30,0));
            selector->drop();
            camera->addAnimator(anim);
            anim->drop();
    }
    IAnimatedMeshSceneNode* modelNode = 0;

    modelNode = mScene->addAnimatedMeshSceneNode(mScene->getMesh("./media/faerie.md2"),
                                            0, 3);
    modelNode->setPosition(vector3df(-70,-15,-120));
    modelNode->setScale(vector3df(2, 2, 2));
    modelNode->setMD2Animation(EMAT_RUN);
    modelNode->setAnimationSpeed(20.f);
    SMaterial material;
    material.setTexture(0, mDriver->getTexture("./media/faerie2.bmp"));
    material.Lighting = true;
    material.NormalizeNormals = true;
    modelNode->getMaterial(0) = material;

    selector = mScene->createTriangleSelector(modelNode);
    modelNode->setTriangleSelector(selector);
    selector->drop();

    scene::ISceneNode* lightNode = mScene->addLightSceneNode(0, vector3df(-70,-15,-100),
            video::SColorf(1.0f,0.8f,0.9f,1.0f), 200.0f);
    scene::ISceneNodeAnimator* anim = 0;
    anim = mScene->createFlyCircleAnimator(vector3df(-70,-10,-100),60.0f,0.0015f);
    lightNode->addAnimator(anim);
    anim->drop();

    lightNode = mScene->addBillboardSceneNode(lightNode, dimension2d<f32>(10, 10));
    lightNode->setMaterialFlag(video::EMF_LIGHTING, false);
    lightNode->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    lightNode->setMaterialTexture(0, mDriver->getTexture("./media/particle.bmp"));

    modelNode->addShadowVolumeSceneNode();
    mScene->setShadowColor(SColor(200,0,0,0));
    modelNode->setScale(core::vector3df(2,2,2));
    modelNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);

    mDevice->getCursorControl()->setVisible(false);
}

void DrawWidget::drawIrrlichtScene()
{
    mDevice->getTimer()->tick();
    mDriver->beginScene( true, true);
    mScene->drawAll();
    mDriver->endScene();
    ((QWidget*)parent())->setWindowTitle(QString("FPS=%1").arg(mDriver->getFPS()));
}

void DrawWidget::moveIrrlichtMouseEvent(QMouseEvent* event, bool keyPressed)
{
    irr::SEvent irrEvent;

    irrEvent.EventType = irr::EET_MOUSE_INPUT_EVENT;

    switch ( event->button() )
    {
    case Qt::LeftButton:
        irrEvent.MouseInput.Event = keyPressed ? irr::EMIE_LMOUSE_PRESSED_DOWN:irr::EMIE_LMOUSE_LEFT_UP;
        break;

    case Qt::MidButton:
        irrEvent.MouseInput.Event = keyPressed ? irr::EMIE_MMOUSE_PRESSED_DOWN:irr::EMIE_MMOUSE_LEFT_UP;
        break;

    case Qt::RightButton:
        irrEvent.MouseInput.Event = keyPressed ? irr::EMIE_RMOUSE_PRESSED_DOWN:irr::EMIE_RMOUSE_LEFT_UP;
        break;

    default:
        irrEvent.MouseInput.Event = EMIE_MOUSE_MOVED;
        break;
    }

    irrEvent.MouseInput.X = event->x();
    irrEvent.MouseInput.Y = event->y();
    irrEvent.MouseInput.Wheel = 0.0f; // Zero is better than undefined

    mDevice->postEventFromUser( irrEvent );
    drawIrrlichtScene();
}

void DrawWidget::mouseMoveEvent(QMouseEvent* event)
{
    moveIrrlichtMouseEvent(event, event->button() != Qt::NoButton);
}

void DrawWidget::mousePressEvent(QMouseEvent *event)
{
    moveIrrlichtMouseEvent(event, true);
}

void DrawWidget::mouseReleaseEvent(QMouseEvent *event)
{
    moveIrrlichtMouseEvent(event, false);
}

void DrawWidget::irrlichtKeyEvent(QKeyEvent *event, bool pressed)
{
    irr::SEvent irrEvent;

    irrEvent.EventType = irr::EET_KEY_INPUT_EVENT;

    SIrrlichtKey irrKey = convertToIrrlichtKey( event->key() );

    if ( irrKey.code == 0 ) return; // Could not find a match for this key

    irrEvent.KeyInput.Key = irrKey.code;
    irrEvent.KeyInput.Control = ((event->modifiers() & Qt::ControlModifier) != 0);
    irrEvent.KeyInput.Shift = ((event->modifiers() & Qt::ShiftModifier) != 0);
    irrEvent.KeyInput.Char = irrKey.ch;
    irrEvent.KeyInput.PressedDown = pressed;

    mDevice->postEventFromUser(irrEvent);

    drawIrrlichtScene();
}

void DrawWidget::keyPressEvent(QKeyEvent * event)
{
    irrlichtKeyEvent(event, true);
    emit keyPressed(event);
}

void DrawWidget::keyReleaseEvent(QKeyEvent *event)
{
    irrlichtKeyEvent(event, false);
}
