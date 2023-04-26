#ifndef AXIS_H
#define AXIS_H
#include"glfunctions.h"
#include <QVector3D>
struct AxisVertex
{
   QVector3D position;
   QVector3D color;
};

const float arrow = 0.05f;
class axis
{
public:
    axis();
    void DrawAxis();
private:
    QOpenGLFunctions_3_3_Core *func;
    GLuint axisVAO;
    GLuint axisVBO;
};

#endif // AXIS_H
