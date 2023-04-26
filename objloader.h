#pragma once
#include "glmesh.h"

namespace OBJLoader
{
    enum class Error
    {
        SUCCESS, FAILURE
    };

    IndexedMesh loadOBJ(const char *_filepath, Error &_error);
}
