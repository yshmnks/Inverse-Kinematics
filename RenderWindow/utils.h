#pragma once

#include "stdafx.h"

// Utility functions
char * textFileRead(const char *);

// From http://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
#ifndef GLERROR_H
#define GLERROR_H
 
void _check_gl_error(const char *file, int line);
 
///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
 
#endif // GLERROR_H

std::string zero_padded_name(std::string, int, int);

struct AxisAngleRotation2 {
    glm::vec2 _axis;
    float _angle;
    AxisAngleRotation2() : _axis(glm::vec2(0, 0)), _angle(0) {}
    AxisAngleRotation2(const glm::vec2& axis, const float& angle) : _axis(axis), _angle(angle) {}
    AxisAngleRotation2(const float& angle, const glm::vec2& axis) : _axis(axis), _angle(angle) {}
    AxisAngleRotation2(const glm::vec3& w) : _axis(glm::normalize(w)), _angle(glm::length(w)) {}
    AxisAngleRotation2(const glm::mat3& R);
    float theta() const { return _axis[0]; }
    float phi() const  { return _axis[1]; }
    void clampAngle() { _angle -= (2 * M_PI)*floor(_angle / (2 * M_PI)); }
    void perturb(const float& dzArcLength, const float& dPolar);
    // consider the rotation vector as specifying the orientation of a coordinate system
    // perturb(...) reorients this coordinate system slightly by...
    // jiggling its z-axis uniformly in a small "hemispherical cap" with geodesic-radius dzArcLength
    // and after having done so, rotates the new x-axis and new y-axis about the new z-axis uniformly in range [-dPolar,dPolar]
    glm::vec3 axisAngleRotation3() const { return _angle*glm::vec3(sin(_axis[0])*cos(_axis[1]), sin(_axis[0])*sin(_axis[1]), cos(_axis[1])); }
    glm::mat3 rotationMatrix() const;
    glm::mat3 coordinateAxes() const;
};


glm::vec3 axisAngle3(const glm::mat3& R);
AxisAngleRotation2 axisAngle2(const glm::mat3& R);

glm::mat3 rotationMatrix(const glm::vec3& w);
glm::mat3 rotationMatrix(const AxisAngleRotation2& axisAngle);
glm::mat3 rotationMatrix(const glm::vec2& axis, const float& angle);
glm::mat3 rotationMatrix(const float& angle, const glm::vec2& axis);

glm::vec3 axisAngleAlignZY3(const glm::vec3& zIn, const glm::vec3& yIn);
AxisAngleRotation2 axisAngleAlignZY2(const glm::vec3& zIn, const glm::vec3& yIn);

glm::vec3 axisAngleRotation3(const glm::vec2& axis, float angle);
glm::vec3 axisAngleRotation3(float angle, const glm::vec2& axis);
glm::vec3 axisAngleRotation3(const AxisAngleRotation2& axisAngle);

AxisAngleRotation2 axisAngleRotation2(const glm::vec3& w);

glm::vec3 axisAngleAlignZ3(const glm::vec3& axisIn);
AxisAngleRotation2 axisAngleAlignZ2(const glm::vec3& axis);
glm::vec3 axisAngleAlignVectors(const glm::vec3& axis, const glm::vec3& target);