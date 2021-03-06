#ifndef _BALLSOCKETJOINT_H_
#define _BALLSOCKETJOINT_H_

#include "stdafx.h"
#include "BodyComponents.h"
#include "Rotation.h"
#include "Math.h"

// PARAMETERS FOR BALLJOINT:
// 0 : main-axis theta
// 1 : main-axis phi
// 2 : rotation angle about the main-axis

namespace Scene {

    class BallSocket : public Socket
    {
    public:
        BallSocket(const int& i = 4, const float& scale = 1, Bone* bone = NULL) : Socket(i, scale, bone) {}
        BallSocket(Bone* bone, const glm::vec3& t, const glm::vec3& w) : Socket(bone, t, w) {}

        std::map<int, float> adjustableParams() const;
        void buildTransformsFromParams() {
            _wToJoint = Math::w(AxisSpinRotation(glm::vec2(_params[0], _params[1]), _params[2]));
        }
        void buildParamsFromTransforms();
        void constrainParams();
        void perturbParams(const float& scale);

        void drawPivot(const float&) const;

        int type() const { return BALL; }
    private:
    };

    class BallJoint : public Joint
    {
    public:
        BallJoint(const int& i = 4, const float& scale = 1, Bone* bone = NULL) : Joint(i, scale, bone) {}
        BallJoint(Bone* bone, const glm::vec3& t, const glm::vec3& w) : Joint(bone, t, w) {}

        void drawPivot(const float&) const;

        int type() const { return BALL; }
    private:
    };

}

#endif