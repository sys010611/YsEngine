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

    glm::vec4 color;
    GLfloat   ambient;        // ambient light 
    GLfloat   diffuse;        // diffuse light 
};