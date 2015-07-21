#ifndef _BODYCOMPONENTS_H_
#define _BODYCOMPONENTS_H_

#include "stdafx.h"
#include "Rotation.h"
#include "GlutDraw.h"
#include "utils.h"
#include "Math.h"
#include "TreeNode.h"
#include "TreeNode.cpp"
#include "TransformStack.h"

enum {
    PIN = 0,
    BALL = 1,
    PRISM = 2
};

namespace Scene {

    class Bone;
    class Joint;
    class Socket;
    class Connection;
    class Skeleton;
    class SkeletonComponent;

    class SkeletonComponent // Wrapper class for Bones and Connections (Sockets and Joints)
    {
    public:
        SkeletonComponent() : _tGlobal(glm::vec3(0, 0, 0)), _wGlobal(glm::vec3(0, 0, 0)) {}
        SkeletonComponent(const glm::vec3& t, const glm::vec3& w) : _tGlobal(t), _wGlobal(w) {}

        glm::vec3 globalTranslation() const { return _tGlobal; }
        glm::vec3 globalRotation() const { return _wGlobal; }

        void setGlobalTranslation(const glm::vec3&);
        void setGlobalRotation(const glm::vec3&);
        void setGlobalTranslationAndRotation(const glm::vec3&, const glm::vec3&);

        virtual void dummy() {}

    protected:
        glm::vec3 _tGlobal;
        glm::vec3 _wGlobal;
    };

    

    class Bone : public SkeletonComponent
    {
        friend class Joint;
        friend class Socket;
        friend class Skeleton;
        friend class Connection;
    public:
        Bone() : SkeletonComponent(), _sockets(std::set<Socket*>()), _joints(std::set<Joint*>()) {}
        Bone(std::vector<Socket*> sockets, std::vector<Joint*> joints);
        void draw(const float& scale = 1) const;
        virtual void doDraw(const float& scale = 0.2) const;

        Joint* attach(Joint*);
        std::vector<Joint*> attach(std::vector<Joint*> joints) { for (auto joint : joints) attach(joint); return joints; }
        void detach(Joint*);

        Socket* attach(Socket*);
        std::vector<Socket*> attach(std::vector<Socket*> sockets) { for (auto socket : sockets) attach(socket); return sockets; }
        void detach(Socket*);

        std::map<Bone*, std::vector<Connection*>> Bone::reachableBonePaths(Connection* = NULL);
        std::set<Bone*> reachableBones(Connection* = NULL);
        Skeleton* addToSkeleton(Skeleton*, Connection* = NULL);

        Skeleton* skeleton() const { return _skeleton; }
        std::set<Joint*> joints() const { return _joints; }
        std::set<Socket*> sockets() const { return _sockets; }
        std::set<Connection*> connections() const;

        bool hasConnection(Connection* connection) const;

        std::map<Connection*, Bone*> connectionToBones() const;
        std::map<Socket*, Bone*> socketToBones() const;
        std::map<Joint*, Bone*> jointToBones() const;

        Connection* getConnectionToBone(Bone*) const;
        

    protected:
        Skeleton* _skeleton;
        std::set<Joint*> _joints;
        std::set<Socket*> _sockets;
    private:
        bool insertConnection(Connection*);
        bool eraseConnection(Connection*);
    };



    class Connection : public SkeletonComponent
    {
        friend class Bone;
        friend class Skeleton;
    public:
        Connection(const int& = 4, const float& = 1, Bone* = NULL);
        Connection(Bone* bone, const glm::vec3& t, const glm::vec3& w) :
            SkeletonComponent(), _bone(bone), _tFromBone(t), _wFromBone(w) {}

        virtual void draw(const float&) const;
        virtual void drawAnchor(const float&) const {
            GlutDraw::drawCylinder(_tFromBone / 2.0f, _tFromBone / 2.0f, 0.02);
        }
        virtual void drawPivot(const float&) const = 0;


        /////////////////
        //// GETTERS ////
        /////////////////

        Skeleton* skeleton() const;
        Connection* opposingConnection() const;
        Bone* opposingBone() const;
        std::pair<Socket*, Joint*> socketJoint();
        glm::vec3 translationToOpposingConnection() const;
        glm::vec3 rotationToOpposingConnection() const;
        glm::vec3 translationFromOpposingConnection() const;
        glm::vec3 rotationFromOpposingConnection() const;

        bool alignToConnection(Connection*, glm::vec3&, glm::vec3&);

        TreeNode<Connection*>* connectionTree(); // in the direction going away from _bone
        // For the other direction, call connectionTree() from opposingConnection()
        TreeNode<Bone*>* boneTree();

        virtual bool transformAnchorToTarget(glm::vec3&, glm::vec3&) const = 0;

        Bone* bone() const { return _bone; }
        glm::vec3 translationFromBone() const { return _tFromBone; }
        glm::vec3 rotationFromBone() const { return _wFromBone; }

        /////////////////
        //// SETTERS ////
        /////////////////

