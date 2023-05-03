#pragma once
#include "camera.h"
#include <memory>

class ArcBallCameraController
{
public:
    explicit ArcBallCameraController(const std::shared_ptr<Camera> &_camera, const QVector3D &_center, float _distance);
    void update(const QVector2D &_mouseDelta, float _scrollDelta, bool _translateCenter);
    void centerCamera();

private:
    std::shared_ptr<Camera> camera;
    QVector3D center;
    float distance;
    bool needupdateCenter=false;
};
