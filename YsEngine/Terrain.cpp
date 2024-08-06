#include "Terrain.h"

#include <vector>
#include <iostream>

#include "stb_image.h"
#include "CommonValues.h"

#include "Mesh.h"
#include "Shader.h"

Terrain::Terrain()
{
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    terrainShader = nullptr;
    loc_PVM = 0;
    width = 0;
    height = 0;
    nChannels = 0;
    VAO = 0;
    VBO = 0;
    IBO = 0;
    NUM_STRIPS = 0;
    NUM_VERTS_PER_STRIP = 0;
    rez = 0;
}

void Terrain::LoadTerrain(const char* fileName)
{
    // shader setup
    terrainShader = new Shader();
    terrainShader->CreateFromFiles("Shaders/terrainVertex.glsl", "Shaders/terrainFragment.glsl");
    loc_PVM = terrainShader->GetPVMLoc();

    unsigned char* data = stbi_load(fileName, &width, &height, &nChannels, 0);

    if (!data)
    {
        std::cerr << "Failed to load image!" << std::endl;
        return;
    }

    std::cout << "width : " << width << std::endl;
    std::cout << "height : " << height << std::endl;
    std::cout << "nChannels : " << nChannels << std::endl;

    /*
    // vertex generation
    float yScale = 64.0f / 256.0f, yShift = 16.0f;  // apply a scale+shift to the height data
    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            // retrieve texel for (i,j) tex coord
            unsigned char* texel = data + (j + width * i) * nChannels;
            // raw height at coordinate
            unsigned char y = texel[0];

            // vertex
            float px = (-height / 2.0f + i);        // v.x
            float py = (y * yScale - yShift); // v.y
            float pz = (-width / 2.0f + j);        // v.z
            Vertex vertex(px, py, pz);

            vertices.push_back(vertex);
        }
    }*/
    
    // vertex generation
    std::vector<Vertex> vertices;
    rez = 20;
    for (unsigned i = 0; i <= rez - 1; i++)
    {
        for (unsigned j = 0; j <= rez - 1; j++)
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

    // index generation
    for (unsigned int i = 0; i < height - 1; i++)       // for each row a.k.a. each strip
    {
        for (unsigned int j = 0; j < width; j++)      // for each column
        {
            for (unsigned int k = 0; k < 2; k++)      // for each side of the strip
            {
                indices.push_back(j + width * (i + k));
            }
        }
    }

    std::cout << "Loaded " << vertices.size() << " vertices" << std::endl;
    std::cout << "Loaded " << indices.size() << " indices" << std::endl;

    stbi_image_free(data);

    NUM_STRIPS = height - 1;
    NUM_VERTS_PER_STRIP = width * 2;

    std::cout << "NUM_STRIPS : " << NUM_STRIPS << std::endl;
    std::cout << "NUM_VERTS_PER_STRIP : " << NUM_VERTS_PER_STRIP << std::endl;

    CreateTerrain();
}

void Terrain::CreateTerrain()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after creating terrain: " << error << std::endl;
    }
}

void Terrain::DrawTerrain(glm::mat4 viewMat, glm::mat4 projMat)
{
    GLenum error;

    terrainShader->UseShader();

    loc_PVM = terrainShader->GetPVMLoc();
    
    glm::mat4 PVM = projMat * viewMat;
    glUniformMatrix4fv(loc_PVM, 1, GL_FALSE, glm::value_ptr(PVM));


    glBindVertexArray(VAO);
    /*
    for (unsigned int strip = 0; strip < NUM_STRIPS; ++strip)
    {
        glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
            NUM_VERTS_PER_STRIP,            // number of indices to render
            GL_UNSIGNED_INT,                // index data type
            (void*)(sizeof(unsigned int) * NUM_VERTS_PER_STRIP * strip)); // offset to starting index
    }*/
    glDrawArrays(GL_PATCHES, 0, 4*rez*rez);

    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after drawing elements: " << error << std::endl;
        return;
    }

    glBindVertexArray(0);
}


Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}
