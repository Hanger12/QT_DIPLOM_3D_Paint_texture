#include "guadandtriangle.h"
#include <QMessageBox>
guadAndtriangle::guadAndtriangle()
    :func(getGLFunctions())
{
    QVector3D positions[] =
    {
        {-0.5f, -0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f},
        {0.5f, 0.5f, 0.0f}
    };

    uint32_t indices[] =
    {
        0, 1, 2, 2, 1, 3
    };

    // create buffers/arrays
    func->glGenVertexArrays(1, &quadVAO);
    func->glGenBuffers(1, &quadVBO);
    func->glGenBuffers(1, &quadEBO);
    func->glBindVertexArray(quadVAO);
    func->glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    func->glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    func->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    func->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex positions
    func->glEnableVertexAttribArray(0);
    func->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), (void*)0);

    func->glBindVertexArray(0);

    QVector3D positionsT[] =
    {
        { -1.0f, -1.0f, 0.0f },
        { 3.0f, -1.0f, 0.0f },
        { -1.0f, 3.0f, 0.0f }
    };

    // create buffers/arrays
    func->glGenVertexArrays(1, &triangleVAO);
    func->glGenBuffers(1, &triangleVBO);
    func->glBindVertexArray(triangleVAO);
    func->glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    func->glBufferData(GL_ARRAY_BUFFER, sizeof(positionsT), positionsT, GL_STATIC_DRAW);

    // vertex positions
    func->glEnableVertexAttribArray(0);
    func->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), (void*)0);

    func->glBindVertexArray(0);
}

void guadAndtriangle::quadEnable()
{
    func->glBindVertexArray(quadVAO);
    func->glEnableVertexAttribArray(0);
}

void guadAndtriangle::triangleEnable()
{
    func->glBindVertexArray(triangleVAO);
    func->glEnableVertexAttribArray(0);
}
