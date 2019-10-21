//
// Created by 李梦凡 on 2019/10/20.
//

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "ball_object.h"

// Game-related State data
SpriteRenderer *Renderer;

GameObject *Player;

BallObject *Ball;

Game::Game(GLuint width, GLuint height)
        : State(GAME_ACTIVE), Keys(), Width(width), Height(height) {

}

Game::~Game() {
    delete Renderer;
    delete Player;
    delete Ball;
}

void Game::Init() {
    // Load shaders
    ResourceManager::LoadShader("../src/shaders/sprite_v.glsl", "../src/shaders/sprite_f.glsl", nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f,
                                      -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4f("projection", projection);
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // 加载纹理
    ResourceManager::LoadTexture("../res/textures/background.jpg", GL_FALSE, "background");
    ResourceManager::LoadTexture("../res/textures/awesomeface.png", GL_TRUE, "face");
    ResourceManager::LoadTexture("../res/textures/block.png", GL_FALSE, "block");
    ResourceManager::LoadTexture("../res/textures/block_solid.png", GL_FALSE, "block_solid");
    ResourceManager::LoadTexture("../res/textures/paddle.png", true, "paddle");
    // 加载关卡
    GameLevel one;
    one.Load("../res/levels/1.lvl", this->Width, this->Height * 0.5);
    GameLevel two;
    two.Load("../res/levels/2.lvl", this->Width, this->Height * 0.5);
    GameLevel three;
    three.Load("../res/levels/3.lvl", this->Width, this->Height * 0.5);
    GameLevel four;
    four.Load("../res/levels/4.lvl", this->Width, this->Height * 0.5);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;
    // 加载挡板
    glm::vec2 playerPos = glm::vec2(
            static_cast<float>(this->Width) / 2 - PLAYER_SIZE.x / 2,
            this->Height - PLAYER_SIZE.y
    );
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
                          ResourceManager::GetTexture("face"));
}

void Game::Update(GLfloat dt) {
    Ball->Move(dt, this->Width);
}

void Game::ProcessInput(GLfloat dt) {
    if (this->State == GAME_ACTIVE) {
        // 平衡速率
        GLfloat velocity = PLAYER_VELOCITY * dt;
        // 移动挡板
        if (this->Keys[GLFW_KEY_A]) {
            if (Player->Position.x >= 0) {
                Player->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D]) {
            if (Player->Position.x <= this->Width - Player->Size.x) {
                Player->Position.x += velocity;
            }
        }
        // 释放球
        if (this->Keys[GLFW_KEY_SPACE]) {
            Ball->Stuck = false;
        }
    }
}

void Game::Render() {
    if (this->State == GAME_ACTIVE) {
        // 绘制背景
        Renderer->DrawSprite(ResourceManager::GetTexture("background"),
                             glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
        );
        // 绘制关卡
        this->Levels[this->Level].Draw(*Renderer);
        // 绘制挡板
        Player->Draw(*Renderer);
        // 绘制球
        Ball->Draw(*Renderer);
    }
}
