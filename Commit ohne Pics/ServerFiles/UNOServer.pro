QT += core network
CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = UNOServer

SOURCES += \
    main.cpp \
    server.cpp

HEADERS += \
    server.h
