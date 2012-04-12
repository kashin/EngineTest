#include "testwindow.h"
#include "drawwidget.h"

#include <QDebug>
#include <QKeyEvent>

TestWindow::TestWindow()
    : mCentralWidget(0)
{
    resize(800,600);
    mCentralWidget = new DrawWidget(this);
    setCentralWidget(mCentralWidget);
    show();
    mCentralWidget->init();

    connect (mCentralWidget, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(onKeyPressed(QKeyEvent*)));
}

void TestWindow::onKeyPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        close();
    }
}

