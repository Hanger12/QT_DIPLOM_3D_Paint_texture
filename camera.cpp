#include "camera.h"

Camera::Camera()
{
}

Camera::Camera(const QVector3D &_position, const QQuaternion &_rotation)
    : position(_position), rotation(_rotation), startPosition(_position), startRotation(_rotation)
{
}

Camera::Camera(const QVector3D & _position, const QVector3D &_pitchYawRoll)
    : position(_position), rotation(QQuaternion::fromEulerAngles(_pitchYawRoll)), startPosition(_position), startRotation(QQuaternion::fromEulerAngles(_pitchYawRoll))
{
}

void Camera::setRotation(const QQuaternion &_rotation)
{
    rotation = _rotation;
    needToUpdateViewMatrix = true;
}

void Camera::setPosition(const QVector3D &_position)
{
    position = _position;
    needToUpdateViewMatrix = true;
}

void Camera::rotate(const QVector3D &_pitchYawRoll)
{
    QQuaternion pitch = QQuaternion::fromAxisAndAngle(QVector3D(2.0,0.0,0.0),_pitchYawRoll.x());
    QQuaternion yaw = QQuaternion::fromAxisAndAngle(QVector3D(0.0,2.0,0.0),_pitchYawRoll.y());
    rotation = pitch*rotation*yaw;
    rotation.normalize();
    viewMatrix.setToIdentity();
    viewMatrix.rotate(rotation.inverted());
    needToUpdateViewMatrix = true;
}

void Camera::translate(const QVector3D &_translation)
{
    // Get the camera's forward and right directions
    QVector3D forward(viewMatrix(0, 2), viewMatrix(1, 2), viewMatrix(2, 2));
    QVector3D right(viewMatrix(0, 0), viewMatrix(1, 0), viewMatrix(2, 0));

    // Scale the translation vector by the camera's speed
    static const float speed = 1.12f;
    QVector3D scaledTranslation = (_translation.z() * forward + _translation.x() * right) * speed;

    // Update the camera position
    position += scaledTranslation;

    needToUpdateViewMatrix = true;
}

void Camera::lookAt(const QVector3D &_targetPosition)
{
    static const QVector3D UP(0.0, 1.0f, 0.0);
    viewMatrix.setToIdentity();
    viewMatrix.lookAt(position, _targetPosition, UP);
    needToUpdateViewMatrix = false;
}

void Camera::setZoom(float _zoom)
{
    assert(zoom > 0.0f);
    zoom = _zoom;
    //SystemManager::getInstance().getSystem<RenderSystem>()->getWindow()->setFieldOfView(Window::DEFAULT_FOV / zoom);
}

const QMatrix4x4 &Camera::getViewMatrix()
{
    return viewMatrix;
}

const QMatrix4x4 &Camera::getViewMattrix2()
{
    return viewMatrix2;
}

const QVector3D &Camera::getPosition() const
{
    return position;
}

const QQuaternion &Camera::getRotation() const
{
    return rotation;
}

QVector3D Camera::getForwardDirection()
{
    return -viewMatrix.column(2).toVector3D().normalized();
}

QVector3D Camera::getUpDirection()
{
    return viewMatrix.column(1).toVector3D().normalized();
}

QVector3D Camera::getRightDirection()
{
    return viewMatrix.column(0).toVector3D().normalized();
}

float Camera::getZoom() const
{
    return zoom;
}

void Camera::reset()
{
    position = startPosition;
    rotation = startRotation;
    needToUpdateViewMatrix = true;
}

void Camera::updateViewMatrix()
{
    viewMatrix.translate(position);
}
