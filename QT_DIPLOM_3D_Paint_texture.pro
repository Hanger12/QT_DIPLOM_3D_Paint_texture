QT       += core gui opengl openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
LIBS+=-lopengl32 -lglu32
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutprogramwidjet.cpp \
    axis.cpp \
    buttons/brushcolorbutton.cpp \
    buttons/brushsizebutton.cpp \
    buttons/labeledtoolbutton.cpp \
    glfunctions.cpp \
    glmesh.cpp \
    glwidget.cpp \
    grid.cpp \
    guadandtriangle.cpp \
    main.cpp \
    mainwindow.cpp \
    material.cpp \
    objloader.cpp \
    sessionsettings.cpp \
    texture.cpp \
    utility.cpp

HEADERS += \
    aboutprogramwidjet.h \
    axis.h \
    buttons/brushcolorbutton.h \
    buttons/brushsizebutton.h \
    buttons/labeledtoolbutton.h \
    glfunctions.h \
    glmesh.h \
    glwidget.h \
    grid.h \
    guadandtriangle.h \
    mainwindow.h \
    material.h \
    objloader.h \
    sessionsettings.h \
    stb/stb_image.h \
    stb/stb_image_resize.h \
    stb/stb_image_write.h \
    texture.h \
    utility.h

FORMS += \
    aboutprogramwidjet.ui \
    mainwindow.ui

TRANSLATIONS += \
    QT_DIPLOM_3D_Paint_texture_ru_RU.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

DISTFILES += \
    Render.frag \
    Render.vert \
    UVview.frag \
    UVview.vert \
    axis.frag \
    axis.vert \
    brush1.png \
    cube.jpeg \
    default1.frag \
    default1.vert \
    grid.frag \
    grid.vert \
    paint.frag \
    paint.vert \
    texture.frag \
    texture.vert
