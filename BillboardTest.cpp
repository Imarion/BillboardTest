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
    if (Vertices != 0) delete[] Vertices;
    if (Indices  != 0) delete[] Indices;
    if (mProgram != 0) delete mProgram;
}

MyWindow::MyWindow() : currentTimeMs(0), currentTimeS(0)
{
    Vertices = 0;
    Indices  = 0;
    mProgram = 0;

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

    gCameraLocation  = mProgram->uniformLocation("camera");
    gModelLocation   = mProgram->uniformLocation("model");
    gSamplerLocation = mProgram->uniformLocation("gSampler");

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    PrepareTexture(GL_TEXTURE_2D, "./data/hackberry_tree_20131230_1040936985.png", mTextureObject);
}

void MyWindow::CreateVertexBuffer()
{
    // C++11 required
    Vertices = new VertexTex[NUM_VERTICES] {
        // front
        VertexTex(QVector3D(-1.0f,  1.0f, 1.0f),  QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D(-1.0f, -1.0f, 1.0f),  QVector2D(0.0f, 0.0f)), // lower left
        VertexTex(QVector3D( 1.0f, -1.0f, 1.0f),  QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D(-1.0f,  1.0f, 1.0f),  QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D( 1.0f, -1.0f, 1.0f),  QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D( 1.0f,  1.0f, 1.0f),  QVector2D(1.0f, 1.0f)), // upper right

        // left
        VertexTex(QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f)), // lower left
        VertexTex(QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(1.0f, 1.0f)), // upper right

        // right
        VertexTex(QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f)), // lower left
        VertexTex(QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f)), // upper right

        // back
        VertexTex(QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f)), // lower left
        VertexTex(QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f)), // upper right

        // top
        VertexTex(QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.0f, 0.0f)), // lower left
        VertexTex(QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(1.0f, 1.0f)), // upper right

        // bottom
        VertexTex(QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f)), // lower left
        VertexTex(QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 1.0f)), // upper left
        VertexTex(QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // lower right
        VertexTex(QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(1.0f, 1.0f)), // upper right

/*
        VertexTex(QVector3D(-1.0f, -1.0f, 0.0f),  QVector2D(0.0f, 0.0f)), // 0 lower left front
        VertexTex(QVector3D( 1.0f, -1.0f, 0.0f),  QVector2D(1.0f, 0.0f)), // 1 lower right front
        VertexTex(QVector3D(-1.0f,  1.0f, 0.0f),  QVector2D(0.0f, 1.0f)), // 2 upper left front
        VertexTex(QVector3D( 1.0f,  1.0f, 0.0f),  QVector2D(1.0f, 1.0f)), // 3 upper right front
        VertexTex(QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f)), // 4 lower left back (=lower right when looking at ...)
        VertexTex(QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f)), // 5 lower right back
        VertexTex(QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f)), // 6 upper left back
        VertexTex(QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f)), // 7 upper right back
*/
    };
    Indices = new unsigned int[NUM_INDICES] {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35
    };
    /*
    Indices = new unsigned int[NUM_INDICES] {
         0, 2, 3,
         0, 1, 2
    };
    */

    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0])*NUM_VERTICES, Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &mIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0])*NUM_INDICES, Indices, GL_STATIC_DRAW);
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

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex), 0);    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex), (const GLvoid*)(sizeof(Vertices[0].getPos())));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), (const GLvoid*)((sizeof(Vertices[0].getPos()))+(sizeof(Vertices[0].getNormal()))));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), (const GLvoid*)((sizeof(Vertices[0].getPos()))+(sizeof(Vertices[0].getNormal()))+sizeof(Vertices[0].getTexCoord())));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureObject);
    glUniform1i(gSamplerLocation, 0);

    static float Scale = 0.0f;
    Scale += 0.1f; // tut 12

    QMatrix4x4 WVP, World;
    QVector3D  CameraPos(0.0f, 0.0f, 5.0f);

    //World.translate(0.0f, 0.0f, 1.0f);
    //World.translate(0.0f, 0.0f, Scale/10.0f);
    World.rotate(Scale*2, 1.0f, 0.0f, 0.0f);
    //mPointLight.setPosition(mPointLight.getPosition()-QVector3D(0.0f, 0.0f, Scale/1000.0f));

    WVP.perspective(60.0f, (float)this->width()/(float)this->height(), 1.0f, 100.0f);
    WVP.lookAt(CameraPos, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
    QMatrix4x4 CameraMat(WVP);

    WVP *= World;

    mProgram->bind();
    {        
        glUniformMatrix4fv(gCameraLocation, 1, GL_FALSE, CameraMat.constData());
        glUniformMatrix4fv(gModelLocation,  1, GL_FALSE, World.constData());

        glDrawElements(GL_TRIANGLES, NUM_INDICES, GL_UNSIGNED_INT, 0);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
    }
    mProgram->release();

    mContext->swapBuffers(this);
}

void MyWindow::initShaders()
{
    QOpenGLShader vShader(QOpenGLShader::Vertex);
    QOpenGLShader gShader(QOpenGLShader::Geometry);
    QOpenGLShader fShader(QOpenGLShader::Fragment);    
    QFile         shaderFile;
    QByteArray    shaderSource;

    //mProgram
    // Shader 1
    shaderFile.setFileName(":/vshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "vertex 1 compile: " << vShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/gshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "geometry 1 compile: " << gShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/fshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "frag   1 compile: " << fShader.compileSourceCode(shaderSource);

    mProgram = new (QOpenGLShaderProgram);
    mProgram->addShader(&vShader);
    mProgram->addShader(&gShader);
    mProgram->addShader(&fShader);
    qDebug() << "shader link 1: " << mProgram->link();
}

void MyWindow::PrepareTexture(GLenum TextureTarget, const QString& FileName, GLuint& TexObject)
{
    QImage TexImg;

    if (!TexImg.load(FileName)) qDebug() << "Erreur chargement texture";
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
            break;
        case Qt::Key_Q:
            break;
        case Qt::Key_S:
            break;
        case Qt::Key_D:
            break;
        case Qt::Key_X:
            break;
        case Qt::Key_W:
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