#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QObject>
#include <QMatrix4x4>
#include <QMatrix3x3>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QQuaternion>
#include <QList>
#include <vector>
#include <memory>
#include <QOpenGLFunctions_3_3_Core>
#include<QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

#include "glmesh.h"
#include "axis.h"
#include "grid.h"
#include "material.h"
#include "guadandtriangle.h"

#include "arcballcameracontroller.h"
#define PAINT_FBO_WIDTH 2048


enum class ViewMode
{
    DEFAULT, TEXTURE, RENDER, UV
};

enum class TextureMode
{
    ALBEDO, METALLIC, ROUGHNESS, AMBIENT_OCCLUSION, EMISSIVE, DISPLACEMENT
};

class GLwidget: public QOpenGLWidget,protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    GLwidget(QWidget *parent=nullptr);
    ~GLwidget();
    void centercamera(const QVector3D setcenter);
    void setMesh(const IndexedMesh &_mesh);
    void toggleWireframe(bool _enabled);
    void setViewMode(ViewMode _viewMode);
    void setTextureMode(TextureMode _textureMode);
    void setTexture(const std::string &_filepath, TextureMode _textureType);
    void clearActiveTexture(const QVector3D &_clearColor);
    void clearAllTextures();
    Material *getMaterial();
    QVector3D getPaintColor() const;
    float getStrokeWidth() const;
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int _width, int _height) override;
    void mouseMoveEvent(QMouseEvent *_event) override;
    void mousePressEvent(QMouseEvent *_event) override;
    void wheelEvent(QWheelEvent *_event) override;
    void initshaider();
    void initcube(float width);
    void drawBrush();
    QOpenGLFramebufferObject* PaintFBO();
    QOpenGLFramebufferObject* _paintFbo = 0;
private:
    QOpenGLFunctions_3_3_Core *func;
    TextureMode textureMode;
    bool wireframe;
    ViewMode viewMode;
    // fbo and associated texture handles
    GLuint fbo;
    QOpenGLFramebufferObject *sourceFBO;
    QOpenGLFramebufferObject *targetFBO;
    GLuint colorTexture;
    GLuint uvTexture;
    GLuint depthTexture;
    GLuint paintFbo;
    int Width;
    int Height;
    std::shared_ptr<Camera> camera;
    ArcBallCameraController cameraController;
    GLuint paintTextureWidth;
    GLuint paintTextureHeight;
    QVector3D paintColor;
    QVector2D mouseCoord;
    QVector2D prevMouseCoord;
    int strokeWidth;
    bool restart;
    float uvZoom;

    QMatrix3x3 scale;
    QMatrix3x3 translate;

    QVector2D uvTranslate;
    QList<QVector2D> _strokePoints;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelMatrix;
    QMatrix4x4 m_projectionMatrixGrid;
    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_programGrid;
    QOpenGLShaderProgram m_programPaint;
    QOpenGLShaderProgram m_programTexture;
    QOpenGLShaderProgram m_programUV;
    QOpenGLShaderProgram m_programRender;
    QOpenGLShaderProgram m_programBLIT;
    QVector2D m_mousePosition;
    QQuaternion m_rotation;
    std::shared_ptr<GLMesh> glmesh;
    axis *Axis;
    Grid *grid;
    guadAndtriangle *guadandtringle;
    float m_z;
    float m_x;
    float m_y;
    QVector2D mouse_pos;
    GLuint getPaintTexture(TextureMode _textureMode);
    std::shared_ptr<Texture> createTexture(int _width, int _height);
    void createAttachments(int _width, int _height);
    void paint();
    QOpenGLTexture * m_texture;
    QOpenGLExtraFunctions* f;
    Material material;

    std::shared_ptr<Texture> irradianceTexture;
    std::shared_ptr<Texture> reflectanceTexture;
    std::shared_ptr<Texture> brdfLUT;
};


#endif // GLWIDGET_H
