#
# QUpgrade - Cross-platform firmware flashing tool
#

QT       += core gui widgets webkit network webkitwidgets serialport

TARGET = qupgrade
TEMPLATE = app

SOURCES += src/apps/qupgrade/main.cpp\
        src/apps/qupgrade/dialog.cpp\
        src/apps/qupgrade/hled.cpp \
        src/apps/qupgrade/uploader.cpp \
        src/apps/qupgrade/compat/qgc.cpp \
        src/apps/qupgrade/qgcfirmwareupgradeworker.cpp \
        src/apps/qupgrade/qupgrademainwindow.cpp

HEADERS  += src/apps/qupgrade/dialog.h \
            src/apps/qupgrade/hled.h \
            src/apps/qupgrade/uploader.h \
            src/apps/qupgrade/compat/QGC.h \
            src/apps/qupgrade/qgcfirmwareupgradeworker.h \
            src/apps/qupgrade/qupgrademainwindow.h

INCLUDEPATH += src/apps/qupgrade \
               src/apps/qupgrade/compat

FORMS    += src/apps/qupgrade/dialog.ui \
            src/apps/qupgrade/qupgrademainwindow.ui

#
# Deployment
#

# Setup our supported build flavors
CONFIG(debug, debug|release) {
    message(Debug flavor)
    CONFIG += DebugBuild
} else:CONFIG(release, debug|release) {
    message(Release flavor)
    CONFIG += ReleaseBuild
} else {
    error(Unsupported build flavor)
}

BASEDIR = $${PWD}
DebugBuild {
	TARGETDIR = $${OUT_PWD}/debug
	BUILDDIR = $${OUT_PWD}/build-debug
}
ReleaseBuild {
	TARGETDIR = $${OUT_PWD}/release
	BUILDDIR = $${OUT_PWD}/build-release
}

# This is deployment black magic that automates the whole Visual Studio deployment process.
# Just run the NSIS script from the deploy folder to create a complete installer.

win32-msvc2010|win32-msvc2012|win32-msvc2013 {
        QMAKE_POST_LINK += $$quote(echo "Copying files"$$escape_expand(\\n))
} else {
        QMAKE_POST_LINK += $$quote(echo "Copying files")
}

# MAC OS X
macx|macx-g++42|macx-g++|macx-llvm {
    QMAKE_POST_LINK += && cp -rf $$BASEDIR/files $$TARGETDIR/qupgrade.app/Contents/MacOS
    ICON = $$BASEDIR/files/logo/qupgrade_logo.icns
}


# Windows (32bit), Visual Studio
win32-msvc2010|win32-msvc2012|win32-msvc2013 {

    win32-msvc2010 {
            message(Building for Windows Visual Studio 2010 (32bit))
    }
    win32-msvc2013 {
            message(Building for Windows Visual Studio 2013 (32bit))
    }

    # Specify multi-process compilation within Visual Studio.
    # (drastically improves compilation times for multi-core computers)
    QMAKE_CXXFLAGS_DEBUG += -MP -openssl-linked
    QMAKE_CXXFLAGS_RELEASE += -MP -openssl-linked

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
    D_DIR = $$[QT_INSTALL_LIBEXECS]
    DLL_DIR = $$replace(D_DIR, "/", "\\")
    
    #OpenSSL from: http://slproweb.com/products/Win32OpenSSL.html
    OPENSSLDIR_WIN = "C:\OpenSSL-Win32"

    # Copy dependencies
    DebugBuild: DLL_QT_DEBUGCHAR = "d"
    ReleaseBuild: DLL_QT_DEBUGCHAR = ""
    COPY_FILE_LIST = \
        $$DLL_DIR\\icu*.dll \
        $$DLL_DIR\\Qt5Core$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Gui$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Multimedia$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5MultimediaWidgets$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Network$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5OpenGL$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Positioning$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5PrintSupport$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Qml$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Quick$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Sensors$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5SerialPort$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5OpenGL$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Sql$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5WebKit$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5WebKitWidgets$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Widgets$${DLL_QT_DEBUGCHAR}.dll \
        $$DLL_DIR\\Qt5Xml$${DLL_QT_DEBUGCHAR}.dll
    for(COPY_FILE, COPY_FILE_LIST) {
        QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"$$COPY_FILE\" \"$$TARGETDIR_WIN\"
    }

    ReleaseBuild {
        # Copy Visual Studio DLLs
        # Note that this is only done for release because the debugging versions of these DLLs cannot be redistributed.
        win32-msvc2010 {
            QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"C:\\Windows\\System32\\msvcp100.dll\"  \"$$TARGETDIR_WIN\"
            QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"C:\\Windows\\System32\\msvcr100.dll\"  \"$$TARGETDIR_WIN\"
        }
        else:win32-msvc2012 {
            QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"C:\\Windows\\System32\\msvcp110.dll\"  \"$$TARGETDIR_WIN\"
            QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"C:\\Windows\\System32\\msvcr110.dll\"  \"$$TARGETDIR_WIN\"
        }
        else:win32-msvc2013 {
            QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"C:\\Windows\\System32\\msvcp120.dll\"  \"$$TARGETDIR_WIN\"
            QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"C:\\Windows\\System32\\msvcr120.dll\"  \"$$TARGETDIR_WIN\"
        }
        else {
            error("Visual studio version not supported, installation cannot be completed.")
        }
    }
	
	message($$QMAKE_POST_LINK)

}

RESOURCES += \
    qupgrade.qrc
