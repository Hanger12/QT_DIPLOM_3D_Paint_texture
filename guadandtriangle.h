#ifndef GUADANDTRIANGLE_H
#define GUADANDTRIANGLE_H
#include"glfunctions.h"
#include <QVector3D>
class guadAndtriangle
{
public:
    guadAndtriangle();
    void quadEnable();
    void drawbuffers();
    void genbuffers();
    void defaulbuffers();
    void paint(QVector3D painColor,float strokWidth,bool restart,QVector2D mouseCoord);
    void createAttachments(int width,int height,QWidget *parent);

private:
    QOpenGLFunctions_3_3_Core *func;
    GLuint triangleVAO;
    GLuint triangleVBO;
    GLuint quadVAO;
    GLuint quadVBO;
    GLuint quadEBO;

    GLuint fbo;
    GLuint colorTexture;
    GLuint uvTexture;
    GLuint depthTexture;
    GLuint paintFbo;
};

#endif // GUADANDTRIANGLE_H
