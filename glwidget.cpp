#include "glwidget.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLExtraFunctions>
#include <iostream>
#include <thread>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QTextStream>
#include <QPainter>
#include <QMessageBox>
#include <QVector>
#include <stb/stb_image_write.h>
#include "texture.h"
#include "utility.h"
#include <QMatrix3x3>
#include <QGenericMatrix>
#include "sessionsettings.h"
GLwidget::GLwidget(QWidget *parent)
    :QOpenGLWidget(parent),
      textureMode(TextureMode::ALBEDO),
      wireframe(false),
      viewMode(ViewMode::DEFAULT),
      camera(new Camera()),
      cameraController(camera, QVector3D(0.0f,0.0f,0.0f), 5.0f),
      paintTextureWidth(2048),
      paintTextureHeight(2048),
      strokeWidth(60.0f),
      uvZoom(1.0f),
      uvTranslate(0.0, 1.0)
{
    m_z=-6.0;
    m_x=0.0f;
    m_y=0.0f;
}

GLwidget::~GLwidget()
{
    makeCurrent();
    doneCurrent();
}
QOpenGLTexture* brushTexture = 0;
QOpenGLContext* context= new QOpenGLContext();
void GLwidget::initializeGL()
{
    f = QOpenGLContext::currentContext()->extraFunctions();
    // get gl functions
    {
        initializeOpenGLFunctions();
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glLineWidth(1.0f);
        glEnable(GL_LINE_SMOOTH);
        glClearColor(1,1,1,0);
        initshaider();
        //initcube(1.0f);
        //m_meshes[0]->translete(QVector3D(0.0,0.0,0.0));
        //initcube(1.0f);
        //m_meshes[1]->translete(QVector3D(1.2,0.0,0.0));
    }

    Axis=new axis();
    grid=new Grid();
    guadandtringle=new guadAndtriangle();

    glGenFramebuffers(1, &fbo);
    glGenFramebuffers(1, &paintFbo);
    createAttachments(1, 1);
    brushTexture = new QOpenGLTexture(QImage(QString(":/image/brush1.png")));
    {
        material.setAlbedoMap(createTexture(paintTextureWidth, paintTextureHeight));
        material.setMetallicMap(createTexture(paintTextureWidth, paintTextureHeight));
        material.setRoughnessMap(createTexture(paintTextureWidth, paintTextureHeight));
        material.setAoMap(createTexture(paintTextureWidth, paintTextureHeight));
        material.setEmissiveMap(createTexture(paintTextureWidth, paintTextureHeight));
        material.setDisplacementMap(createTexture(paintTextureWidth, paintTextureHeight));

        clearAllTextures();
    }
    if (!QOpenGLContext::currentContext()->functions()->hasOpenGLFeature(QOpenGLFunctions::MultipleRenderTargets)) {
        qDebug("Multiple render targets not supported");
    }
    irradianceTexture = Texture::createTexture("D:/Project/QT_Diplom_3D_Paint/QT_DIPLOM_3D_Paint_texture/Textures/irradiance.dds");
    reflectanceTexture = Texture::createTexture("D:/Project/QT_Diplom_3D_Paint/QT_DIPLOM_3D_Paint_texture/Textures/reflectance.dds");
    brdfLUT = Texture::createTexture("D:/Project/QT_Diplom_3D_Paint/QT_DIPLOM_3D_Paint_texture/Textures/brdf.dds");

    // bind IBL textures
    {
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(irradianceTexture->getTarget(), irradianceTexture->getId());
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(reflectanceTexture->getTarget(), reflectanceTexture->getId());
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, brdfLUT->getId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    m_texture = new QOpenGLTexture(QImage(":/image/cube.jpeg").mirrored());
    m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture->setWrapMode(QOpenGLTexture::Repeat);
}

void GLwidget::paintGL()
{
    float r, g, b;
    settings()->brushColor().getRgbF(&r,&g,&b);
    paintColor = QVector3D(r,g,b);
    strokeWidth = settings()->brushSize();
    //glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    //GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    //glDrawBuffer(GL_COLOR_ATTACHMENT0);
    //glDrawBuffer(GL_COLOR_ATTACHMENT1);
    f->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    f->glDrawBuffers(2, bufs);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    float aspect = Width/ (Height ? (float)Height : 1);
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45,aspect,0.1,1000.0f);
    QMatrix4x4 viewMatrix;
    viewMatrix.setToIdentity();
    viewMatrix.translate(m_x,m_y,m_z);
    viewMatrix.rotate(m_rotation);
    QMatrix4x4 viewMatrixGrid;
    //    viewMatrixGrid.setToIdentity();
    //    viewMatrix.translate(0.0,-30.0,0.0);
    //    viewMatrix.rotate(m_rotation);
    if (glmesh)
    {
        switch (viewMode)
        {
        case ViewMode::DEFAULT:
        {
            m_program.bind();
            m_program.setUniformValue("u_projectionMatrix",m_projectionMatrix);
            m_program.setUniformValue("u_modelMatrix",m_modelMatrix);
            m_program.setUniformValue("u_lightpower",3.0f);
            m_program.setUniformValue("u_lightposition",QVector4D(0.0,0.0,0.0,1.0));
            m_program.setUniformValue("u_viewMatrix",viewMatrix);
            glmesh->enableVertexAttribArrays();
            m_program.setUniformValue("u_linemode",false);
            glmesh->render();
            if (wireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                m_program.setUniformValue("u_linemode",true);
                glmesh->render();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            m_program.release();
            break;
        }
        case ViewMode::TEXTURE:
        {
            m_programTexture.bind();
            m_programTexture.setUniformValue("uModelViewProjection",m_projectionMatrix*viewMatrix);
            m_programTexture.setUniformValue("uTexture",0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, getPaintTexture(textureMode));
            glmesh->enableVertexAttribArrays();
            glmesh->render();
            m_programTexture.release();
            break;
        }
        case ViewMode::UV:
        {
            m_programUV.bind();
            scale(0, 0) = uvZoom;
            scale(0, 1) = 0.0f;
            scale(0, 2) = 0.0f;
            scale(1, 0) = 0.0f;
            scale(1, 1) = uvZoom;
            scale(1, 2) = 0.0f;
            scale(2, 0) = 0.0f;
            scale(2, 1) = 0.0f;
            scale(2, 2) = 1.0;

            translate(0, 0) = 1.0f;
            translate(0, 1) = 0.0f;
            translate(0, 2) = uvTranslate.x();
            translate(1, 0) = 0.0f;
            translate(1, 1) = 1.0f;
            translate(1, 2) = 1.0-uvTranslate.y();
            translate(2, 0) = -0.0199004;
            translate(2, 1) = -0.47363;
            translate(2, 2) = 1.0f;
            m_programUV.setUniformValue("uTransformation",translate*scale);
            m_programUV.setUniformValue("uTexture",0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, getPaintTexture(textureMode));
            glmesh->enableVertexAttribArrays();
            m_programUV.setUniformValue("uGridMode",false);
            glmesh->render();
            if (wireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                m_programUV.setUniformValue("uGridMode",true);
                glmesh->render();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            m_programUV.release();
            break;
        }
        case ViewMode::RENDER:
        {
            m_programRender.bind();
            m_programRender.setUniformValue("uModelViewProjectionMatrix",m_projectionMatrix*camera->getViewMatrix());
            m_programRender.setUniformValue("uModelMatrix",m_modelMatrix);
            m_programRender.setUniformValue("uAtlasData",QVector4D(1.0f, 1.0f,0.0f, 0.0f));
            m_programRender.setUniformValue("uMaterial.albedo",material.getAlbedo());
            m_programRender.setUniformValue("uMaterial.metallic",material.getMetallic());
            m_programRender.setUniformValue("uMaterial.roughness",material.getRoughness());
            m_programRender.setUniformValue("uMaterial.emissive",material.getEmissive());
            m_programRender.setUniformValue("uMaterial.mapBitField",material.getMapBitField());
            m_programRender.setUniformValue("uLightColor",QVector3D(1.0,1.0f,1.0));
            m_programRender.setUniformValue("uLightDirection",QVector3D(1.0f, 1.0f, -1.0f).normalized());
            m_programRender.setUniformValue("uCamPos",camera->getPosition());
            m_programRender.setUniformValue("albedoMap",0);
            m_programRender.setUniformValue("metallicMap",2);
            m_programRender.setUniformValue("roughnessMap",3);
            m_programRender.setUniformValue("aoMap",4);
            m_programRender.setUniformValue("emissiveMap",5);
            m_programRender.setUniformValue("uDisplacementMap",6);
            m_programRender.setUniformValue("uIrradianceMap",7);
            m_programRender.setUniformValue("uPrefilterMap",8);
            m_programRender.setUniformValue("uBrdfLUT",9);
            material.bindTextures();
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(irradianceTexture->getTarget(), irradianceTexture->getId());
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(reflectanceTexture->getTarget(), reflectanceTexture->getId());
            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, brdfLUT->getId());
            glmesh->enableVertexAttribArrays();
            glmesh->render();
            m_programRender.release();
            break;
        }

        }
    }
    if (viewMode != ViewMode::UV&&viewMode != ViewMode::RENDER)
    {
        m_programGrid.bind();
        m_programGrid.setUniformValue("uModelViewProjection",m_projectionMatrix*viewMatrix);
        grid->drawgrid();
        Axis->DrawAxis();
    }
    f->glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    f->glReadBuffer(GL_COLOR_ATTACHMENT0);
    f->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
    f->glBlitFramebuffer(0,0,Width,Height,0, 0,Width,Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void GLwidget::resizeGL(int _width, int _height)
{
    Width = _width+300;
    Height = _height+200;
    createAttachments(Width,Height);
    //    m_projectionMatrixGrid.setToIdentity();
    //    m_projectionMatrixGrid.perspective(45,aspect,0.1,20.0f);
}

void GLwidget::mouseMoveEvent(QMouseEvent *_event)
{
    QVector2D diff =QVector2D(_event->position())-m_mousePosition;
    m_mousePosition = QVector2D(_event->position());
    if(_event->buttons()==Qt::RightButton)
    {
        if (viewMode != ViewMode::UV)
        {
            if(viewMode != ViewMode::RENDER)
            {
                float angle = diff.length()/2.0;
                QVector3D axis= QVector3D(diff.y(),diff.x(),0.0);
                m_rotation = QQuaternion::fromAxisAndAngle(axis,angle)*m_rotation;
                update();
            }
            else
            {
                cameraController.update(diff, 0.0f, false);
                update();
            }
        }
        else
        {
            uvTranslate += diff/(float)Width;
            update();
        }
    }
    if(_event->buttons()==Qt::LeftButton&&viewMode!=ViewMode::DEFAULT)
    {
        QPoint currentPosQ = this->mapFromGlobal(QCursor::pos());
        QVector2D currentPos = QVector2D(currentPosQ.x()+(currentPosQ.x()*0.25), currentPosQ.y()+(currentPosQ.y()*0.25+60));
        //QPoint currentPosQ = _event->pos();
        mouseCoord = currentPos;
        paint();
        update();
        //_strokePoints.append(QVector2D(_event->pos().x(), height()-_event->pos().y()));
    }
    if(_event->buttons()==Qt::MiddleButton&&viewMode != ViewMode::UV)
    {
        if(viewMode != ViewMode::RENDER)
        {

            m_x += diff.x()/160;
            m_y -= diff.y()/160;
            update();
        }
    }
}

void GLwidget::mousePressEvent(QMouseEvent *_event)
{
    QPoint currentPosQ = this->mapFromGlobal(QCursor::pos());
    QVector2D currentPos = QVector2D(currentPosQ.x()+(currentPosQ.x()*0.25), currentPosQ.y()+(currentPosQ.y()*0.25+60));
    mouseCoord = currentPos;
    if(_event->buttons()==Qt::RightButton)
    {
        m_mousePosition= QVector2D(_event->position());
    }
    if(_event->buttons()==Qt::LeftButton&&viewMode!=ViewMode::DEFAULT)
    {
        restart = true;
        paint();
        update();
        //_strokePoints.append(QVector2D(_event->pos().x(), height()-_event->pos().y()));
        //_strokePoints.append(QVector2D(_event->pos().x(), height()-_event->pos().y()));
    }
    if(_event->buttons()==Qt::MiddleButton&&viewMode!=ViewMode::UV)
    {
        m_mousePosition= QVector2D(_event->position());
    }
    _event->accept();
}

void GLwidget::wheelEvent(QWheelEvent *_event)
{
    if (viewMode != ViewMode::UV)
    {
        if(viewMode == ViewMode::RENDER)
        {
            cameraController.update(QVector2D(0.0f,0.0f), _event->angleDelta().y(), false);
            update();
        }
        else
        {
            if(_event->angleDelta().y()>0)
            {
                m_z+=0.25;
            }else if(_event->angleDelta().y()<0)
            {
                m_z-=0.25;
            }
            update();
        }

    }
    else
    {
        const float SCROLL_DELTA_MULT = 0.001f;
        uvZoom += _event->angleDelta().y() * SCROLL_DELTA_MULT;
        update();
    }
}

void GLwidget::initshaider()
{
    if(!m_programGrid.addShaderFromSourceFile(QOpenGLShader::Fragment,":/grid.frag"))
    {
        std::cout<<"ERROR::/grid.frag"<<std::endl;
        close();
    }
    if(!m_programGrid.addShaderFromSourceFile(QOpenGLShader::Vertex,":/grid.vert"))
    {
        std::cout<<"ERROR::/grid.vert"<<std::endl;
        close();
    }
    if(!m_programPaint.addShaderFromSourceFile(QOpenGLShader::Vertex,":/paint.vert"))
    {
        std::cout<<"ERROR::/paint.vert"<<std::endl;
        close();
    }
    if(!m_programPaint.addShaderFromSourceFile(QOpenGLShader::Fragment,":/paint.frag"))
    {
        std::cout<<"ERROR::/paint.frag"<<std::endl;
        close();
    }
    if(!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex,":/default1.vert"))
    {
        std::cout<<"ERROR::/default1.vert"<<std::endl;
        close();
    }
    if(!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment,":/default1.frag"))
    {
        std::cout<<"ERROR::/default1.frag"<<std::endl;
        close();
    }
    if(!m_programTexture.addShaderFromSourceFile(QOpenGLShader::Vertex,":/texture.vert"))
    {
        std::cout<<"ERROR::/texture.vert"<<std::endl;
        close();
    }
    if(!m_programTexture.addShaderFromSourceFile(QOpenGLShader::Fragment,":/texture.frag"))
    {
        std::cout<<"ERROR::/texture.frag"<<std::endl;
        close();
    }
    if(!m_programUV.addShaderFromSourceFile(QOpenGLShader::Vertex,":/UVview.vert"))
    {
        std::cout<<"ERROR::/UVview.vert"<<std::endl;
        close();
    }
    if(!m_programUV.addShaderFromSourceFile(QOpenGLShader::Fragment,":/UVview.frag"))
    {
        std::cout<<"ERROR::/UVview.frag"<<std::endl;
        close();
    }
    if(!m_programUV.link())
    {
        std::cout<<"ERROR:UVview"<<std::endl;
        close();
    }
    if(!m_programRender.addShaderFromSourceFile(QOpenGLShader::Vertex,":/Render.vert"))
    {
        std::cout<<"ERROR::/Render.vert"<<std::endl;
        close();
    }
    if(!m_programRender.addShaderFromSourceFile(QOpenGLShader::Fragment,":/Render.frag"))
    {
        std::cout<<"ERROR::/Render.frag"<<std::endl;
        close();
    }
    if(!m_programRender.link())
    {
        std::cout<<"ERROR:Render"<<std::endl;
        close();
    }
    if(!m_programBLIT.addShaderFromSourceFile(QOpenGLShader::Vertex,":/blit.vert"))
    {
        std::cout<<"ERROR::/blit.vert"<<std::endl;
        close();
    }
    if(!m_programBLIT.addShaderFromSourceFile(QOpenGLShader::Fragment,":/blit.frag"))
    {
        std::cout<<"ERROR::/blit.frag"<<std::endl;
        close();
    }
    if(!m_programBLIT.link())
    {
        std::cout<<"ERROR:blit"<<std::endl;
        close();
    }
    if(!m_program.link())
    {
        std::cout<<"ERROR:defoult1"<<std::endl;
        close();
    }
    if(!m_programTexture.link())
    {
        std::cout<<"ERROR:texture"<<std::endl;
        close();
    }
    if(!m_programPaint.link())
    {
        std::cout<<"ERROR:paint"<<std::endl;
        close();
    }
    if(!m_programGrid.link())
    {
        std::cout<<"ERROR:grid"<<std::endl;
        close();
    }
}

void GLwidget::initcube(float width)
{
    //    float width_div_2=width/2.0f;
    //    QVector<vertexData> vertexes;
    //    vertexes.append(vertexData(QVector3D(-width_div_2,width_div_2,width_div_2),QVector2D(0.0,1.0),QVector3D(0.0,0.0,1.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,-width_div_2,width_div_2),QVector2D(0.0,0.0),QVector3D(0.0,0.0,1.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,width_div_2,width_div_2),QVector2D(1.0,1.0),QVector3D(0.0,0.0,1.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,-width_div_2,width_div_2),QVector2D(1.0,0.0),QVector3D(0.0,0.0,1.0)));

    //    vertexes.append(vertexData(QVector3D(width_div_2,width_div_2,width_div_2),QVector2D(0.0,1.0),QVector3D(1.0,0.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,-width_div_2,width_div_2),QVector2D(0.0,0.0),QVector3D(1.0,0.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,width_div_2,-width_div_2),QVector2D(1.0,1.0),QVector3D(1.0,0.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,-width_div_2,-width_div_2),QVector2D(1.0,0.0),QVector3D(1.0,0.0,0.0)));

    //    vertexes.append(vertexData(QVector3D(width_div_2,width_div_2,width_div_2),QVector2D(0.0,1.0),QVector3D(0.0,1.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,width_div_2,-width_div_2),QVector2D(0.0,0.0),QVector3D(0.0,1.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,width_div_2,width_div_2),QVector2D(1.0,1.0),QVector3D(0.0,1.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,width_div_2,-width_div_2),QVector2D(1.0,0.0),QVector3D(0.0,1.0,0.0)));

    //    vertexes.append(vertexData(QVector3D(width_div_2,width_div_2,-width_div_2),QVector2D(0.0,1.0),QVector3D(0.0,0.0,-1.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,-width_div_2,-width_div_2),QVector2D(0.0,0.0),QVector3D(0.0,0.0,-1.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,width_div_2,-width_div_2),QVector2D(1.0,1.0),QVector3D(0.0,0.0,-1.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,-width_div_2,-width_div_2),QVector2D(1.0,0.0),QVector3D(0.0,0.0,-1.0)));

    //    vertexes.append(vertexData(QVector3D(-width_div_2,width_div_2,width_div_2),QVector2D(0.0,1.0),QVector3D(-1.0,0.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,width_div_2,-width_div_2),QVector2D(0.0,0.0),QVector3D(-1.0,0.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,-width_div_2,width_div_2),QVector2D(1.0,1.0),QVector3D(-1.0,0.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,-width_div_2,-width_div_2),QVector2D(1.0,0.0),QVector3D(-1.0,0.0,0.0)));

    //    vertexes.append(vertexData(QVector3D(-width_div_2,-width_div_2,width_div_2),QVector2D(0.0,1.0),QVector3D(0.0,-1.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(-width_div_2,-width_div_2,-width_div_2),QVector2D(0.0,0.0),QVector3D(0.0,-1.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,-width_div_2,width_div_2),QVector2D(1.0,1.0),QVector3D(0.0,-1.0,0.0)));
    //    vertexes.append(vertexData(QVector3D(width_div_2,-width_div_2,-width_div_2),QVector2D(1.0,0.0),QVector3D(0.0,-1.0,0.0)));

    //    QVector<GLuint> indexes;
    //    for(int i=0;i<24;i+=4)
    //    {
    //        indexes.append(i+0);
    //        indexes.append(i+1);
    //        indexes.append(i+2);
    //        indexes.append(i+2);
    //        indexes.append(i+1);
    //        indexes.append(i+3);
    //    }
    //    mesh = new Mesh(vertexes,indexes,QImage(":/image/cube.jpeg"));
}

void GLwidget::paint()
{
    // make the opengl context current
    makeCurrent();

    // bind the offscreen fbo and set read buffer to texture coordinate attachment
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT1);

    // we may need to restart the line
    if (restart)
    {
        restart = false;
        prevMouseCoord = mouseCoord;
    }

    // determine the amount of points to draw into the texture
    unsigned int pointCount = std::ceil
            (std::sqrt(std::pow(mouseCoord.x() - prevMouseCoord.x(), 2)
                       + std::pow(mouseCoord.y() - prevMouseCoord.y(), 2)));
    //std::cout<<"PointCount="<<pointCount<<std::endl;
    QVector2D textureSize(paintTextureWidth, paintTextureHeight);
    QVector<QVector2D> coords;
    for (unsigned int i = 0; i < pointCount; ++i)
    {
        double t = static_cast<double>(i) / static_cast<double>(pointCount);
        QVector2D coord = prevMouseCoord * (1.0 - t) + mouseCoord * t;
        //std::cout<<"coordX= "<<coord.x()<<" coordY= "<<coord.y()<<std::endl;
        //std::cout<<"mouseCoordX= "<<mouseCoord.x()<<" mouseCoordY= "<<mouseCoord.y()<<std::endl;
        QVector4D data;
        glReadPixels(GLint(coord.x()),GLint(Height-coord.y()), 1, 1, GL_RGBA, GL_FLOAT, &data);
        // a coordinate is only valid if marked by the fragment shader in the previous draw pass
        if (data.y() > 0.0)
        {
            coords.push_back(data.toVector2D());
        }
    }

    prevMouseCoord = mouseCoord;

    // bind paint shader
    m_programPaint.bind();
    if (textureMode == TextureMode::ALBEDO || textureMode == TextureMode::EMISSIVE)
    {
        m_programPaint.setUniformValue("uColor",paintColor);
    }
    else
    {
        m_programPaint.setUniformValue("uColor",QVector3D(0.922,0.914,0.651));
    }

    // bind the paint fbo and the current texture
    glBindFramebuffer(GL_FRAMEBUFFER, paintFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, getPaintTexture(textureMode), 0);
    // set view port
    glViewport(0, 0, paintTextureWidth, paintTextureHeight);

    // setup blend state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // bind quad mesh for painting points into the texture
    guadandtringle->quadEnable();
    // iterate over all texture coordinates and draw a point
    for (QVector2D &paintCoord : coords)
    {
        QMatrix4x4 transform;
        transform.translate(QVector3D(paintCoord*2.0 - QVector2D(1.0, 1.0), 0.0));
        transform.scale(QVector3D((1.0 / static_cast<double>(paintTextureWidth)) *
                                  static_cast<int>(strokeWidth), (1.0 / static_cast<double>(paintTextureHeight)) *
                                  static_cast<int>(strokeWidth), 1.0));
        m_programPaint.setUniformValue("uTransformation",transform);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    // reset state to default
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width(), height());
}

