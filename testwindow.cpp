#include "testwindow.h"
#include "drawwidget.h"

#include <QDebug>
#include <QKeyEvent>

TestWindow::TestWindow()
    : mCentralWidget(0)
{
    resize( 1024, 768 );
    mCentralWidget = new DrawWidget(this);
    setCentralWidget(mCentralWidget);
    show();
    mCentralWidget->init();

    connect (mCentralWidget, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(onKeyPressed(QKeyEvent*)));
}

void TestWindow::paintEvent(QPaintEvent *paintEvent)
{
    mCentralWidget->paintEvent(paintEvent);
}

void TestWindow::onKeyPressed(QKeyEvent *event)
{
    qDebug() << "keyPressed" << event->key();
    if (event->key() == Qt::Key_Escape)
    {
        close();
    }
}

