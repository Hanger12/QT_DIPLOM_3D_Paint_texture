#include "axis.h"

axis::axis()
    :func(getGLFunctions())
{
    AxisVertex axisVertices[] =
    {
        // x axis

        { QVector3D(0.0f,0.0f,0.0f), QVector3D(1.0f, 0.0f, 0.0f) },
        { QVector3D(1.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f) },

        // arrow
        { QVector3D(1.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f) },
        { QVector3D(1.0f - 3.0f * arrow, arrow, arrow), QVector3D(1.0f, 0.0f, 0.0f) },

        { QVector3D(1.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f) },
        { QVector3D(1.0f - 3.0f * arrow, -arrow, arrow), QVector3D(1.0f, 0.0f, 0.0f) },

        { QVector3D(1.0f, 0.0f, 0.0f),QVector3D(1.0f, 0.0f, 0.0f) },
        { QVector3D(1.0f - 3.0f * arrow, arrow, -arrow), QVector3D(1.0f, 0.0f, 0.0f) },

        { QVector3D(1.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f) },
        { QVector3D(1.0f - 3.0f * arrow, -arrow, -arrow), QVector3D(1.0f, 0.0f, 0.0f) },

        // y axis

        { QVector3D(0.0f,0.0f,0.0f), QVector3D(0.0f, 1.0f, 0.0f) },
        { QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f) },

        // arrow
        { QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f) },
        { QVector3D(arrow, 1.0f - 3.0f * arrow, arrow),QVector3D(0.0f, 1.0f, 0.0f) },

        { QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f) },
        { QVector3D(-arrow,1.0f - 3.0f * arrow, arrow), QVector3D(0.0f, 1.0f, 0.0f) },

        { QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f) },
        { QVector3D(arrow, 1.0f - 3.0f * arrow, -arrow), QVector3D(0.0f, 1.0f, 0.0f) },

        { QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f) },
        { QVector3D(-arrow, 1.0f - 3.0f * arrow, -arrow), QVector3D(0.0f, 1.0f, 0.0f) },


        // z axis

        { QVector3D(0.0f,0.0f,0.0f),QVector3D(0.0f, 0.0f, 1.0f) },
        { QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f) },

        // arrow
        { QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f) },
        { QVector3D(arrow, arrow, 1.0f - 3.0f * arrow), QVector3D(0.0f, 0.0f, 1.0f) },

        { QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f) },
        { QVector3D(-arrow, arrow,1.0f - 3.0f * arrow), QVector3D(0.0f, 0.0f, 1.0f) },

        { QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f) },
        { QVector3D(arrow, -arrow, 1.0f - 3.0f * arrow), QVector3D(0.0f, 0.0f, 1.0f) },

        { QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f) },
        {QVector3D(-arrow, -arrow, 1.0f - 3.0f * arrow), QVector3D(0.0f, 0.0f, 1.0f) },

    };
    func->glGenVertexArrays(1, &axisVAO);
    func->glGenBuffers(1, &axisVBO);
    func->glBindVertexArray(axisVAO);
    func->glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    func->glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

    // vertex positions
    func->glEnableVertexAttribArray(0);
    func->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AxisVertex), (void*)0);

    // vertex color
    func->glEnableVertexAttribArray(1);
    func->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AxisVertex), (void*)offsetof(AxisVertex, color));

    func->glBindVertexArray(0);
}

void axis::DrawAxis()
{

    func->glBindVertexArray(axisVAO);
    func->glEnableVertexAttribArray(0);
    func->glEnableVertexAttribArray(1);

    func->glClear(GL_DEPTH_BUFFER_BIT);

    func->glDrawArrays(GL_LINES, 0, 30);
}

