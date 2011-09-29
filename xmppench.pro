#-------------------------------------------------
#
# Project created by QtCreator 2011-09-16T13:17:24
#
#-------------------------------------------------

QT       -= core

QT       -= gui

TARGET = xmppench
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    LatencyWorkloadBenchmark.cpp \
    ActiveSessionPair.cpp \
    IdleSession.cpp \
    StaticDomainNameResolver.cpp \
    BenchmarkNetworkFactories.cpp \
    BoostEventLoop.cpp

LIBS += -L/Users/tobias/dev/rep/swift-trunk/Swiften -L/Users/tobias/dev/rep/swift-trunk/3rdParty/Boost -L/Users/tobias/dev/rep/swift-trunk/3rdParty/LibIDN -lSwiften -lSwiften_Boost -lSwiften_IDN -lxml2 -lz -lssl -lcrypto -lresolv

INCLUDEPATH = /Users/tobias/dev/rep/swift-trunk /Users/tobias/dev/rep/swift-trunk/3rdParty/Boost/src

HEADERS += \
    LatencyWorkloadBenchmark.h \
    AccountDataProvider.h \
    BenchmarkSession.h \
    ActiveSessionPair.h \
    IdleSession.h \
    StaticDomainNameResolver.h \
    BenchmarkNetworkFactories.h \
    BoostEventLoop.h































