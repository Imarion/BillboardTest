#include "BillboardTest.h"

#include <QDebug>
#include <QFile>
#include <QImage>

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>

#include <cmath>

#include "vertex.h"

#define NUM_VERTICES    36
#define NUM_INDICES     36
//#define NUM_VERTICES    4
//#define NUM_INDICES     6


MyWindow::~MyWindow()
{
    if (TreeVertices != 0)  delete[] TreeVertices;
    if (TreeIndices  != 0)  delete[] TreeIndices;
    if (mTreeProgram != 0)  delete   mTreeProgram;
    if (GrassVertices != 0) delete[] GrassVertices;
    if (GrassIndices  != 0) delete[] GrassIndices;
    if (mGrassProgram != 0) delete   mGrassProgram;
}

MyWindow::MyWindow() : currentTimeMs(0), currentTimeS(0)
{
    TreeVertices = 0;
    TreeIndices  = 0;
    mTreeProgram = 0;
    GrassVertices = 0;
    GrassIndices  = 0;
    mGrassProgram = 0;

    setSurfaceType(QWindow::OpenGLSurface);
    setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
    create();

    resize(800, 600);

    mContext = new QOpenGLContext(this);
    mContext->setFormat(format);
    mContext->create();

    mContext->makeCurrent( this );

    mFuncs = mContext->versionFunctions<QOpenGLFunctions_3_3_Core>();
    if ( !mFuncs )
    {
        qWarning( "Could not obtain OpenGL versions object" );
        exit( 1 );
    }
    if (mFuncs->initializeOpenGLFunctions() == GL_FALSE)
    {
        qWarning( "Could not initialize core open GL functions" );
        exit( 1 );
    }

    initializeOpenGLFunctions();

    QTimer *repaintTimer = new QTimer(this);
    connect(repaintTimer, &QTimer::timeout, this, &MyWindow::render);
    repaintTimer->start(1000/60);

    QTimer *elapsedTimer = new QTimer(this);
    connect(elapsedTimer, &QTimer::timeout, this, &MyWindow::modCurTime);
    elapsedTimer->start(1);       
}

void MyWindow::modCurTime()
{
    currentTimeMs++;
    currentTimeS=currentTimeMs/1000.0f;
}

void MyWindow::initialize()
{
    mFuncs->glGenVertexArrays(1, &mVAO);
    mFuncs->glBindVertexArray(mVAO);

    CreateVertexBuffer();
    initShaders();

    gTreeCameraLocation  = mTreeProgram->uniformLocation("gCameraPos");
    gTreeVPLocation      = mTreeProgram->uniformLocation("gVP");
    gTreeSamplerLocation = mTreeProgram->uniformLocation("gColorMap");

    gGrassCameraLocation  = mGrassProgram->uniformLocation("gCameraPos");
    gGrassVPLocation      = mGrassProgram->uniformLocation("gVP");
    gGrassSamplerLocation = mGrassProgram->uniformLocation("gColorMap");

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    PrepareTexture(GL_TEXTURE_2D, "./data/hackberry_tree_20131230_1040936985.png", mTreeTextureObject, true);
    PrepareTexture(GL_TEXTURE_2D, "./data/dirt_grass_20120516_1324302946.jpg",     mGrassTextureObject, true);

    cam.setPosition(QVector3D(0.0f, 0.1f, 4.0f));
    cam.setFieldOfView(60.0f);
    cam.lookAt(QVector3D(0.0f, 0.1f, 0.0f));
    cam.setViewportAspectRatio((float)this->width()/(float)this->height());
}

void MyWindow::CreateVertexBuffer()
{
    // C++11 required
    TreeVertices = new Vertex {
        Vertex(QVector3D(0.0f,  0.0f, 0.0f),  QVector3D(1.0f, 1.0f, 1.0f))
    };

    GrassVertices = new VertexTex[4] {
        VertexTex(QVector3D(-100.0f,  0.0f,  100.0f),  QVector2D(0.0f, 0.0f)),
        VertexTex(QVector3D( 100.0f,  0.0f,  100.0f),  QVector2D(100.0f, 0.0f)),
        VertexTex(QVector3D( 100.0f,  0.0f, -100.0f),  QVector2D(100.0f, 100.0f)),
        VertexTex(QVector3D(-100.0f,  0.0f, -100.0f),  QVector2D(0.0f, 100.0f))
    };

    GrassIndices = new unsigned int[6] {
         0, 2, 3,
         0, 1, 2
    };

    glGenBuffers(1, &mTreeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mTreeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TreeVertices[0]), TreeVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &mTreeIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTreeIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TreeIndices[0]), TreeIndices, GL_STATIC_DRAW);

    glGenBuffers(1, &mGrassVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mGrassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrassVertices[0])*4, GrassVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &mGrassIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGrassIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GrassIndices[0])*6, GrassIndices, GL_STATIC_DRAW);
}

