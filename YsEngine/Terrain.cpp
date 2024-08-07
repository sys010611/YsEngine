#include "Terrain.h"

#include <vector>
#include <iostream>

#include "stb_image.h"
#include "CommonValues.h"

#include "Mesh.h"
#include "Shader.h"

void Terrain::LoadTerrain(const char* fileName)
{
    // shader setup
    terrainShader = new Shader();
    terrainShader->CreateFromFiles("Shaders/terrainVertex.glsl",  "Shaders/terrainTC.glsl",
                                "Shaders/terrainTE.glsl", "Shaders/terrainFragment.glsl");

    // -------------------------------------------------------------------------
    int width, height, nrChannels;
    unsigned char* data = stbi_load(fileName, &width, &height, &nrChannels, 0);
    if(!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        return;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    // -------------------------------------------------------------------------

    // vertex generation
    rez = 20;
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


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glm::mat4 PVM = projMat * viewMat;
    loc_PVM = terrainShader->GetPVMLoc();
    glUniformMatrix4fv(loc_PVM, 1, GL_FALSE, glm::value_ptr(PVM));
    terrainShader->setInt("heightSampler", 0);
    terrainShader->setMat4("modelViewMat", viewMat);

    glBindVertexArray(VAO);
    glDrawArrays(GL_PATCHES, 0, 4*rez*rez);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after drawing terrain: " << error << std::endl;
        return;
    }

    glBindVertexArray(0);
}

Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
