#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

class Light
{
public:
    Light(GLfloat ambient, GLfloat diffuse, glm::vec4 color) :
        ambientIntensity(ambient),
        diffuseIntensity(diffuse),
        color(color)
    {}

    GLfloat* GetAmbientIntensity() { return &ambientIntensity; }
    GLfloat* GetDiffuseIntensity() { return &diffuseIntensity; }

    glm::vec4 color;
    GLfloat   ambientIntensity;        // ambient light 
    GLfloat   diffuseIntensity;        // diffuse light 
};