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

    // FIXME: implement our own camera to avoid this stupid hack
    if (key == Qt::Key_Space)
    {
        irrKey.code = irr::KEY_KEY_J;
        irrKey.ch = (wchar_t)( key );
        return irrKey;
    }

    // Letters A..Z and numbers 0..9 are mapped directly
    if ( (key >= Qt::Key_A && key <= Qt::Key_Z) || (key >= Qt::Key_0 && key <= Qt::Key_9) )
    {
        irrKey.code = (irr::EKEY_CODE)( key );
        irrKey.ch = (wchar_t)( key );
    }
    else
    {
        // map keys individually
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
    }
    return irrKey;
}

class MoveModelAnimator: public ISceneNodeAnimator
{
public:
    explicit MoveModelAnimator(const vector3df& startPoint,
                               const vector3df& endPoint, u32 timeForWay, const vector3df& rotation)
        : mStartPoint(startPoint),
          mEndPoint(endPoint),
          mAnimatedTime(0),
          mAnimationTime(timeForWay)
    {
        mRotation = rotation;
        vector3df direction = mEndPoint - mStartPoint;
        mRotation.Y = (-atan2(direction.Z, direction.X) * irr::core::RADTODEG);
    }

    virtual void animateNode (ISceneNode *node, u32 timeMs)
    {
        Q_UNUSED(timeMs);

        if ( node->getRotation().Y != mRotation.Y)
        {
            node->setRotation(mRotation);
            return;
        }
        ++mAnimatedTime;
        node->setPosition(node->getPosition() + (mEndPoint-mStartPoint)/(mAnimationTime));
        if (hasFinished())
            node->removeAnimator(this);
    }

    virtual ISceneNodeAnimator* createClone(ISceneNode* node,
                    ISceneManager* newManager=0)
    {
        Q_UNUSED(node);
        Q_UNUSED(newManager);
        return 0;
    }

    virtual bool hasFinished(void) const
    {
        return mAnimatedTime >= mAnimationTime;
    }

private:
    vector3df mStartPoint;
    vector3df mEndPoint;
    vector3df mRotation;
    u32 mAnimatedTime;
    u32 mAnimationTime;
};

DrawWidget::DrawWidget(QWidget *parent)
    : QGLWidget(parent),
      mModelNode(0),
      mMoveModelAnimator(0)
{
    setAttribute( Qt::WA_PaintOnScreen, true );
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setMouseTracking( true );
    setFocusPolicy( Qt::ClickFocus );
    setFocus( Qt::OtherFocusReason );
}

DrawWidget::~DrawWidget()
{
    if (!mMoveModelAnimator)
    {
        mMoveModelAnimator->drop();
    }

    if (!mScene)
    {
        mScene->drop();
        mScene = 0;
    }

    if( !mDevice)
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
                    selector, camera, vector3df(30,50,30),
                    vector3df(0,-10,0), vector3df(0,30,0));
            camera->addAnimator(anim);
            anim->drop();
    }
    mModelNode = mScene->addAnimatedMeshSceneNode(mScene->getMesh("./media/faerie.md2"),
                                                  0, 3);
    if (selector)
    {
        const core::aabbox3d<f32>& box = mModelNode->getBoundingBox();
        core::vector3df radius = box.MaxEdge - box.getCenter();
        ISceneNodeAnimatorCollisionResponse* anim = mScene->createCollisionResponseAnimator(
                    selector, mModelNode, radius,
                    vector3df(0,-10,0), vector3df(0,30,0));
        selector->drop();
//      MoveCollisionResponse* response = new MoveCollisionResponse(this);
//      anim->setCollisionCallback(response);
//      response->drop();
        mModelNode->addAnimator(anim);
        anim->drop();
    }
    mModelNode->setPosition(vector3df(-70,-15,-120));
    mModelNode->setScale(vector3df(2, 2, 2));
    mModelNode->setMD2Animation(EMAT_STAND);
    mModelNode->setAnimationSpeed(20.f);
    mModelNode->setRotation(vector3df(0.5f,1.0f,1.0));
    SMaterial material;
    material.setTexture(0, mDriver->getTexture("./media/faerie2.bmp"));
    material.Lighting = true;
    material.NormalizeNormals = true;
    mModelNode->getMaterial(0) = material;

    selector = mScene->createOctreeTriangleSelector(mModelNode->getMesh(),mModelNode, 64);
    mModelNode->setTriangleSelector(selector);
    selector->drop();

    ISceneNode* lightNode = mScene->addLightSceneNode(0, vector3df(-70,-15,-100),
            video::SColorf(1.0f,0.8f,0.9f,1.0f), 150.0f);
    ISceneNodeAnimator* anim = 0;
    anim = mScene->createFlyCircleAnimator(vector3df(-70,-10,-100),60.0f,0.0015f);
    lightNode->addAnimator(anim);
    anim->drop();

    lightNode = mScene->addBillboardSceneNode(lightNode, dimension2d<f32>(10, 10));
    lightNode->setMaterialFlag(EMF_LIGHTING, false);
    lightNode->setMaterialType(EMT_TRANSPARENT_ADD_COLOR);
    lightNode->setMaterialTexture(0, mDriver->getTexture("./media/particle.bmp"));

    mModelNode->addShadowVolumeSceneNode();
    mScene->setShadowColor(SColor(200,0,0,0));
    mModelNode->setScale(vector3df(2,2,2));
    mModelNode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);

    mCursor = mScene->addBillboardSceneNode();
    mCursor->setMaterialType(EMT_TRANSPARENT_ADD_COLOR );
    mCursor->setMaterialTexture(0, mDriver->getTexture("./media/particle.bmp"));
    mCursor->setMaterialFlag(EMF_LIGHTING, false);
    mCursor->setMaterialFlag(EMF_ZBUFFER, false);
    mCursor->setSize(dimension2d<f32>(20.0f, 20.0f));
    mCursor->setID(0);

    mDevice->getCursorControl()->setVisible(false);
}

