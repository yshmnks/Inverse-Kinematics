#include "SkeletonComponents.h"

using namespace std;
using namespace glm;
using namespace Math;

void Bone::draw(const float& scale) const {

    doDraw();

    for (auto joint : _joints) {
        joint->draw(0.1);
    }
}

void Bone::doDraw(const float& scale) const {

    glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
    GlutDraw::drawParallelepiped(glm::vec3(scale, 0, 0), glm::vec3(0, scale, 0) / 8.0f, glm::vec3(0, 0, scale) / 8.0f, glm::vec3(scale, 0, 0));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
    GlutDraw::drawParallelepiped(glm::vec3(0, scale, 0), glm::vec3(0, 0, scale) / 8.0f, glm::vec3(scale, 0, 0) / 8.0f, glm::vec3(0, scale, 0));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
    GlutDraw::drawParallelepiped(glm::vec3(0, 0, scale), glm::vec3(scale, 0, 0) / 8.0f, glm::vec3(0, scale, 0) / 8.0f, glm::vec3(0, 0, scale));

    glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
    GlutDraw::drawSphere(glm::vec3(0, 0, 0), glm::vec3(0, 0, scale));
}