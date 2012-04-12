QT += opengl
CONFIG += link_pkgconfig
PKGCONFIG += libIrrlicht

LIBS += -lIrrlicht

SOURCES += \
    main.cpp \
    testwindow.cpp \
    drawwidget.cpp

HEADERS += \
    testwindow.h \
    drawwidget.h
