#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

class Light
{
public:
    Light(GLfloat ambient, GLfloat diffuse, glm::vec4 color) :
        ambient(ambient),
        diffuse(diffuse),
        color(color)
    {}

    GLfloat* GetAmbient() { return &ambient; }
    GLfloat* GetDiffuse() { return &diffuse; }

    glm::vec4 color;
    GLfloat   ambient;        // ambient light 
    GLfloat   diffuse;        // diffuse light 
};