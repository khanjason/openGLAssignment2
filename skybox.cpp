#include "terrain_object.h"
#include <glm/gtc/noise.hpp>
#include "glm/gtc/random.hpp"
#include <stdio.h>
#include <iostream>

#include "stb_image.h"
#include "skybox.h"
#include <glm\ext\matrix_transform.hpp>
//class for skybox 
//Jason Khan		//adapted from https://learnopengl.com/Advanced-OpenGL/Cubemaps
using namespace std;
using namespace glm;

Skybox::Skybox()
{
    
    VAO = 0;
    VBO = 0;


}

Skybox::~Skybox()
{
}
//initiate buffers function before draw
void Skybox::make() {
    GLfloat skyboxVertices[108] =
    {
        // positions          
        -10.0f,  10.0f, -10.0f,
        -10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,

        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,

        -10.0f, -10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,

        -10.0f,  10.0f, -10.0f,
        10.0f,  10.0f, -10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
        10.0f, -10.0f,  10.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //https://assetstore.unity.com/packages/2d/textures-materials/sky/spaceskies-free-80503
    std::vector<std::string> faces
    {
        "../../Images/left2.png",
        "../../Images/right2.png",
        "../../Images/up2.png",
        "../../Images/down2.png",
        "../../Images/front2.png",
        "../../Images/back2.png"
    };

    cubemapTexture = loadCubemap(faces);
}

void Skybox::Draw(const glm::mat4 view, const glm::mat4 projection,GLuint program)
{
    glDepthMask(GL_FALSE);
    glDepthRange(1.0f, 1.0f);
    glDepthFunc(GL_LEQUAL);
    
    glUseProgram(program);
    
    GLuint viewID;
    GLuint projectionID;
    viewID = glGetUniformLocation(program, "view");
    projectionID= glGetUniformLocation(program, "projection");
    glm::mat4 skyview=glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(viewID, 1, GL_FALSE, &skyview[0][0]);
    glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
    
    // skybox cube
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); 
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
}

GLuint Skybox::loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        int pixel_format = 0;
        if (nrChannels == 3)
            pixel_format = GL_RGB;
        else
            pixel_format = GL_RGBA;

        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            printf("Stb can't load this : %s\n", faces[i].c_str());
            stbi_image_free(data);
        }
    }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    
 
    return textureID;
}