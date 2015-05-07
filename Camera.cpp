/*
 tdogl::Camera

 Copyright 2012 Thomas Dalling - http://tomdalling.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include "Camera.h"

using namespace tdogl;

static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock

static inline float RadiansToDegrees(float radians) {
    return radians * 180.0f / (float)M_PI;
}

#include <iostream>
#include <sstream>
//#include <Windows.h>
#include <QDebug>

using namespace std;

Camera::Camera() :
    _position(0.0f, 0.0f, 1.0f),
    _horizontalAngle(0.0f),
    _verticalAngle(0.0f),
    _fieldOfView(50.0f),
    _nearPlane(0.01f),
    _farPlane(100.0f),
    _viewportAspectRatio(4.0f/3.0f)
{
}

const QVector3D& Camera::position() const {
    return _position;
}

void Camera::setPosition(const QVector3D& position) {
    _position = position;
}

void Camera::offsetPosition(const QVector3D& offset) {
    _position += offset;
}

float Camera::fieldOfView() const {
    return _fieldOfView;
}

void Camera::setFieldOfView(float fieldOfView) {
    Q_ASSERT(fieldOfView > 0.0f && fieldOfView < 180.0f);
    _fieldOfView = fieldOfView;
}

float Camera::nearPlane() const {
    return _nearPlane;
}

float Camera::farPlane() const {
    return _farPlane;
}

void Camera::setNearAndFarPlanes(float nearPlane, float farPlane) {
    Q_ASSERT(nearPlane > 0.0f);
    Q_ASSERT(farPlane > nearPlane);
    _nearPlane = nearPlane;
    _farPlane = farPlane;
}

QMatrix4x4 Camera::orientation() const {
    QMatrix4x4 orientation;
    orientation.rotate(_verticalAngle, QVector3D(1,0,0));
    orientation.rotate(_horizontalAngle, QVector3D(0,1,0));
    return orientation;
}

void Camera::offsetOrientation(float upAngle, float rightAngle) {
    _horizontalAngle += rightAngle;
    _verticalAngle += upAngle;
    normalizeAngles();
}

void Camera::lookAt(QVector3D position) {
    Q_ASSERT(position != _position);
    QVector3D direction(position - _position);
    direction.normalize();
    _verticalAngle = RadiansToDegrees(asinf(-direction.y()));
    _horizontalAngle = -RadiansToDegrees(atan2f(-direction.x(), -direction.z()));
    normalizeAngles();
}

float Camera::viewportAspectRatio() const {
    return _viewportAspectRatio;
}

void Camera::setViewportAspectRatio(float viewportAspectRatio) {
    Q_ASSERT(viewportAspectRatio > 0.0);
    _viewportAspectRatio = viewportAspectRatio;
}

QVector3D Camera::forward() const {
    QVector4D forward = (orientation().inverted()) * QVector4D(0,0,-1,1);
    return QVector3D(forward);
}

QVector3D Camera::right() const {
    QVector4D right = (orientation().inverted()) * QVector4D(1,0,0,1);
    return QVector3D(right);
}

QVector3D Camera::up() const {
    QVector4D up = (orientation().inverted()) * QVector4D(0,1,0,1);
    return QVector3D(up);
}

QMatrix4x4 Camera::matrix() const {
    return projection() * view();
}

QMatrix4x4 Camera::projection() const {
    QMatrix4x4 temp;
    temp.perspective(_fieldOfView, _viewportAspectRatio, _nearPlane, _farPlane);
    return temp;
}

QMatrix4x4 Camera::view() const {
    QMatrix4x4 temp;
    temp.translate(-_position);
    return orientation() * temp;
}

void Camera::normalizeAngles() {
    _horizontalAngle = fmodf(_horizontalAngle, 360.0f);
    //fmodf can return negative values, but this will make them all positive
    if(_horizontalAngle < 0.0f)
        _horizontalAngle += 360.0f;

    if(_verticalAngle > MaxVerticalAngle)
        _verticalAngle = MaxVerticalAngle;
    else if(_verticalAngle < -MaxVerticalAngle)
        _verticalAngle = -MaxVerticalAngle;
}

void Camera::printPosition() {
    qDebug() << "x: " << _position.x() << endl;
    qDebug() << "y: " << _position.y() << endl;
    qDebug() << "z: " << _position.z() << endl;
}
