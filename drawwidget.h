#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>

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

signals:
    void keyPressed(QKeyEvent* event);

private:
    void createIrrlichtDevice();
    void buildIrrlichtScene();
    void drawIrrlichtScene();

private:
    irr::IrrlichtDevice *mDevice;
    irr::scene::ISceneManager *mScene;
    irr::video::IVideoDriver *mDriver;
};

#endif // DRAWWIDGET_H
