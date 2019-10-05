######################################################################
# Automatically generated by qmake (3.0) pon. gru 3 17:34:15 2018
######################################################################

TEMPLATE = app
TARGET = "FIR Controller"
CONFIG += c++14

DEFINES += WORKDIR_STARTUP_CHECK
#Debugging defines
#linux: DEFINES += COMD
#linux: DEFINES += USERD=\"\\\"franciszek\\\"\"

DEFINES += _USE_MATH_DEFINES
DEFINES += SSH_TIMEOUT=3L
QT += widgets core printsupport concurrent network
RESOURCES += \
    data/data.qrc

INCLUDEPATH += . include/

#if using opengl:
#win32:LIBS += opengl32.lib
#DEFINES += QCUSTOMPLOT_USE_OPENGL

linux{
QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp -lssh
}

win32{
QMAKE_CXXFLAGS += /openmp
RC_ICONS = icon.ico
DEFINES += WIN32_LEAN_AND_MEAN
LIBS += -L$$PWD/win32/lib/ -lssh
INCLUDEPATH += $$PWD/win32/include
DEPENDPATH += $$PWD/win32/include
}

DEFINES += GITHUBREPO=\"\\\"https://github.com/franciszekjuras/firgui\\\"\"
DEFINES += VERSIONSTRING=\"\\\"1.1.9-beta.1\\\"\"
DEFINES += IS_PRERELEASE
win32{
VERSION = 1.1.9.1
QMAKE_TARGET_PRODUCT = "FIR Controller"
QMAKE_TARGET_DESCRIPTION = "Controller for FIR filter based on Red Pitaya board."
}

# Input
HEADERS += \
    boxupdate.h \
    cautoupdatergithub.h \
    window.h \
    groupssh.h \
    qcustomplot.h \
    firker.h \
    kerplot.h \
    groupconfig.h \
    groupspecs.h \
    bitstreamspecs.h \
    waitingspinnerwidget.h \
    boolmapwatcher.h \
    style.h \
    switch.h \
    ssh.h
SOURCES += main.cpp \
    boxupdate.cpp \
    cautoupdatergithub.cpp \
    window.cpp \
    groupssh.cpp \
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
    pm.cpp \
    ssh.cpp
