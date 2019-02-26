######################################################################
# Automatically generated by qmake (3.0) pon. gru 3 17:34:15 2018
######################################################################

TEMPLATE = app
TARGET = firgui
INCLUDEPATH += . include/
CONFIG += c++14
QMAKE_CXXFLAGS += -H -fopenmp
LIBS += -lssh -fopenmp
DEFINES += QCUSTOMPLOT_USE_OPENGL
QT += widgets core printsupport concurrent
RESOURCES += qdarkstyle/style.qrc \
    data/data.qrc

# Input
HEADERS += \
    window.h \
    groupssh.h \
    ssh.h \
    qcustomplot.h \
    firker.h \
    kerplot.h \
    groupconfig.h \
    groupspecs.h \
    bitstreamspecs.h \
    waitingspinnerwidget.h \
    boolmapwatcher.h \
    style.h \
    switch.h
SOURCES += main.cpp \
    window.cpp \
    groupssh.cpp \
    ssh.cpp \
    qcustomplot.cpp \
    firker.cpp \
    kerplot.cpp \
    groupconfig.cpp \
    groupspecs.cpp \
    bitstreamspecs.cpp \
    waitingspinnerwidget.cpp \
    boolmapwatcher.cpp \
    switch.cpp \
    band.cpp \
    barycentric.cpp \
    cheby.cpp \
    eigenvalue.cpp \
    pm.cpp
