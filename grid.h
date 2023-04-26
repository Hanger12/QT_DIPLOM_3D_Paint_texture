#ifndef GRID_H
#define GRID_H
#include "glfunctions.h"
#include <QVector3D>
struct GridVertex
{
    QVector3D position;
    QVector3D color;
};
class Grid
{
public:
    Grid();
    void drawgrid();
private:
    QOpenGLFunctions_3_3_Core *func;
    GLuint gridVAO;
    GLuint gridVBO;
};

#endif // GRID_H
