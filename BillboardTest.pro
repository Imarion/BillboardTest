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
    fshader_tree.txt \
    gshader_tree.txt \
    vshader_tree.txt \
    fshader_grass.txt \
    vshader_grass.txt

RESOURCES += \
    shaders.qrc

