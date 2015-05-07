#QT += gui-private core-private
QT += gui core

HEADERS += BillboardTest.h \
    vertex.h \
    vertexcol.h \
    vertextex.h \
    Camera.h

SOURCES += BillboardTest.cpp main.cpp \
    vertex.cpp \
    vertexcol.cpp \
    vertextex.cpp \
    Camera.cpp

OTHER_FILES += \
    vshader.txt \
    gshader.txt \
    fshader.txt

RESOURCES += \
    shaders.qrc

