#include "scene.h"

using namespace Scene;
using namespace std;
using namespace glm;
/** Global variables **/
int Object::NEXTID = 0;
float jointRadius = 0.1;

// find rotation vector to align object axis with global z-axis


inline glm::vec3 composeRotation(const glm::vec3& w0, const glm::vec3& w1) {
    return axisAngle3(rotationMatrix(w1)*rotationMatrix(w0));
}

/* Method Definitions */

void Camera::pushRotation() const {
    glm::vec3 w = axisAngleAlignZY3(-_dir, _up);
    float angle = glm::length(w);
    if (angle > 0) {
        w /= angle;
        angle *= 180 / M_PI;
        glRotatef(angle, w[0], w[1], w[2]);
    }
}

void World::addObject(Object * obj)
{
    _objects.push_back(obj);
    obj->setWorld(this);
}

void World::draw()
{
    for (auto &object : _objects)
    {
        auto shader = _shaderMap.find(object->getID());
        if (shader != _shaderMap.end())
        {
            object->draw(_shaderMap[object->getID()]);
        }
        else
        {
            object->draw();
        }
    }
}

void World::removeObject(Object * obj)
{
    auto sameID = [&](Object * object) { return object->getID() == obj->getID();  };
    auto to_remove = std::remove_if(std::begin(_objects), std::end(_objects), sameID);
    _objects.erase(to_remove);
}

void World::assignShader(Object * obj, Shader * shader)
{
    _shaderMap[obj->getID()] = shader;
}
Shader * World::findShader(Object * obj)
{
    return _shaderMap[obj->getID()];
}

void Object::draw()
{
    if (!_visible) return;

    GLfloat color[] = { _color[0], _color[1], _color[2], _color[3] };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, color);

    float theta = _rotation._axis[0];
    float phi = _rotation._axis[1];

    glPushMatrix();
    glTranslated(_translation[0], _translation[1], _translation[2]);
    glRotatef(
        (180.0f/M_PI)*_rotation._angle,
        sin(theta)*cos(phi),
        sin(theta)*sin(phi),
        cos(theta));

    doDraw();

    glPopMatrix();
}

void Object::draw(Shader * shader)
{
    if (!_visible) return;

    float theta = _rotation._axis[0];
    float phi = _rotation._axis[1];

    glPushMatrix();
    glTranslated(_translation[0], _translation[1], _translation[2]);
    glRotatef(
        (180.0f / M_PI)*_rotation._angle,
        sin(theta)*cos(phi),
        sin(theta)*cos(phi),
        cos(theta));

    shader->link();
    doDraw();
    shader->unlink();
    glPopMatrix();


    glPopAttrib();
}

void Grid::doDraw()
{
    for (int r = -(_rows / 2); r <= (_rows / 2); r++)
    {
        GlutDraw::drawLine(
            glm::vec3(-(_cols / 2.0f)*_gap, 0, r*_gap),
            glm::vec3((_cols / 2.0f)*_gap, 0, r*_gap));
    }
    for (int c = -(_cols / 2); c <= (_cols / 2); c++)
    {
        GlutDraw::drawLine(
            glm::vec3(c*_gap, 0, -(_rows / 2.0f)*_gap),
            glm::vec3(c*_gap, 0, (_rows / 2.0f)*_gap));
    }
}



void Arrow::doDraw() {
    GlutDraw::drawLine(glm::vec3(0,0,0),glm::vec3(0,0,_length));
    float alpha = 1.0f / 16;
    GlutDraw::drawCone(glm::vec3(0, 0, (1 - alpha)*_length), _length*alpha / 2, glm::vec3(0, 0, alpha*_length));
}

void AnchoredBox::doDraw() {
    glPushMatrix();
    glTranslatef(0, 0, -_dimensions[2] / 2);
    Box::doDraw();
    glPopMatrix();
}

void Box::doDraw() {
    glm::vec3 half = _dimensions / 2.0f;
    GlutDraw::drawParallelepiped(glm::vec3(0, 0, 0), glm::vec3(half[0], 0, 0), glm::vec3(0, half[1], 0), glm::vec3(0, 0, half[2]));
}

void ObjGeometry::doDraw()
{
    if (!_geomReady)
    {
        _readGeom();
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, &_vertices[0]);
    glNormalPointer(GL_FLOAT, 0, &_normals[0]);

    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

    return;
}