void GLwidget::drawBrush()
{
    brushTexture->bind();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), 0, height(), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    QPoint cursorP = this->mapFromGlobal(QCursor::pos());
    cursorP.setY(height() - cursorP.y());
    int brushRadius = 50;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0, 0);
        glVertex2f(-brushRadius + cursorP.x(), -brushRadius + cursorP.y());
        glTexCoord2f(1, 0);
        glVertex2f(brushRadius + cursorP.x(), -brushRadius + cursorP.y());
        glTexCoord2f(1, 1);
        glVertex2f(brushRadius + cursorP.x(), brushRadius + cursorP.y());
        glTexCoord2f(0, 1);
        glVertex2f(-brushRadius + cursorP.x(), brushRadius + cursorP.y());
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_MODELVIEW);
    brushTexture->release();
    update();
}

void GLwidget::clearAllTextures()
{
    makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, paintFbo);

    // albedo
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, material.getAlbedoMap()->getId(), 0);
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // metallic
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, material.getMetallicMap()->getId(), 0);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // roughness
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, material.getRoughnessMap()->getId(), 0);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // ao
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, material.getAoMap()->getId(), 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // emissive
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, material.getEmissiveMap()->getId(), 0);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // displacement
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, material.getDisplacementMap()->getId(), 0);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.2f,0.2f,0.2f,0);
}

