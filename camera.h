#pragma once
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

class Camera
{
public:
    Camera();
    explicit Camera(const QVector3D &_position, const QQuaternion &_rotation);
    explicit Camera(const QVector3D &_position, const QVector3D &_pitchYawRoll);
    void setRotation(const QQuaternion &_rotation);
    void setPosition(const QVector3D &_position);
    void rotate(const QVector3D &_pitchYawRollOffset);
    void translate(const QVector3D &_translationOffset);
    void lookAt(const QVector3D &_targetPosition);
    void setZoom(float _zoom);
    const QMatrix4x4 &getViewMatrix();
    const QMatrix4x4 &getViewMattrix2();
    const QVector3D &getPosition() const;
    const QQuaternion &getRotation() const;
    QVector3D getForwardDirection();
    QVector3D getUpDirection();
    QVector3D getRightDirection();
    float getZoom() const;
    void reset();

private:
    QVector3D position;
    QQuaternion rotation;

    QVector3D startPosition;
    QQuaternion startRotation;

    QMatrix4x4 viewMatrix;
    QMatrix4x4 viewMatrix2;
    float zoom = 1.0f;
    bool needToUpdateViewMatrix = true;

    void updateViewMatrix();
};