// Adopted from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
int ObjGeometry::_readGeom()
{
    std::vector< int > vertexIndices, uvIndices, normalIndices;
    std::vector< glm::vec3 > tempVertices;
    std::vector< glm::vec2 > tempUVs;
    std::vector< glm::vec3 > tempNormals;
    int lineCount = 0;
    int faceCount = 0;
    int vertCount = 0;

    std::ifstream file;
    file.open(_filename, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "Could not open " << _filename << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream linestream(line);
        std::string type;
        if (line.find("v ") == 0)
        {
            glm::vec3 vertex;
            linestream >> type >> vertex.x >> vertex.y >> vertex.z;
            vertex.x = vertex.x;
            vertex.y = vertex.y;
            vertex.z = vertex.z;
            tempVertices.push_back(vertex);
            vertCount++;
        }
        else if (line.find("vn ") == 0)
        {
            glm::vec3 normal;
            linestream >> type >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        }
        else if (line.find("vt ") == 0)
        {
            glm::vec2 uv;
            linestream >> type >> uv.x >> uv.y;
            tempUVs.push_back(uv);
        }
        else if (line.find("f ") == 0)
        {
            int vertexIndex[3], normalIndex[3], uvIndex[3];
            char delim;
            linestream >> type >>
                vertexIndex[0] >> delim >> uvIndex[0] >> delim >> normalIndex[0] >>
                vertexIndex[1] >> delim >> uvIndex[1] >> delim >> normalIndex[1] >>
                vertexIndex[2] >> delim >> uvIndex[2] >> delim >> normalIndex[2];

            for (int i = 0; i < 3; i++)
            {
                vertexIndices.push_back(vertexIndex[i]);
                normalIndices.push_back(normalIndex[i]);
                uvIndices.push_back(uvIndex[i]);
            }
            faceCount++;
        }

        lineCount++;
        //if (lineCount % 1000 == 0)
        //{
        //    std::cout << "Parsing obj line: " << lineCount << "\r";
        //}
    }
    std::cout << "Parsed " << lineCount << " lines Verts: " << vertCount << " Triangles: " << faceCount << std::endl;
    file.close();

    for (int i = 0; i < vertexIndices.size(); i++)
    {
        int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = tempVertices[vertexIndex - 1];
        _vertices.push_back(vertex);
    }
    for (int i = 0; i < normalIndices.size(); i++)
    {
        int normalIndex = normalIndices[i];
        glm::vec3 normal = tempNormals[normalIndex - 1];
        _normals.push_back(normal);
    }
    for (int i = 0; i < uvIndices.size(); i++)
    {
        int uvIndex = uvIndices[i];
        glm::vec2 uv = tempUVs[uvIndex - 1];
        _uvs.push_back(uv);
    }

    _geomReady = true;

    return lineCount;
}

World & Scene::createWorld()
{
    World * new_world = new World();
    return *new_world;
}

void Shader::_initShaders()
{
    if (_vertfile == "" || _fragfile == "")
    {
        std::cout << "No shaders! Initialization failing." << std::endl;
        return;
    }
    else if (_shaderReady)
    {
        std::cout << "Shader has already initialized." << std::endl;
        return;
    }

    char *vs, *fs;

    if (_vertfile == "" && _fragfile == ""){ return; }
    _program = glCreateProgram();

    if (_vertfile != "")
    {
        _vertex = glCreateShader(GL_VERTEX_SHADER);
        vs = textFileRead(_vertfile.c_str());
        const char * vv = vs;
        glShaderSource(_vertex, 1, &vv, NULL);
        free(vs);
        glCompileShader(_vertex);
        if (_checkShaderError(_vertex))
        {
            std::cout << _vertfile << " compiled successfully." << std::endl;
            glAttachShader(_program, _vertex);
        }
    }
    if (_fragfile != "")
    {
        _frag = glCreateShader(GL_FRAGMENT_SHADER);
        fs = textFileRead(_fragfile.c_str());
        const char * ff = fs;
        glShaderSource(_frag, 1, &ff, NULL);
        free(fs);
        glCompileShader(_frag);
        if (_checkShaderError(_frag))
        {
            std::cout << _fragfile << " compiled successfully." << std::endl;
            glAttachShader(_program, _frag);
        }
    }

    glLinkProgram(_program);

    glDetachShader(_program, _vertex);
    glDetachShader(_program, _frag);
    glDeleteShader(_vertex);
    glDeleteShader(_frag);

    _shaderReady = true;
    return;
}

