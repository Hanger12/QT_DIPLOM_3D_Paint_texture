#include "glfunctions.h"
#include <QOpenGLContext>
#include <QDebug>

QOpenGLFunctions_3_3_Core* getGLFunctions()
{
    static QOpenGLFunctions_3_3_Core* funcs = nullptr;

    if (!funcs)
    {
        QOpenGLContext* currentContext = QOpenGLContext::currentContext();
        if (!currentContext)
        {
            qWarning() << "Could not obtain current OpenGL context";
            return nullptr;
        }

        funcs = new QOpenGLFunctions_3_3_Core;
        if (!funcs->initializeOpenGLFunctions())
        {
            qWarning() << "Failed to initialize OpenGL functions";
            delete funcs;
            funcs = nullptr;
            return nullptr;
        }
    }

    return funcs;
}
