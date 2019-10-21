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
    // 更新对象
    Ball->Move(dt, this->Width);
    // 检测碰撞
    this->DoCollisions();
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

// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);

GLboolean CheckCollision(BallObject &one, GameObject &two);

void Game::DoCollisions() {
    for (GameObject &box : this->Levels[this->Level].Bricks) {
        if (!box.Destroyed) {
            if (CheckCollision(*Ball, box)) {
                if (!box.IsSolid)
                    box.Destroyed = GL_TRUE;
            }
        }
    }
}

GLboolean CheckCollision(GameObject &one, GameObject &two) { // AABB - AABB collision
    // Collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                      two.Position.x + two.Size.x >= one.Position.x;
    // Collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                      two.Position.y + two.Size.y >= one.Position.y;
    // Collision only if on both axes
    return collisionX && collisionY;
}

GLboolean CheckCollision(BallObject &one, GameObject &two) { // AABB - Circle collision
    // 获取圆的中心
    glm::vec2 center(one.Position + one.Radius);
    // 计算AABB的信息（中心、半边长）
    glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
    glm::vec2 aabb_center(
            two.Position.x + aabb_half_extents.x,
            two.Position.y + aabb_half_extents.y
    );
    // 获取两个中心的差矢量
    glm::vec2 difference = center - aabb_center;
    // 限制运算：这个限制后矢量P¯就是AABB上距离圆最近的点。
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // AABB_center加上clamped这样就得到了碰撞箱上距离圆最近的点closest
    glm::vec2 closest = aabb_center + clamped;
    // 获得圆心center和最近点closest的矢量并判断是否 length <= radius
    difference = closest - center;
    return glm::length(difference) < one.Radius;
}