bool Shader::_checkShaderError(GLuint shader)
{
    GLint result = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result == GL_TRUE) return true;

    GLint logsize = 0;
    char * log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
    log = (char *)malloc(logsize + 1);
    glGetShaderInfoLog(shader, logsize, &result, log);

    std::cout << log << std::endl;
    return false;
}

void Shader::link()
{
    glUseProgram(_program);
}

void Shader::unlink()
{
    glUseProgram(0);
}





void Path::doDraw() {
    int n = 1024;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= n; i++) {
        glm::vec3 pt = _axisAngleRotation((float)i / n);
        glVertex3f(pt[0], pt[1], pt[2]);
    }
    glEnd();
}


Arm::Arm(std::vector<float>& lengths) :
Object(),
_lengths(lengths),
_localRotations(std::vector<AxisAngleRotation2>(lengths.size())),
_types(std::vector<int>(lengths.size(), BALL)),
_radius(*std::min_element(lengths.begin(), lengths.end()) / 8)
{
    int nJoints = _lengths.size();
    if (nJoints == 0) return;
    _globalRotations.resize(nJoints);
    _globalTranslations.resize(nJoints);
    glm::mat3 R = rotationMat(); // start with the anchor
    _globalTranslations[0] = _translation;
    R = rotationMatrix(_localRotations[0])*R;
    _globalRotations[0] = axisAngle2(R);
    for (int i = 1; i < nJoints; i++) {
        _globalTranslations[i] = _globalTranslations[i - 1] + R*glm::vec3(0, 0, _lengths[i - 1]);
        R = rotationMatrix(_localRotations[i])*R;
        _globalRotations[i] = axisAngle2(R);
    }
    _tip = _globalTranslations.back() + R*glm::vec3(0, 0, _lengths.back());

    for (int i = 0; i < nJoints; i++) {
        _dR_dTheta.push_back(glm::mat3(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)));
        _dR_dPhi.push_back(glm::mat3(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)));
        _dR_dAngle.push_back(glm::mat3(glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 0)));
    }

}

void Arm::printRotations() const {
    printf("base:  ");
    printVec3(_rotation.axisAngleRotation3());
    for (int i = 0; i < _lengths.size(); i++) {
        std::string head = "    :  ";
        std::string number = std::to_string(i);
        head.replace(head.begin(), head.begin() + number.length(), number);
        printf("%s", head.c_str());
        printVec3(_localRotations[i].axisAngleRotation3(), false);
        printf("  | ");
        printVec3(_globalRotations[i].axisAngleRotation3());
        printMat3(_globalRotations[i].rotationMatrix());
    }
}

void Arm::append(const float& length, const int& type, const glm::vec3& wLocal) {
    _lengths.push_back(length);
    _types.push_back(type);
    _localRotations.push_back(axisAngleRotation2(wLocal));
    _globalRotations.push_back(glm::vec3(0, 0, 0));
    _globalTranslations.push_back(glm::vec3(0, 0, 0));
    update(_lengths.size() - 1);
}
void Arm::append(const float& length, const int& type, const glm::vec2& axisLocal, const float& angleLocal) {
    _lengths.push_back(length);
    _types.push_back(type);
    _localRotations.push_back(AxisAngleRotation2(axisLocal,angleLocal));
    _globalRotations.push_back(glm::vec3(0, 0, 0));
    _globalTranslations.push_back(glm::vec3(0, 0, 0));
    update(_lengths.size() - 1);
}
void Arm::append(const float& length, const int& type, const AxisAngleRotation2& axisAngleLocal) {
    _lengths.push_back(length);
    _types.push_back(type);
    _localRotations.push_back(axisAngleLocal);
    _globalRotations.push_back(glm::vec3(0, 0, 0));
    _globalTranslations.push_back(glm::vec3(0, 0, 0));
    update(_lengths.size() - 1);
}

