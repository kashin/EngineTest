#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QMainWindow>

class DrawWidget;

class TestWindow : public QMainWindow
{
    Q_OBJECT
public:
    TestWindow();

    virtual void paintEvent(QPaintEvent *);

private slots:
    void onKeyPressed(QKeyEvent* event);

private:
    DrawWidget* mCentralWidget;
};

#endif // TESTWINDOW_H
