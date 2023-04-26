#pragma once
#include <QVector3D>
#include <QVector2D>
#include <QVector>
#include <vector>
#include <memory>
#include <QOpenGLFunctions_3_3_Core>
#include "glfunctions.h"
struct Vertex
{
    QVector3D position;
    QVector3D normal;
    QVector2D texCoord;
};

struct IndexedMesh
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

class GLMesh
{
public:
    static std::shared_ptr<GLMesh> createMesh(const IndexedMesh &_indexedMesh);

    GLMesh(const GLMesh &) = delete;
    GLMesh(const GLMesh &&) = delete;
    GLMesh &operator= (const GLMesh &) = delete;
    GLMesh &operator= (const GLMesh &&) = delete;
    ~GLMesh();
    void enableVertexAttribArrays() const;
    void render() const;

private:
    QOpenGLFunctions_3_3_Core *funcs;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    IndexedMesh indexedMesh;

    explicit GLMesh(const IndexedMesh &_indexedMesh);
};