void Arm::updateGlobalTransforms(const int& index) {
    if (index == 2) {
        int j = 0;
    }
    int nJoints = _lengths.size();
    if (nJoints == 0) return;
    glm::mat3 R;
    if (index == 0) {
        R = rotationMat(); // starting at the anchor
        _globalTranslations[0] = _translation;
    }
    else {
        R = rotationMatrix(_globalRotations[index - 1]);
        _globalTranslations[index] = _globalTranslations[index - 1] + R*glm::vec3(0, 0, _lengths[index - 1]);
    }
    R = rotationMatrix(_localRotations[index])*R;
    _globalRotations[index] = AxisAngleRotation2(R);
    for (int i = index + 1; i < nJoints; i++) {
        _globalTranslations[i] = _globalTranslations[i - 1] + R*glm::vec3(0, 0, _lengths[i - 1]);
        R = rotationMatrix(_localRotations[i])*R;
        _globalRotations[i] = AxisAngleRotation2(R);
    }
    _tip = _globalTranslations.back() + R*glm::vec3(0, 0, _lengths.back());
}

void Arm::setLocalRotation(const int& joint, const glm::vec3& wLocal) {
    _localRotations[joint] = axisAngleRotation2(wLocal);
    update(joint);
}
void Arm::setLocalRotation(const int& joint, const AxisAngleRotation2& axisAngle) {
    _localRotations[joint] = axisAngle;
    update(joint);
}
void Arm::setLocalRotationAxis(const int& joint, const glm::vec3& axis) {
    _localRotations[joint]._axis = glm::vec2(acos(axis[2] / glm::length(axis)), atan2(axis[1], axis[0]));
    update(joint);
}
void Arm::setLocalRotationAxis(const int& joint, const glm::vec2& axis) {
    _localRotations[joint]._axis = axis;
    update(joint);
}
void Arm::setLocalRotationTheta(const int& joint, const float& theta) {
    _localRotations[joint]._axis[0] = theta;
    update(joint);
}
void Arm::setLocalRotationPhi(const int& joint, const float& phi) {
    _localRotations[joint]._axis[1] = phi;
    update(joint);
}
void Arm::setLocalRotationAngle(const int& joint, const float& angle) {
    _localRotations[joint]._angle = angle;
    update(joint);
}

float Arm::armLength() {
    float lengthSum = 0;
    for (int i = 0; i < _lengths.size(); i++) {
        lengthSum += _lengths[i];
    }
    return lengthSum;
}

float Arm::armReach() {
    float reach = 0;
    return reach;
}

void Arm::doDraw() {
    glPushMatrix();
    for (int i = 0; i < _lengths.size(); i++) {

        float theta = _localRotations[i]._axis[0];
        float phi = _localRotations[i]._axis[1];

        glRotatef(
            (180.0f / M_PI)*_localRotations[i]._angle,
            sin(theta)*cos(phi),
            sin(theta)*sin(phi),
            cos(theta));

        if (_types[i]==BALL) GlutDraw::drawSphere(glm::vec3(0, 0, 0), _radius);
        //if (_types[i] == PIN) GlutDraw::drawCylinder(glm::vec3(0,0,0),,_radius)
        GlutDraw::drawParallelepiped(glm::vec3(0, 0, _lengths[i] / 2), glm::vec3(0, _radius, 0), glm::vec3(_radius, 0, 0), glm::vec3(0, 0, _lengths[i] / 2 - _radius));

        glTranslatef(0, 0, _lengths[i]);
    }
    glPopMatrix();
}

void Arm::jiggle(const float& dzArcLength, const float& dPolar) {
    for (int i = 0; i < _lengths.size(); i++) {
        _localRotations[i].perturb(dzArcLength, dPolar);
    }
    update();
}


