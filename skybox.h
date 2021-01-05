#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class Skybox
{
public:
    Skybox();
    ~Skybox();

    void Draw(const glm::mat4 view, const glm::mat4 projection, GLuint program);
    void make();

private:
    unsigned int cubemapTexture;

    GLuint VAO;
    GLuint VBO;

  

    GLuint loadCubemap(std::vector<std::string> faces);
};