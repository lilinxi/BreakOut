//
// Created by 李梦凡 on 2019/10/21.
//

#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include <glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "texture2d.h"
#include "shader.h"


class SpriteRenderer {
public:
    // Constructor (inits shaders/shapes)
    // add const to & in order to fix bug: candidate constructor not viable, expects a l-value for 1st argument
    SpriteRenderer(const Shader &shader);

    // Destructor
    ~SpriteRenderer();

    // Renders a defined quad textured with given sprite
    void
    // add const to & in order to fix bug: candidate constructor not viable, expects a l-value for 1st argument
    DrawSprite(const Texture2D &texture, glm::vec2 position, glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f,
               glm::vec3 color = glm::vec3(1.0f));

private:
    // Render state
    Shader shader;
    GLuint quadVAO;

    // Initializes and configures the quad's buffer and vertex attributes
    void initRenderData();
};

#endif