void Arm::updateRotationDerivative(const int& joint) {

    if (joint < 0) {
        for (int i = 0; i < _lengths.size(); i++) {
            updateRotationDerivative(i);
        }
        return;
    }

    float theta = _localRotations[joint]._axis[0];
    float phi = _localRotations[joint]._axis[1];
    float angle = _localRotations[joint]._angle;

    _dR_dAngle[joint][0][0] = 0;
    _dR_dAngle[joint][0][1] = cos(theta)*(cos(theta)*sin(angle) + cos(angle));
    _dR_dAngle[joint][0][2] = sin(theta)*sin(phi)*(sin(theta)*sin(phi)*sin(angle) - cos(angle));

    _dR_dAngle[joint][1][0] = cos(theta)*(cos(theta)*sin(angle) - cos(angle));
    _dR_dAngle[joint][1][1] = 0;
    _dR_dAngle[joint][1][2] = sin(theta)*cos(phi)*(sin(theta)*cos(phi)*sin(angle) + cos(angle));

    _dR_dAngle[joint][2][0] = sin(theta)*sin(phi)*(sin(theta)*sin(phi)*sin(angle) + cos(angle));
    _dR_dAngle[joint][2][1] = sin(theta)*cos(phi)*(sin(theta)*cos(phi)*sin(angle) - cos(angle));
    _dR_dAngle[joint][2][2] = 0;

    if (_types[joint] == BALL) {
        _dR_dTheta[joint][0][0] = 0;
        _dR_dTheta[joint][0][1] = (2 * (cos(angle) - 1)*cos(theta) - sin(angle))*sin(theta);
        _dR_dTheta[joint][0][2] = cos(theta)*sin(phi)*(2 * (cos(angle) - 1)*sin(theta)*sin(phi) - sin(angle));

        _dR_dTheta[joint][1][0] = (2 * (cos(angle) - 1)*cos(theta) + sin(angle))*sin(theta);
        _dR_dTheta[joint][1][1] = 0;
        _dR_dTheta[joint][1][2] = cos(theta)*cos(phi)*(2 * (cos(angle) - 1)*sin(theta)*cos(phi) + sin(angle));

        _dR_dTheta[joint][2][0] = cos(theta)*sin(phi)*(2 * (cos(angle) - 1)*sin(theta)*sin(phi) + sin(angle));
        _dR_dTheta[joint][2][1] = cos(theta)*cos(phi)*(2 * (cos(angle) - 1)*sin(theta)*cos(phi) - sin(angle));
        _dR_dTheta[joint][2][2] = 0;

        //////////////////////////////////////////////////////////////////////////////////////////////

        _dR_dPhi[joint][0][0] = 0;
        _dR_dPhi[joint][0][1] = 0;
        _dR_dPhi[joint][0][2] = sin(theta)*cos(phi)*(2 * (cos(angle) - 1)*sin(theta)*sin(phi) - sin(angle));

        _dR_dPhi[joint][1][0] = 0;
        _dR_dPhi[joint][1][1] = 0;
        _dR_dPhi[joint][1][2] = sin(theta)*sin(phi)*(2 * (cos(angle) - 1)*sin(theta)*cos(phi) - sin(angle));

        _dR_dPhi[joint][2][0] = sin(theta)*cos(phi)*(2 * (cos(angle) - 1)*sin(theta)*sin(phi) + sin(angle));
        _dR_dPhi[joint][2][1] = sin(theta)*sin(phi)*(2 * (cos(angle) - 1)*sin(theta)*cos(phi) + sin(angle));
        _dR_dPhi[joint][2][2] = 0;
    }
}

arma::mat Arm::forwardJacobian_analytic() const {
    int nJoints = _lengths.size();
    std::vector<glm::mat3> Rs(nJoints);
    std::vector<std::vector<glm::mat3>> R_accum(nJoints);
    for (int i = 0; i < nJoints; i++) {
        Rs[i] = rotationMatrix(_localRotations[i]);
    }
    for (int i = 0; i < nJoints; i++) {
        R_accum[i].push_back(Rs[i]);
        for (int j = i + 1; j < nJoints; j++) {
            R_accum[i].push_back(Rs[j] * R_accum[i].back());
        }
    }

    auto Rproduct = [&](const int& i, const int& j) {
        if (j < i) return glm::mat3();
        else return R_accum[i][j - i];
    };

    // set the size for the jacobian matrix
    int nParams = 0;
    for (int i = 0; i < nJoints; i++) {
        if (_types[i] == BALL) {
            nParams += 3;
        }
        else if (_types[i] == PIN) {
            nParams += 1;
        }
        else if (_types[i] == PRISM) {
            nParams += 1;
        }
    }
    arma::mat J(3,nParams);

    nParams = 0;
    glm::mat3 M;
    glm::vec3 dTip_dParam;
    for (int joint = 0; joint < nJoints; joint++) {
        for (int p = 0; p < 3; p++) {
            dTip_dParam = glm::vec3(0, 0, 0);
            if (p == 0) M = _dR_dTheta[joint] * Rproduct(0, joint - 1);
            else if (p == 1) M = _dR_dPhi[joint] * Rproduct(0, joint - 1);
            else M = _dR_dAngle[joint] * Rproduct(0, joint - 1);
            for (int i = joint; i < nJoints; i++) {
                dTip_dParam += _lengths[i] * (Rproduct(joint + 1, i) * M)[2];
            }
            J(0, nParams) = dTip_dParam[0];
            J(1, nParams) = dTip_dParam[1];
            J(2, nParams) = dTip_dParam[2];
            nParams++;
        }
    }
    J.print();
    return J;
}

