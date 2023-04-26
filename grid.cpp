#include "grid.h"

Grid::Grid()
    :func(getGLFunctions())
{
    GridVertex gridVertices[44];
    for (unsigned int i = 0; i < 11; ++i)
    {
        double xPos = (i - 5.0f);
        gridVertices[i * 2] = { QVector3D(xPos, 0.0f, -5.0f) * 0.1f, (i == 5) ? QVector3D(0.0f, 0.0f, 1.0f) : QVector3D(0.0,0.2,0.0) };
        gridVertices[i * 2 + 1] = { QVector3D(xPos, 0.0f, 5.0f) * 0.1f, (i == 5) ? QVector3D(0.0f, 0.0f, 1.0f) : QVector3D(0.0,0.2,0.0) };
    }

    for (unsigned int i = 0; i < 11; ++i)
    {
        double yPos = (i - 5.0f);
        gridVertices[22 + i * 2] = { QVector3D(-5.0f, 0.0f, yPos) * 0.1f, (i == 5) ? QVector3D(1.0f, 0.0f, 0.0f) : QVector3D(0.0,0.2,0.0) };
        gridVertices[22 + i * 2 + 1] = { QVector3D(5.0f, 0.0f, yPos) * 0.1f, (i == 5) ? QVector3D(1.0f, 0.0f, 0.0f) : QVector3D(0.0,0.2,0.0) };
    }

    // create buffers/arrays
    func->glGenVertexArrays(1, &gridVAO);
    func->glGenBuffers(1, &gridVBO);
    func->glBindVertexArray(gridVAO);
    func->glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    func->glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);

    // vertex positions
    func->glEnableVertexAttribArray(0);
    func->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GridVertex), (void*)0);

    // vertex color
    func->glEnableVertexAttribArray(1);
    func->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GridVertex), (void*)offsetof(GridVertex, color));

    func->glBindVertexArray(0);
}

void Grid::drawgrid()
{
    func->glBindVertexArray(gridVAO);
    func->glEnableVertexAttribArray(0);
    func->glEnableVertexAttribArray(1);

    func->glDrawArrays(GL_LINES, 0, 44);
}
