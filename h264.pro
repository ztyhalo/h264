TEMPLATE = app
CONFIG += console C++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

linux-gnueabi-oe-g++{
    message("oearm")
    DEFINES += ARM
    LIBS += -lpthread -lfslvpuwrap -lg2d
}
SOURCES += \
        date/com_date.cpp \
        h264depay/h264depay.cpp \
        main.cpp \
        rtcp/rtcp.cpp \
        rtp/rtp.cpp \
        rtsp/rtsp.cpp \
        socket/socket.cpp \
        tcp/tcp_class.cpp \
        tcp/tcp_client.cpp \
        timer/timers.cpp \
        udp/udp.cpp \
        v4l2/v4l2.cpp \
        vpudec/vpudec.cpp \
        zprint/zprint.cpp

HEADERS += \
    date/com_date.h \
    epoll/e_poll.h \
    h264depay/h264depay.h \
    mutex/mutex_class.h \
    rtcp/rtcp.h \
    rtp/rtp.h \
    rtsp/rtsp.h \
    socket/socket.h \
    tcp/tcp_class.h \
    tcp/tcp_client.h \
    timer/timers.h \
    udp/udp.h \
    v4l2/v4l2.h \
    vpudec/phymem.h \
    vpudec/vpudec.h \
    zprint/version.h \
    zprint/zprint.h
