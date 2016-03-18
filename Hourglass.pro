#-------------------------------------------------
#
# Project created by QtCreator 2011-08-28T15:29:07
#
#-------------------------------------------------

QT       += core gui widgets
QT       += multimedia

TARGET = Hourglass
TEMPLATE = app
ICON = icon.icns

SOURCES += main.cpp\
    Tray.cpp \
    MissingsFrm.cpp \
    CheckFrm.cpp \
    SettingsFrm.cpp \
    IsOnline.h \
    IsOnline.cpp \
    RecordFrm.cpp

HEADERS  += \
    Tray.h \
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

win32{
LIBS += \
    "C:\Program Files\Microsoft SDKs\Windows\v7.0A\Lib\WS2_32.lib" \
    "C:\Program Files\Microsoft SDKs\Windows\v7.0A\Lib\SensAPI.lib"
}
