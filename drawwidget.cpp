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

void DrawWidget::createIrrlichtDevice()
{

    qDebug() << "INIT IT!!!";
    if(mDevice != 0)
        return;

    grabKeyboard();
    grabMouse();
    SIrrlichtCreationParameters params;
    params.DriverType = EDT_OPENGL;
    params.WindowId = (void *) winId();
    params.WindowSize = irr::core::dimension2d<irr::u32>( size().width(), size().height() );

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
    IAnimatedMesh* mesh = mScene->getMesh("./20kdm2.bsp");
    ISceneNode* node =
            mScene->addOctreeSceneNode(mesh->getMesh(0), 0, -1, 1024);
    if (node)
    {
        node->setPosition(core::vector3df(-1300,-144,-1249));
    }

    mScene->addCameraSceneNodeFPS();
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

QPaintEngine * DrawWidget::paintEngine() const
{
    return 0;
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

void DrawWidget::init()
{
    createIrrlichtDevice();
}

void DrawWidget::moveCamera(const QVector3D &moveVector)
{
    ICameraSceneNode* camera = mScene->getActiveCamera();
    vector3df position = camera->getPosition();
    position += vector3df(moveVector.x(), moveVector.y(), moveVector.z());
    camera->setPosition(position);
    drawIrrlichtScene();
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

    mDevice->postEventFromUser( irrEvent );

    drawIrrlichtScene();
}

void DrawWidget::onDrawTimer()
{
    qDebug() << QString("FPS=%1").arg(mDriver->getFPS());
    drawIrrlichtScene();
}

void DrawWidget::initializeGL()
{
    makeCurrent();
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

//        ICameraSceneNode *cam = mScene->getActiveCamera();
//        if (cam != 0)
//        {
//            cam->setAspectRatio(size.Width / size.Height);
//        }
    }
}

void DrawWidget::paintGL()
{
    qDebug() << "paintGL";
    if (mDevice == 0)
        return;
    drawIrrlichtScene();
    update();
}

