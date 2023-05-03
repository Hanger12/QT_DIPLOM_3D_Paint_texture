#include "arcballcameracontroller.h"

ArcBallCameraController::ArcBallCameraController(const std::shared_ptr<Camera> &_camera, const QVector3D &_center, float _distance)
    :camera(_camera),
    center(_center),
    distance(_distance)
{
}

void ArcBallCameraController::update(const QVector2D &_mouseDelta, float _scrollDelta, bool _translateCenter)
{
    const float SCROLL_DELTA_MULT = -0.0005f;
    const float MOUSE_DELTA_MULT = 0.01f;
    distance += _scrollDelta * SCROLL_DELTA_MULT * distance;
    distance = qMax(0.0f, distance);
    if (_translateCenter)
    {
        center -= camera->getRightDirection() * _mouseDelta.x() * MOUSE_DELTA_MULT * 0.1f * distance;
        center += camera->getUpDirection() * _mouseDelta.y() * MOUSE_DELTA_MULT * 0.1f * distance;
    }
    else
    {
        if(!needupdateCenter)
        {
            float pitch = _mouseDelta.y()*1.2;
            float camY = camera->getForwardDirection().y();
            pitch = qBound(-1.0f, pitch - camY, 1.0f) + camY;
            camera->rotate(QVector3D(pitch, _mouseDelta.x() * 0.5f, 0.0f));
        }
        else
        {
            camera->updateViewMatrix();
            needupdateCenter=false;
            camera->rotate(QVector3D(0.0,0.0, 0.0f));
        }
    } 
    camera->setPosition(center - camera->getForwardDirection() * distance);
    camera->lookAt(center);
}

void ArcBallCameraController::centerCamera()
{
    needupdateCenter=true;
    center = QVector3D(0.0f,0.0f,0.0f);
    update(QVector2D(0.0f,0.0f), 0.0f, false);
}
