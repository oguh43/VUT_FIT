#-------------------------------------------------
#
# Project created by QtCreator 2025-04-29T19:12:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = FSMApp
TEMPLATE = app

CONFIG += c++17

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE -= -O2

QMAKE_CXXFLAGS_RELEASE += -Ofast
QMAKE_CFLAGS_RELEASE += -Ofast

QMAKE_CXXFLAGS += -Wall -Werror -pedantic -Wno-unused-parameter -Wno-missing-field-initializers

mkpath(../build)

OBJECTS_DIR = ../build/obj
MOC_DIR = ../build/moc
UI_DIR = ../build/ui
RCC_DIR = ../build/rcc

DESTDIR = ..

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
# DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/automatoneditor.cpp \
    src/dialogaddstate.cpp \
    src/dialogaddtransition.cpp \
    src/fsm_bridge.cpp \
    src/moore_machine.cpp \
    src/machine_simulator.cpp \
    src/state.cpp \
    src/transition.cpp \
    src/machine_file_handler.cpp \
    src/expression_parser.cpp \
    src/state_ellipse_item.cpp \
    src/dialogaddbooleantransition.cpp \
    src/dialogaddvariable.cpp \
    src/code_generator.cpp \
    src/dialogedittransition.cpp \
    src/includable_generator.cpp \
    src/comm_bridge.cpp \
    src/machine_connector.cpp

HEADERS += \
    headers/mainwindow.h \
    headers/automatoneditor.h \
    headers/dialogaddstate.h \
    headers/dialogaddtransition.h \
    headers/fsm_bridge.h \
    headers/moore_machine.h \
    headers/machine_simulator.h \
    headers/state.h \
    headers/transition.h \
    headers/machine_file_handler.h \
    headers/expression_parser.h \
    headers/machine_variable.h \
    headers/point.h \
    headers/string_utils.h \
    headers/state_ellipse_item.h \
    headers/dialogaddbooleantransition.h \
    headers/dialogaddvariable.h \
    headers/code_generator.h \
    headers/dialogedittransition.h \
    headers/includable_generator.h \
    headers/comm_bridge.h \
    headers/machine_connector.h

FORMS += \
    forms/mainwindow.ui \
    forms/automatoneditor.ui \
    forms/dialogaddstate.ui \
    forms/dialogaddtransition.ui \
    forms/dialogaddbooleantransition.ui \
    forms/dialogaddvariable.ui \
    forms/dialogedittransition.ui
