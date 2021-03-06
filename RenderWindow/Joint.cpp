#include "BodyComponents.h"
#include "GlutDraw.h"

using namespace std;
using namespace glm;
using namespace Math;
using namespace Scene;

Socket* Joint::couple(Socket* socket) {
    if (socket == _socket || socket->type() != type())
        return socket;
    if (socket == NULL)
        decouple();
    else {
        if (_bone != NULL) {
            _bone->_joints.erase(this);
            _bone->attach(this);
        }
        socket->_joint = this;
        _socket = socket;
    }
    return socket;
}
void Joint::decouple() {
    if (_socket != NULL) {
        if (_bone != NULL) {
            Bone* bone = _bone;                 // 1: Backup the bone to which this socket is anchored
            _bone->detach(this);                // 2: Detach this socket from the anchor (keeping the socket-joint link in tact)
            bone->_joints.insert(this);         // 3: Reattach this socket to its anchor without skeleton updates
        }
        _socket->_joint = NULL;                 // 4: Sever the socket-joint link
        _socket = NULL;                         //    ...
    }
}

bool Joint::transformAnchorToTarget(glm::vec3& t, glm::vec3& w) const {
    glm::vec3 translation;
    glm::vec3 rotation;
    if (!_socket->transformAnchorToTarget(translation, rotation)) return false;
    t = Math::R(-rotation)*(-translation);
    w = -rotation;
    return true;
}
