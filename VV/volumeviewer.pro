QT       += core gui widgets

TARGET = VolumeViewer
TEMPLATE = app

SOURCES += main.cpp \
    mainsettings.cpp

SOURCES += \
    mainwidget.cpp \
    geometryengine.cpp

HEADERS += \
    mainwidget.h \
    geometryengine.h \
    mainsettings.h

RESOURCES += \
    shaders.qrc \
    textures.qrc \
    data.qrc \
    data2.qrc \
    data3.qrc \
    data4.qrc \
    data5.qrc \
    data6.qrc \
    data7.qrc \
    data8.qrc \
    data9.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/opengl/cube
INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