        void setTranslationFromBone(const glm::vec3& translation) { _tFromBone = translation; }
        void setRotationFromBone(const glm::vec3& w) { _wFromBone = Math::clampRotation(w); }

        Bone* attach(Bone* bone);
        void dettach();

    protected:
        Bone* _bone;
        glm::vec3 _tFromBone;
        glm::vec3 _wFromBone;
        
    };

    class Joint : public Connection
    {
        friend class Socket;
        friend class Skeleton;
    public:
        Joint(const int& i = 4, const float& scale = 1, Bone* bone = NULL) : Connection(i, scale, NULL) { Connection::attach(bone); }
        Joint(Bone* bone, const glm::vec3& t, const glm::vec3& w) : Connection(bone, t, w) {}

        Socket* couple(Socket* socket);
        void decouple();

        Socket* socket() const { return _socket; }

        bool transformAnchorToTarget(glm::vec3&, glm::vec3&) const;

        virtual int type() const { return -1; }
    protected:

        Socket* _socket;
    };

    class Socket :public Connection
    {
        friend class Joint;
    public:
        Socket(const int& i = 4, const float& scale = 1, Bone* bone = NULL);
        Socket(Bone* bone, const glm::vec3& t, const glm::vec3& w) :
            Connection(bone, t, w), _tToJoint(glm::vec3(0, 0, 0)), _wToJoint(glm::vec3(0, 0, 0)) {}

        /////////////////
        //// DRAWING ////
        /////////////////

        Joint* couple(Joint*);
        void decouple();

        //////////////////////////
        //// PARAMETERIZATION ////
        //////////////////////////

        virtual void constrainParams() {}
        virtual std::map<int, float> adjustableParams() const { return _params; }
        void setParams(const std::map<int, float>& params_unconstrained);
        void setParam(const int& key, const float& value);
        void setConstraint(const int& key, const float& value);

        void restore() { _params = _stashedParams; }
        void backup() { _stashedParams = _params; }

        void perturbJoint(const float& scale = 1);
        virtual float reach() const { return 0; }

        /////////////////
        //// GETTERS ////
        /////////////////

        std::map<int, float> params() const { return _params; }
        bool getConstraint(const int& key, float& value) const;
        bool getParam(const int& key, float& value) const;

        glm::vec3 translationToJoint() const { return _tToJoint; }
        glm::vec3 rotationToJoint() const { return _wToJoint; }
        glm::vec3 translationFromJoint() const { return Math::rotate(-_tToJoint, -_wToJoint); }
        glm::vec3 rotationFromJoint() const { return -_wToJoint; }
        Joint* joint() const { return _joint; }

        bool transformAnchorToTarget(glm::vec3&, glm::vec3&) const;

        // in the name of the function below, Params refers to the params of the socket
        // Tip refers to the translation from the local coordinate system origin to the position of the argument

        std::pair<arma::mat, arma::mat> J(SkeletonComponent* tip);

        virtual int type() const { return -2; }
    protected:
        //////////////////////////////////////////////////////////////////
        //// DANGEROUS FUNCTIONS THAT SHOULD NOT BE ACCESSED PUBLICLY ////
        //////////////////////////////////////////////////////////////////

        virtual void perturbParams(const float& scale) {}
        virtual void buildTransformsFromParams() {}
        virtual void buildParamsFromTransforms() {}


        /////////////////////////////////////////////////////////////////////////////////////////////
        //// The following should always be called via the connection, so we make them protected ////
        /////////////////////////////////////////////////////////////////////////////////////////////

        void setTranslationToJoint(const glm::vec3& translation);
        void setRotationToJoint(const glm::vec3& w);

        //////////////////////////
        //// MEMBER VARIABLES ////
        //////////////////////////

        Joint* _joint;

        glm::vec3 _tToJoint;            // IN THE COORDINATE FRAME OF THE JOINT's "BASE" (in "default" configuration)
        glm::vec3 _wToJoint;

        std::map<int, float> _constraints;  // Constraints are keyed on indices of our choosing
        std::map<int, float> _params;
        std::map<int, float> _stashedParams;
    };

    class Skeleton
    {
        friend class Bone;
    public:
        Skeleton() : _bones(std::set<Bone*>()) {}
        Skeleton(Bone* bone) { _bones = bone->reachableBones(); for (auto bone : _bones) bone->_skeleton = this; }

        std::set<std::pair<Socket*, Joint*>> socketJoints() const;
        std::set<Bone*> bones() const { return _bones; }
        std::set<Socket*> sockets() const;
        std::set<Joint*> joints() const;

        // The following updates global transformations assuming that the "root of input" is fixed (and doens't need to be updated)
        void updateGlobals(TreeNode<Bone*>* = NULL);

        void jiggle(const float& amplitude = 1) { for (auto socket : sockets()) socket->perturbJoint(amplitude); }
    private:
        std::set<Bone*> _bones;
    };

}
#endif