#include <QWindow>
#include <QTimer>
#include <QString>
#include <QKeyEvent>

#include <QVector3D>
#include <QMatrix4x4>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>

#include <QOpenGLShaderProgram>

#include "vertextex.h"
#include "vertex.h"
#include "Camera.h"

#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)

//class MyWindow : public QWindow, protected QOpenGLFunctions_3_3_Core
class MyWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit MyWindow();
    ~MyWindow();
    virtual void keyPressEvent( QKeyEvent *keyEvent );    

private slots:
    void render();

private:    
    void initialize();
    void modCurTime();

    void initShaders();
    void CreateVertexBuffer();

    void PrepareTexture(GLenum TextureTarget, const QString& FileName, GLuint& TexObject, bool flip);

    void PrintCoordOglDevOrig(QVector3D pos, QVector3D camera);
    void PrintCoordMoiRightHanded(QVector3D pos, QVector3D camera);

protected:
    void resizeEvent(QResizeEvent *);

private:
    QOpenGLContext *mContext;
    QOpenGLFunctions_3_3_Core *mFuncs;

    QOpenGLShaderProgram *mTreeProgram;
    QOpenGLShaderProgram *mGrassProgram;

    QTimer mRepaintTimer;
    double currentTimeMs;
    double currentTimeS;
    bool   mUpdateSize;

    GLuint mVAO, mTreeVBO, mTreeIBO;
    GLuint mGrassVBO, mGrassIBO;
    GLuint mTreeTextureObject, mGrassTextureObject;

    GLuint gTreeCameraLocation,  gTreeVPLocation,  gTreeSamplerLocation;
    GLuint gGrassCameraLocation, gGrassVPLocation, gGrassSamplerLocation;

    Vertex        *TreeVertices;
    VertexTex     *GrassVertices;
    unsigned int  *TreeIndices, *GrassIndices;
    tdogl::Camera cam;

    //debug
    void printMatrix(const QMatrix4x4& mat);
};
