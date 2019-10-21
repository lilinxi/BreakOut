//
// Created by 李梦凡 on 2019/10/20.
//

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"

// Game-related State data
SpriteRenderer  *Renderer;

Game::Game(GLuint width, GLuint height)
        : State(GAME_ACTIVE), Keys(), Width(width), Height(height) {

}

Game::~Game() {
    delete Renderer;
}

void Game::Init() {
    // Load shaders
    ResourceManager::LoadShader("../src/shaders/sprite_v.glsl", "../src/shaders/sprite_f.glsl", nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f,
                                      -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4f("projection", projection);
    // Load textures
    ResourceManager::LoadTexture("../res/textures/awesomeface.png", GL_TRUE, "face");
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
}

void Game::Update(GLfloat dt) {

}

void Game::ProcessInput(GLfloat dt) {

}

void Game::Render() {
    Renderer->DrawSprite(ResourceManager::GetTexture("face"), glm::vec2(200, 200), glm::vec2(300, 400), 45.0f,
                         glm::vec3(0.0f, 1.0f, 0.0f));
}