void DrawWidget::drawIrrlichtScene()
{
    mDevice->getTimer()->tick();
    checkMoveAnimation();
    mDriver->beginScene( true, true);
    mScene->drawAll();
    mCursor->setPosition(getCursoreIntersation());
    mDriver->endScene();
    ((QWidget*)parent())->setWindowTitle(QString("FPS=%1").arg(mDriver->getFPS()));
}

void DrawWidget::irrlichtMouseEvent(QMouseEvent* event, bool keyPressed)
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

    if (keyPressed && mModelNode) //add move model animation
    {
        animatedMoveModelToPosition(getCursoreIntersation());
    }

    drawIrrlichtScene();
}

void DrawWidget::mouseMoveEvent(QMouseEvent* event)
{
    irrlichtMouseEvent(event, event->button() != Qt::NoButton);
}

void DrawWidget::mousePressEvent(QMouseEvent *event)
{
    irrlichtMouseEvent(event, true);
}

void DrawWidget::mouseReleaseEvent(QMouseEvent *event)
{
    irrlichtMouseEvent(event, false);
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

void DrawWidget::animatedMoveModelToPosition(irr::core::vector3df transition, irr::scene::ISceneNode *modelNode)
{
    if (modelNode != 0) //FIXME: add something useful here.
        return;

    f32 distanceToNewPoint = transition.getDistanceFrom(mModelNode->getPosition());
    u32 timeForAnimation = u32(distanceToNewPoint);
    stopMoveAnimation();
    mMoveModelAnimator = new MoveModelAnimator(mModelNode->getPosition(), transition, timeForAnimation, mModelNode->getRotation());
    mModelNode->addAnimator(mMoveModelAnimator);
    mModelNode->setMD2Animation(EMAT_RUN);
}

irr::core::vector3df DrawWidget::getCursoreIntersation()
{
    line3d<f32> ray;
    ICameraSceneNode* camera = mScene->getActiveCamera();
    ray.start = camera->getPosition();
    ray.end = ray.start + (camera->getTarget() - ray.start).normalize() * 1000.0f;

    // Tracks the current intersection point with the level or a mesh
    vector3df intersection;
    // Used to show with triangle has been hit
    triangle3df hitTriangle;

    ISceneNode *selectedSceneNode =
            mScene->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
                ray,
                intersection, // This will be the position of the collision
                hitTriangle); // This will be the triangle hit in the collision


    // If the ray hit anything, move the billboard to the collision position.
    if(selectedSceneNode)
    {
        return intersection; //mCursor->setPosition(intersection);
    }
    return vector3df();// FIXME: return an invalid value, but what value?
}

void DrawWidget::checkMoveAnimation()
{
    if (mMoveModelAnimator && mMoveModelAnimator->hasFinished())
    {
        stopMoveAnimation();
    }
}

void DrawWidget::onCollisionDetected()
{
    stopMoveAnimation();
}

void DrawWidget::stopMoveAnimation()
{
    if (mMoveModelAnimator && mModelNode)
    {
        mModelNode->removeAnimator(mMoveModelAnimator);
        mMoveModelAnimator->drop();
        mMoveModelAnimator = 0;
        mModelNode->setMD2Animation(EMAT_STAND);
    }
}