arma::mat Arm::forwardJacobian_numeric() {
    int nJoints = _lengths.size();
    // set the size for the jacobian matrix
    int nParams = 0;
    for (int i = 0; i < nJoints; i++) {
        if (_types[i] == BALL) {
            nParams += 3;
        }
        else if (_types[i] == PIN) {
            nParams += 1;
        }
        else if (_types[i] == PRISM) {
            nParams += 1;
        }
    }
    arma::mat J(3, nParams);

    float dTheta = M_PI / 128;
    float dPhi = M_PI / 128;
    float dAngle = M_PI / 128;

    std::vector<AxisAngleRotation2> localRotations = _localRotations;
    std::vector<AxisAngleRotation2> globalRotations = _globalRotations;
    std::vector<glm::vec3> globalTranslations = _globalTranslations;

    glm::vec3 dTip_dParam;
    for (int joint = 0; joint < nJoints; joint++) {

        float theta = _localRotations[joint]._axis[0];
        setLocalRotationTheta(joint, theta + dTheta);
        dTip_dParam = _tip;
        setLocalRotationTheta(joint, theta - dTheta);
        dTip_dParam -= _tip;
        dTip_dParam /= (2 * dTheta);
        J(0, 3 * joint + 0) = dTip_dParam[0];
        J(1, 3 * joint + 0) = dTip_dParam[1];
        J(2, 3 * joint + 0) = dTip_dParam[2];
        _localRotations = localRotations;
        _globalRotations = globalRotations;
        _globalTranslations = globalTranslations;

        float phi = _localRotations[joint]._axis[1];
        setLocalRotationPhi(joint, phi + dPhi);
        dTip_dParam = _tip;
        setLocalRotationPhi(joint, phi - dPhi);
        dTip_dParam -= _tip;
        dTip_dParam /= (2 * dPhi);
        J(0, 3 * joint + 1) = dTip_dParam[0];
        J(1, 3 * joint + 1) = dTip_dParam[1];
        J(2, 3 * joint + 1) = dTip_dParam[2];
        _localRotations = localRotations;
        _globalRotations = globalRotations;
        _globalTranslations = globalTranslations;

        float angle = _localRotations[joint]._angle;
        setLocalRotationAngle(joint, angle + dAngle);
        dTip_dParam = _tip;
        setLocalRotationAngle(joint, angle - dAngle);
        dTip_dParam -= _tip;
        dTip_dParam /= (2 * dAngle);
        J(0, 3 * joint + 2) = dTip_dParam[0];
        J(1, 3 * joint + 2) = dTip_dParam[1];
        J(2, 3 * joint + 2) = dTip_dParam[2];
        _localRotations = localRotations;
        _globalRotations = globalRotations;
        _globalTranslations = globalTranslations;
    }
    J.print();
    return J;
}

void Arm::nudgeTip(const glm::vec3& displacement) {
    arma::mat J = arma::pinv(forwardJacobian_numeric());
    arma::vec d(3);
    d[0] = displacement[0];
    d[1] = displacement[1];
    d[2] = displacement[2];
    arma::vec dParams = J*d;
    for (int joint = 0; joint < _lengths.size(); joint++) {
        _localRotations[joint]._axis[0] += dParams[3 * joint + 0];
        _localRotations[joint]._axis[1] += dParams[3 * joint + 1];
        _localRotations[joint]._angle += dParams[3 * joint + 2];
    }
    update();
}