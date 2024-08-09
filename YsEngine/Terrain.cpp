#include "Terrain.h"

#include <vector>
#include <iostream>

#include "stb_image.h"
#include "CommonValues.h"

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

void Terrain::LoadTerrain(const char* fileName)
{
    // shader setup
    terrainShader = new Shader();
    terrainShader->CreateFromFiles("Shaders/terrainVertex.glsl", "Shaders/terrainFragment.glsl", nullptr,
                                "Shaders/terrainTC.glsl", "Shaders/terrainTE.glsl");
    // -------------------------------------------------------------------------
    // height map 로드
    heightMap = new Texture(fileName);
    heightMap->LoadTexture(4);
    // -------------------------------------------------------------------------
    // diffuse map 로드    
    diffuseMap = new Texture("Textures/aerial_grass_rock_4k.blend/aerial_grass_rock_diff_4k.jpg");
    diffuseMap->LoadTexture(4);
    // -------------------------------------------------------------------------

    // vertex generation
    rez = 20;
    width = heightMap->GetWidth();
    height = heightMap->GetHeight();
    for (unsigned i = 0; i < rez; i++)
    {
        for (unsigned j = 0; j < rez; j++)
        {
            Vertex vertexA;
            vertexA.Position.x = -width / 2.0f + width * i / (float)rez;
            vertexA.Position.y = 0.f;
            vertexA.Position.z = -height / 2.0f + height * j / (float)rez;
            vertexA.TexCoords.x = i / (float)rez;
            vertexA.TexCoords.y = j / (float)rez;
            vertices.push_back(vertexA);

            Vertex vertexB;
            vertexB.Position.x = -width / 2.0f + width * (i + 1) / (float)rez;
            vertexB.Position.y = 0.f;
            vertexB.Position.z = -height / 2.0f + height * j / (float)rez;
            vertexB.TexCoords.x = (i + 1) / (float)rez;
            vertexB.TexCoords.y = j / (float)rez;
            vertices.push_back(vertexB);

            Vertex vertexC;
            vertexC.Position.x = -width / 2.0f + width * i / (float)rez;
            vertexC.Position.y = 0.f;
            vertexC.Position.z = -height / 2.0f + height * (j + 1) / (float)rez;
            vertexC.TexCoords.x = i / (float)rez;
            vertexC.TexCoords.y = (j + 1) / (float)rez;
            vertices.push_back(vertexC);

            Vertex vertexD;
            vertexD.Position.x = -width / 2.0f + width * (i + 1) / (float)rez;
            vertexD.Position.y = 0.f;
            vertexD.Position.z = -height / 2.0f + height * (j + 1) / (float)rez;
            vertexD.TexCoords.x = (i + 1) / (float)rez;
            vertexD.TexCoords.y = (j + 1) / (float)rez;
            vertices.push_back(vertexD);
        }
    }

    std::cout << "Loaded " << rez * rez << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << rez * rez * 4 << " vertices in vertex shader" << std::endl;

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after generating vertices: " << error << std::endl;
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);

    // texture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
}

void Terrain::DrawTerrain(glm::mat4 viewMat, glm::mat4 projMat)
{
    terrainShader->UseShader();

    heightMap->UseTexture(GL_TEXTURE0);
    terrainShader->setInt("heightSampler", 0);

    diffuseMap->UseTexture(GL_TEXTURE1);
    terrainShader->setInt("diffuseSampler", 1);
    
    glm::mat4 PVM = projMat * viewMat;
    loc_PVM = terrainShader->GetPVMLoc();
    glUniformMatrix4fv(loc_PVM, 1, GL_FALSE, glm::value_ptr(PVM));
    terrainShader->setMat4("modelViewMat", viewMat);

    glm::vec2 texelSize(1.f/width, 1.f/height);
    terrainShader->setVec2("texelSize", texelSize);

    terrainShader->setFloat("HEIGHT_SCALE", 64.f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_PATCHES, 0, 4*rez*rez);

    glBindVertexArray(0);
}

void Terrain::UseShader()
{
    terrainShader->UseShader();
}

Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
