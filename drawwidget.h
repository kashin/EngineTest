#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include <QVector3D>
#include <QTimer>

namespace irr {
    class IrrlichtDevice;
    namespace scene {
        class ISceneManager;
    }
    namespace video {
        class IVideoDriver;
    }
}

class DrawWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrawWidget(QWidget* parent = 0 );
    ~DrawWidget();

    void init();
    virtual void paintEvent( QPaintEvent *event );
    virtual void resizeEvent( QResizeEvent *event );
    virtual QPaintEngine * paintEngine() const;
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

signals:
    void keyPressed(QKeyEvent* event);

private slots:
    void onDrawTimer();

private:
    void moveCamera(const QVector3D& moveVector);
    void moveIrrlichtMouseEvent(QMouseEvent* event, bool keyPressed = true);
    void irrlichtKeyEvent(QKeyEvent* event, bool pressed);

    void createIrrlichtDevice();
    void buildIrrlichtScene();
    void drawIrrlichtScene();

private:
    irr::IrrlichtDevice *mDevice;
    irr::scene::ISceneManager *mScene;
    irr::video::IVideoDriver *mDriver;
    QTimer mDrawTimer;
};

#endif // DRAWWIDGET_H
