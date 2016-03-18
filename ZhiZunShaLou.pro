#-------------------------------------------------
#
# Project created by QtCreator 2011-08-28T15:29:07
#
#-------------------------------------------------

QT       += core gui
QT       += phonon

TARGET = ZhiZunShaLou
TEMPLATE = app


SOURCES += main.cpp\
    Tray.cpp \
    norwegianwoodstyle.cpp \
    MissingsFrm.cpp \
    CheckFrm.cpp \
    SettingsFrm.cpp \
    IsOnline.h \
    IsOnline.cpp \
    RecordFrm.cpp

HEADERS  += \
    Tray.h \
    norwegianwoodstyle.h \
    defines.h \
    MissingsFrm.h \
    CheckFrm.h \
    SettingsFrm.h \
    RecordFrm.h

FORMS    += \
    missingsfrm.ui \
    checkfrm.ui \
    settingsfrm.ui \
    RecordFrm.ui

RESOURCES += \
    resource.qrc

RC_FILE += \
    res.rc

LIBS += \
    "C:\Program Files\Microsoft SDKs\Windows\v7.0A\Lib\WS2_32.lib" \
    "C:\Program Files\Microsoft SDKs\Windows\v7.0A\Lib\SensAPI.lib"
