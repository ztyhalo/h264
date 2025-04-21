TEMPLATE = app
CONFIG += console C++11
#CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $$PWD/
#DEFINES += BREAKPAD

#QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -std=gnu++11
DEFINES += ARM

for(libname,DEFINES){
    contains(libname,BREAKPAD){
        message("BREAKPAD")
        BREAKPADLIBDIR=/home/z/zty/a9/xianshi/vpulib/
        LIBS += -L$$BREAKPADLIBDIR/lib -lbreakpad_client
        INCLUDEPATH += $$BREAKPADLIBDIR/include/breakpad
        INCLUDEPATH += $$BREAKPADLIBDIR/include/breakpad/client
        INCLUDEPATH += $$BREAKPADLIBDIR/include/breakpad/client/linux/handler
    }
}

COMMLIB_PATH = /home/z/zty/app/commonlib

INCLUDEPATH += $${COMMLIB_PATH}/\
            $${COMMLIB_PATH}/bufmodel\
            $${COMMLIB_PATH}/date\
            $${COMMLIB_PATH}/epoll\
            $${COMMLIB_PATH}/file\
            $${COMMLIB_PATH}/mutex\
            $${COMMLIB_PATH}/prodata\
            $${COMMLIB_PATH}/prodata/sem\
            $${COMMLIB_PATH}/reflect\
            $${COMMLIB_PATH}/sempro\
            $${COMMLIB_PATH}/sigslot\
            $${COMMLIB_PATH}/socket\
            $${COMMLIB_PATH}/tcp\
            $${COMMLIB_PATH}/timer\
            $${COMMLIB_PATH}/udp\
            $${COMMLIB_PATH}/zprint

linux-gnueabi-oe-g++{
    message("oearm")
    DEFINES += ARM
    LIBS += -lpthread -lfslvpuwrap -lg2d
    LIBS += -L/home/z/zty/app/build/arm_poky_4_8-Debug -lcommonlib
}

SOURCES += \
        h264depay/h264depay.cpp \
        main.cpp \
        netlinkstatus/netlinkstatus.cpp \
        rtcp/rtcp.cpp \
        rtp/rtp.cpp \
        rtsp/rtsp.cpp \
        v4l2/v4l2.cpp \
        vpudec/vpudec.cpp


HEADERS += \
    h264depay/h264depay.h \
    netlinkstatus/netlinkstatus.h \
    rtcp/rtcp.h \
    rtp/rtp.h \
    rtsp/rtsp.h \
    v4l2/v4l2.h \
    vpudec/phymem.h \
    vpudec/vpudec.h

