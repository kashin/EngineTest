#include "drawwidget.h"

#include <irrlicht/irrlicht.h>
#include <QDebug>
#include <QResizeEvent>

using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;

DrawWidget::DrawWidget(QWidget *parent)
    : QWidget(parent)
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
    }
}

void DrawWidget::createIrrlichtDevice()
{

    if(mDevice != 0)
        return;

    SIrrlichtCreationParameters createParams;
    createParams.WindowId = (void *) winId();

    mDevice = createDeviceEx(createParams);
    if( mDevice == 0 )
        qDebug() << "failed to create irrlicht device";

    mDriver = mDevice->getVideoDriver();
    mScene = mDevice->getSceneManager();

    buildIrrlichtScene();
}

void DrawWidget::buildIrrlichtScene()
{
    mScene->addMeshSceneNode( mScene->getMesh( "./sydney.md2" ));
    ILightSceneNode *light = mScene->addLightSceneNode();
    light->setLightType( ELT_DIRECTIONAL );
    light->setRotation( vector3df( 25.0f, 45.0f, 0.0f ));
    light->getLightData().AmbientColor = SColorf( 0.2f, 0.2f, 0.2f, 1.0f );
    light->getLightData().DiffuseColor = SColorf( 0.8f, 0.8f, 0.8f, 1.0f );

    mScene->addCameraSceneNode( 0, vector3df(50,50,-40), vector3df(0,5,0) );
}

void DrawWidget::drawIrrlichtScene()
{
    mDriver->beginScene( true, false, SColor( 255, 128, 128, 128 ));
    mScene->drawAll();
    mDriver->endScene();
}

void DrawWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    drawIrrlichtScene();
}

void DrawWidget::resizeEvent(QResizeEvent *event)
{
    if (mDevice != 0)
    {
        dimension2d<uint> size;
        size.Width = event->size().width();
        size.Height = event->size().height();
        mDriver->OnResize( size );

        ICameraSceneNode *cam = mScene->getActiveCamera();
        if (cam != 0)
        {
            cam->setAspectRatio(size.Width / size.Height);
        }
    }
}

QPaintEngine * DrawWidget::paintEngine() const
{
    return 0;
}

void DrawWidget::keyPressEvent(QKeyEvent * event)
{
    emit keyPressed(event);
}

void DrawWidget::init()
{
    createIrrlichtDevice();
}