void MyWindow::resizeEvent(QResizeEvent *)
{
    mUpdateSize = true;
}

void MyWindow::render()
{
    if(!isVisible() || !isExposed())
        return;

    if (!mContext->makeCurrent(this))
        return;

    static bool initialized = false;
    if (!initialized) {
        initialize();
        initialized = true;
    }

    if (mUpdateSize) {
        glViewport(0, 0, size().width(), size().height());
        mUpdateSize = false;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Tree
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, mTreeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex), 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTreeTextureObject);
    glUniform1i(gTreeSamplerLocation, 0);

    static float Scale = 0.0f;
    Scale += 0.1f; // tut 12

    QMatrix4x4 WVP, World;    

    //World.translate(0.0f, 0.0f, 1.0f);
    //World.translate(0.0f, 0.0f, Scale/10.0f);
    World.rotate(Scale*2, 1.0f, 0.0f, 0.0f);
    //mPointLight.setPosition(mPointLight.getPosition()-QVector3D(0.0f, 0.0f, Scale/1000.0f));

    //WVP.perspective(60.0f, (float)this->width()/(float)this->height(), 1.0f, 100.0f);
    //WVP.lookAt(cam.position(), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
    QMatrix4x4 CameraMat(WVP);

    //PrintCoordOglDevOrig(QVector3D(0.0f,  0.0f, 0.0f), cam.position());
    //PrintCoordMoiRightHanded(QVector3D(0.0f,  0.0f, 0.0f), cam.position());

    //WVP *= World;

    mTreeProgram->bind();
    {        
        glUniform3f(gTreeCameraLocation, cam.position().x(), cam.position().y(), cam.position().z());
        glUniformMatrix4fv(gTreeVPLocation,  1, GL_FALSE, cam.matrix().constData());

        glDrawArrays(GL_POINTS, 0, 1);

        glDisableVertexAttribArray(0);
    }
    mTreeProgram->release();


    // Grass
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, mGrassVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), (const GLvoid *)((sizeof(GrassVertices[0].getPos()))+(sizeof(GrassVertices[0].getNormal()))));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mGrassTextureObject);
    glUniform1i(gGrassSamplerLocation, 0);

    mGrassProgram->bind();
    {
        glUniform3f(gGrassCameraLocation, cam.position().x(), cam.position().y(), cam.position().z());
        glUniformMatrix4fv(gGrassVPLocation,  1, GL_FALSE, cam.matrix().constData());

        //glDrawArrays(GL_TRIANGLES, 0, 4);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    mGrassProgram->release();


    mContext->swapBuffers(this);
}