Material *GLwidget::getMaterial()
{
    return &material;
}

QOpenGLFramebufferObject *GLwidget::PaintFBO()
{
    if (!_paintFbo) {
        QOpenGLFramebufferObjectFormat format;
        format.setInternalTextureFormat(GL_R32F);
        _paintFbo = new QOpenGLFramebufferObject(PAINT_FBO_WIDTH, PAINT_FBO_WIDTH, format);

        _paintFbo->bind();
        glClearColor(0,0,0,0); // red is paint intensity
        glClear(GL_COLOR_BUFFER_BIT);
        _paintFbo->release();
    }
    return _paintFbo;
}

void GLwidget::createAttachments(int _width, int _height)
{
    // this is safe since deleting a 0 value is defined as no-op
    glDeleteTextures(1, &colorTexture);
    glDeleteTextures(2, &uvTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    glGenTextures(1, &uvTexture);
    glBindTexture(GL_TEXTURE_2D, uvTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _width, _height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, uvTexture, 0);

    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, _width, _height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    f->glDrawBuffers(sizeof(bufs) / sizeof(GLenum), bufs);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        QMessageBox::critical(this, "Failed to initialize Framebuffer", "The framebuffer could not be successfully initialized. The program will now terminate.");
        exit(EXIT_FAILURE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint GLwidget::getPaintTexture(TextureMode _textureMode)
{
    switch (_textureMode)
    {
    case TextureMode::ALBEDO:
    {
        return  material.getAlbedoMap()->getId();
    }
    case TextureMode::METALLIC:
    {
        return  material.getMetallicMap()->getId();
    }
    case TextureMode::ROUGHNESS:
    {
        return  material.getRoughnessMap()->getId();
    }
    case TextureMode::AMBIENT_OCCLUSION:
    {
        return  material.getAoMap()->getId();
    }
    case TextureMode::EMISSIVE:
    {
        return  material.getEmissiveMap()->getId();
    }
    case TextureMode::DISPLACEMENT:
    {
        return  material.getDisplacementMap()->getId();
    }
    }
    return 0;
}

std::shared_ptr<Texture> GLwidget::createTexture(int _width, int _height)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return Texture::createTexture(tex, GL_TEXTURE_2D);
}

void GLwidget::centercamera(const QVector3D setcenter)
{
    m_x=setcenter.x();
    m_y=setcenter.y();
    m_z=setcenter.z();
    uvZoom=1.0;
    uvTranslate=QVector2D(0.0,1.0);
    m_rotation = QQuaternion(1,QVector3D(0.0,0.0,0.0));
    cameraController.centerCamera();
    update();
}

void GLwidget::setMesh(const IndexedMesh &_mesh)
{
    makeCurrent();

    // free this mesh before allocating storage for the new mesh
    glmesh.reset();

    glmesh = GLMesh::createMesh(_mesh);
}

void GLwidget::toggleWireframe(bool _enabled)
{
    wireframe = _enabled;
}

void GLwidget::setViewMode(ViewMode _viewMode)
{
    viewMode = _viewMode;
}

void GLwidget::setTextureMode(TextureMode _textureMode)
{
    textureMode = _textureMode;
}

void GLwidget::setTexture(const std::string &_filepath, TextureMode _textureType)
{
    makeCurrent();

    std::shared_ptr<Texture> tex = Texture::createTexture(_filepath, true);

    // we only use 2d textures
    if (tex->getTarget() != GL_TEXTURE_2D)
    {
        QMessageBox::critical(this, "Unsupported Texture Type", "The texture you tried to load is not supported as it is not 2D.");
        return;
    }

    // bind paint fbo and bind the requested texture as attachment
    glBindFramebuffer(GL_FRAMEBUFFER, paintFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, getPaintTexture(_textureType), 0);
    glViewport(0, 0, paintTextureWidth, paintTextureHeight);

    m_programBLIT.bind();
    m_programBLIT.setUniformValue("uSourceTexture",15);

    // bind triangle mesh for fullscreen pass
    guadandtringle->triangleEnable();

    // set user selected texture as source texture for blit shader
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, tex->getId());

    // fullscreen pass
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // rebind default fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void GLwidget::clearActiveTexture(const QVector3D &_clearColor)
{
    makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, paintFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, getPaintTexture(textureMode), 0);
    glClearColor(_clearColor.x(), _clearColor.y(), _clearColor.z(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.2f,0.2f,0.2f,0);
}

QVector3D GLwidget::getPaintColor() const
{
    return paintColor;
}

float GLwidget::getStrokeWidth() const
{
    return strokeWidth;
}

void GLwidget::saveTexture(const std::string &_filepath, TextureMode _textureType)
{
    makeCurrent();

    std::unique_ptr<unsigned char[]> textureData = std::make_unique<unsigned char[]>(paintTextureWidth * paintTextureHeight * 4);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, getPaintTexture(_textureType));

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.get());

    // save in seperate thread so that the main window does not get blocked
    std::thread t([textureData = std::move(textureData), _filepath, this]()
    {
        stbi_flip_vertically_on_write(false);
        stbi_write_png(_filepath.c_str(), paintTextureWidth, paintTextureHeight, 4, textureData.get(), 0);
    });
    t.detach();
}

void GLwidget::saveAllTextures(const std::string &_filepath)
{
    std::string formattedTime = Utility::getFormatedTime();

    saveTexture(_filepath + "/" + formattedTime + "_albedo.png", TextureMode::ALBEDO);
    saveTexture(_filepath + "/" + formattedTime + "_metallic.png", TextureMode::METALLIC);
    saveTexture(_filepath + "/" + formattedTime + "_roughness.png", TextureMode::ROUGHNESS);
    saveTexture(_filepath + "/" + formattedTime + "_ambient_occlusion.png", TextureMode::AMBIENT_OCCLUSION);
    saveTexture(_filepath + "/" + formattedTime + "_emissive.png", TextureMode::EMISSIVE);
    saveTexture(_filepath + "/" + formattedTime + "_displacement.png", TextureMode::DISPLACEMENT);
}
