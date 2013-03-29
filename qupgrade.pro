#
# QUpgrade - Cross-platform firmware flashing tool
#

QT       += core gui widgets

TARGET = qupgrade
TEMPLATE = app

linux*:CONFIG += qesp_linux_udev

include(libs/qextserialport/src/qextserialport.pri)

SOURCES += src/apps/qupgrade/main.cpp\
        src/apps/qupgrade/dialog.cpp\
        src/apps/qupgrade/hled.cpp \
        src/apps/qupgrade/uploader.cpp \
        src/apps/qupgrade/qgc.cpp \
        src/apps/qupgrade/qgcfirmwareupgradeworker.cpp
    src/apps/qupgrade/qgcfirmwareupgradeworker.cpp

HEADERS  += src/apps/qupgrade/dialog.h \
            src/apps/qupgrade/hled.h \
            src/apps/qupgrade/uploader.h \
            src/apps/qupgrade/qgc.h \
            src/apps/qupgrade/qgcfirmwareupgradeworker.h
    src/apps/qupgrade/qgcfirmwareupgradeworker.h

INCLUDEPATH += src/apps/qupgrade

FORMS    += src/apps/qupgrade/dialog.ui

#
# Deployment
#

BASEDIR = $${IN_PWD}
linux-g++|linux-g++-64{
    debug {
        TARGETDIR = $${OUT_PWD}/debug
        BUILDDIR = $${OUT_PWD}/build-debug
    }
    release {
        TARGETDIR = $${OUT_PWD}/release
        BUILDDIR = $${OUT_PWD}/build-release
    }
} else {
    TARGETDIR = $${OUT_PWD}
    BUILDDIR = $${OUT_PWD}/build
}

# This is deployment black magic that automates the whole Visual Studio deployment process.
# Just run the NSIS script from the deploy folder to create a complete installer.

win32-msvc2008|win32-msvc2010 {
        QMAKE_POST_LINK += $$quote(echo "Copying files"$$escape_expand(\\n))
} else {
        QMAKE_POST_LINK += $$quote(echo "Copying files")
}


# Windows (32bit), Visual Studio
win32-msvc2008|win32-msvc2010 {

        win32-msvc2008 {
                message(Building for Windows Visual Studio 2008 (32bit))
        }
        win32-msvc2010 {
                message(Building for Windows Visual Studio 2010 (32bit))
        }

        # Specify multi-process compilation within Visual Studio.
        # (drastically improves compilation times for multi-core computers)
        QMAKE_CXXFLAGS_DEBUG += -MP
        QMAKE_CXXFLAGS_RELEASE += -MP

        # Specify the inclusion of (U)INT*_(MAX/MIN) macros within Visual Studio
        DEFINES += __STDC_LIMIT_MACROS

        # For release builds remove support for various Qt debugging macros.
        CONFIG(release, debug|release) {
                DEFINES += QT_NO_DEBUG
        }

        # For debug releases we just want the debugging console.
        CONFIG(debug, debug|release) {
                CONFIG += console
        }

        #INCLUDEPATH += $$BASEDIR/libs/lib/msinttypes

        #RC_FILE = $$BASEDIR/qgroundcontrol.rc

        # Copy dependencies
        BASEDIR_WIN = $$replace(BASEDIR,"/","\\")
        TARGETDIR_WIN = $$replace(TARGETDIR,"/","\\")

        CONFIG(debug, debug|release) {
                # Copy Qt DLLs
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\plugins\\platforms\\qminimal.dll" "$$TARGETDIR_WIN\\debug\\plugins\\platforms" /E /I $$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\plugins\\platforms\\qwindows.dll" "$$TARGETDIR_WIN\\debug\\plugins\\platforms" /E /I $$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Cored.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Guid.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Xmld.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5XmlPatternsd.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Widgetsd.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\lib\\libEGLd.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\lib\\libGLESv2d.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\D3DCompiler_43.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\icuin49d.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\icudt49d.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\icuuc49d.dll" "$$TARGETDIR_WIN\\debug"$$escape_expand(\\n))

        }

        CONFIG(release, debug|release) {
                # Copy Qt DLLs
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\plugins\\platforms\\qminimald.dll" "$$TARGETDIR_WIN\\debug\\plugins\\platforms" /E /I $$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\plugins\\platforms\\qwindowsd.dll" "$$TARGETDIR_WIN\\debug\\plugins\\platforms" /E /I $$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Core.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Gui.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Xml.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5XmlPatterns.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\Qt5Widgets.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\lib\\libEGL.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\lib\\libGLESv2.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\D3DCompiler_43.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\icuin49.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\icudt49.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(xcopy /D /Y "C:\\Qt\\Qt5.0.1-VS\\5.0.1\\msvc2010\\bin\\icuuc49.dll" "$$TARGETDIR_WIN\\release"$$escape_expand(\\n))

                QMAKE_POST_LINK += $$quote(del /F "$$TARGETDIR_WIN\\release\\qupgrade.exp"$$escape_expand(\\n))
                QMAKE_POST_LINK += $$quote(del /F "$$TARGETDIR_WIN\\release\\qupgrade.lib"$$escape_expand(\\n))

                # Copy Visual Studio DLLs
                # Note that this is only done for release because the debugging versions of these DLLs cannot be redistributed.
                # I'm not certain of the path for VS2008, so this only works for VS2010.
                win32-msvc2010 {
                        QMAKE_POST_LINK += $$quote(xcopy /D /Y "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 10.0\\VC\\redist\\x86\\Microsoft.VC100.CRT\\*.dll\""  "$$TARGETDIR_WIN\\release\\"$$escape_expand(\\n))
                }
        }
}
