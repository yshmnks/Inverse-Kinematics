#include "scene.h"
#include "utils.h"

using namespace Scene;
using namespace std;
using namespace glm;
/** Global variables **/
int Object::NEXTID = 0;
float jointRadius = 0.1;

/* Method Definitions */
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

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(_color[0], _color[1], _color[2], _color[3]);

    float theta = glm::length(_w);
    glm::vec3 wHat;
    wHat = _w / theta;

    glPushMatrix();
    glTranslated(_t[0], _t[1], _t[2]);
    if (theta > 0) glRotated(theta, wHat[0], wHat[1], wHat[2]);

    doDraw();

    glPopMatrix();
}

void Object::draw(Shader * shader)
{
    if (!_visible) return;

    float theta = glm::length(_w);
    glm::vec3 wHat;
    wHat = _w / theta;

    glPushMatrix();
    glTranslated(_t[0], _t[1], _t[2]);
    if (theta>0) glRotated(theta, wHat[0], wHat[1], wHat[2]);

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

Arrow::Arrow(const glm::vec3& origin, const glm::vec3& displacement) : Object() {
    _t = origin;
    _length = glm::length(displacement);
    if (displacement[0] == 0 && displacement[1] == 0) {
        _w = glm::vec3(0, 0, 0);
    }
    else {
        _w = (M_PI/2.0f)*glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), displacement/_length);
    }
}

void Arrow::doDraw() {
    float theta = glm::length(_w);
    glm::vec3 displacement;
    if (theta == 0) displacement = glm::vec3(0, 0, _length);
    else displacement = glm::rotate(glm::vec3(0, 0, _length), theta, _w / theta);
    glm::vec3 asdf = _t + displacement;

    GlutDraw::drawLine(_t, _t + displacement);

    float alpha = 1.0f / 16;
    float d = glm::length(displacement);
    GlutDraw::drawCone(_t + (1 - alpha)*displacement, d*alpha/2, alpha*displacement);

}

Cylinder::Cylinder(const glm::vec3& center, const glm::vec3& halfAxis, const float& radius) : Object() {
    _t = center;
    _r = radius;
    _h = 2 * glm::length(halfAxis);
    if (halfAxis[0] == 0 && halfAxis[1] == 0) {
        _w = glm::vec3(0, 0, 0);
    }
    else {
        _w = (M_PI / 2.0f)*glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), 2.0f*halfAxis/_h);
    }
}

void Cylinder::doDraw() {
    float theta = glm::length(_w);

    if (theta > 0) {
        glPushMatrix();
        glm::vec3 rotAxis = _w / theta;
        glRotatef((180.0f / M_PI)*theta, rotAxis[0], rotAxis[1], rotAxis[2]);
    }

    GlutDraw::drawCylinder(_t, glm::vec3(0, 0, _h / 2), _r);

    if (theta > 0) {
        glPopMatrix();
    }
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
        glm::vec3 pt = _parameterization((float)i / n);
        glVertex3f(pt[0], pt[1], pt[2]);
    }
    glEnd();
}

Body* Body::root() {
    Body* body = this;
    while (body->_parent != NULL) { body = body->_parent; }
    return body;
}

void Body::updateGlobalTransform() {
    std::vector<glm::mat3> Rs;
    std::vector<glm::vec3> ts;
    Body* body = this;
    Rs.push_back(rotationMatrix(_w));
    ts.push_back(_t);
    while (body->_parent != NULL) {
        body = body->_parent;
        Rs.push_back(rotationMatrix(body->_w));
        ts.push_back(_t);
    }
    glm::mat3 R = Rs.back();
    glm::vec3 t = ts.back();
    for (int i = Rs.size()-2; i > -1; i--) {
        t += R* ts[i];
        R = Rs[i] * R;
    }
    _body->setRotation(angleAxisVector(R));
    _body->setTranslation(t);
}

void Body::doDraw() {
    _body->doDraw();

    glm::vec3 trans = _body->translation();
    glm::vec3 rot = _body->rotation();
    float theta = glm::length(rot);
    rot /= theta;
    theta *= 180.0f / M_PI;

    glPushMatrix();

    glTranslatef(trans[0], trans[1], trans[2]);
    glRotatef(theta, rot[0], rot[1], rot[2]);

    if (_jointType == BALL) {
        GlutDraw::drawSphere(_t, 0.1);
    }
    if (_jointType == PIN) {
        //gl
        //GlutDraw::drawCylinder(_t,)
    }
    glPopMatrix();
}