void MyWindow::initShaders()
{
    QOpenGLShader vShader(QOpenGLShader::Vertex);
    QOpenGLShader gShader(QOpenGLShader::Geometry);
    QOpenGLShader fShader(QOpenGLShader::Fragment);    
    QFile         shaderFile;
    QByteArray    shaderSource;

    //mTreeProgram
    // Shader 1
    shaderFile.setFileName(":/vshader_tree.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "vertex tree compile: " << vShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/gshader_tree.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "geometry tree compile: " << gShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/fshader_tree.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "frag   tree compile: " << fShader.compileSourceCode(shaderSource);

    mTreeProgram = new (QOpenGLShaderProgram);
    mTreeProgram->addShader(&vShader);
    mTreeProgram->addShader(&gShader);
    mTreeProgram->addShader(&fShader);
    qDebug() << "shader link tree: " << mTreeProgram->link();

    //mGrassProgram
    // Shader 1
    shaderFile.setFileName(":/vshader_grass.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "vertex grass compile: " << vShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/fshader_grass.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "frag   grass compile: " << fShader.compileSourceCode(shaderSource);

    mGrassProgram = new (QOpenGLShaderProgram);
    mGrassProgram->addShader(&vShader);
    mGrassProgram->addShader(&fShader);
    qDebug() << "shader link grass: " << mGrassProgram->link();
}

void MyWindow::PrepareTexture(GLenum TextureTarget, const QString& FileName, GLuint& TexObject, bool flip)
{
    QImage TexImg;

    if (!TexImg.load(FileName)) qDebug() << "Erreur chargement texture";
    if (flip==true) TexImg=TexImg.mirrored();

    glGenTextures(1, &TexObject);
    glBindTexture(TextureTarget, TexObject);
    glTexImage2D(TextureTarget, 0, GL_RGB, TexImg.width(), TexImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, TexImg.bits());
    glTexParameterf(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void MyWindow::keyPressEvent(QKeyEvent *keyEvent)
{
    switch(keyEvent->key())
    {
        case Qt::Key_P:
            break;
        case Qt::Key_Up:
            break;
        case Qt::Key_Down:
            break;
        case Qt::Key_Left:
            break;
        case Qt::Key_Right:
            break;
        case Qt::Key_Delete:
            break;
        case Qt::Key_PageDown:
            break;
        case Qt::Key_Home:
            break;
        case Qt::Key_Z:
            //cam.setPosition(QVector3D(cam.position().x(), cam.position().y(), (cam.position().z())-1.0f));
            cam.offsetPosition(cam.forward());
            cam.printPosition();
            break;
        case Qt::Key_Q:
            //cam.setPosition(QVector3D(cam.position().x()-1.0f, cam.position().y(), cam.position().z()));
            cam.offsetPosition(-cam.right());
            cam.printPosition();
            break;
        case Qt::Key_S:
            //cam.setPosition(QVector3D(cam.position().x(), cam.position().y(), cam.position().z()+1.0f));
            cam.offsetPosition(-cam.forward());
            cam.printPosition();
            break;
        case Qt::Key_D:
            //cam.setPosition(QVector3D(cam.position().x()+1.0f, cam.position().y(), cam.position().z()));
            cam.offsetPosition(cam.right());
            cam.printPosition();
            break;
        case Qt::Key_A:
            cam.offsetOrientation(0.0f, -1.0f);
            break;
        case Qt::Key_E:
            cam.offsetOrientation(0.0f, 1.0f);
            break;
        default:
            break;
    }
}


void MyWindow::printMatrix(const QMatrix4x4& mat)
{
    const float *locMat = mat.transposed().constData();

    for (int i=0; i<4; i++)
    {
        qDebug() << locMat[i*4] << " " << locMat[i*4+1] << " " << locMat[i*4+2] << " " << locMat[i*4+3];
    }
}

void MyWindow::PrintCoordOglDevOrig(QVector3D pos, QVector3D cameraPos)
{
    QVector3D toCamera = QVector3D(cameraPos - pos).normalized();
    QVector3D up(0.0, 1.0, 0.0);
    QVector3D right = QVector3D::crossProduct(toCamera, up);
    QVector3D Pos(pos);

    qDebug() << "tocam: " << toCamera << " right: " << right;

    Pos -= (right * 0.5);
    qDebug() << "pos1: " << Pos;

    Pos.setY(Pos.y()+1.0);
    qDebug() << "pos2: " << Pos;

    Pos.setY(Pos.y()-1.0);
    Pos += right;
    qDebug() << "pos3: " << Pos;

    Pos.setY(Pos.y()+1.0);
    qDebug() << "pos4: " << Pos;
}

void MyWindow::PrintCoordMoiRightHanded(QVector3D pos, QVector3D cameraPos)
{
    QVector3D toCamera = QVector3D(cameraPos - pos).normalized();
    QVector3D up(0.0, 1.0, 0.0);
    QVector3D right = QVector3D::crossProduct(up, toCamera);
    QVector3D Pos(pos);

    qDebug() << "tocam: " << toCamera << " right: " << right;

    Pos += (right * 0.5);
    qDebug() << "pos1: " << Pos;

    Pos.setY(Pos.y()+1.0);
    qDebug() << "pos2: " << Pos;

    Pos -= right;
    Pos.setY(Pos.y()-1.0);
    qDebug() << "pos3: " << Pos;

    Pos.setY(Pos.y()+1.0);
    qDebug() << "pos4: " << Pos;
}
