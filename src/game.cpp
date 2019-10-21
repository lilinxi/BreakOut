//
// Created by 李梦凡 on 2019/10/20.
//

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"

// Game-related State data
SpriteRenderer *Renderer;

GameObject *Player;

BallObject *Ball;

ParticleGenerator *Particles;

PostProcessor *Effects;

GLfloat ShakeTime = 0.0f;

Game::Game(GLuint width, GLuint height)
        : State(GAME_ACTIVE), Keys(), Width(width), Height(height) {

}

Game::~Game() {
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Effects;
}

void Game::Init() {
    // Load shaders
    ResourceManager::LoadShader("../src/shaders/sprite_v.glsl", "../src/shaders/sprite_f.glsl", nullptr, "sprite");
    ResourceManager::LoadShader("../src/shaders/particle_v.glsl", "../src/shaders/particle_f.glsl", nullptr,
                                "particle");
    ResourceManager::LoadShader("../src/shaders/post_processing_v.glsl", "../src/shaders/post_processing_f.glsl",
                                nullptr, "postprocessing");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f,
                                      -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4f("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4f("projection", projection);
    // 加载纹理
    ResourceManager::LoadTexture("../res/textures/background.jpg", GL_FALSE, "background");
    ResourceManager::LoadTexture("../res/textures/awesomeface.png", GL_TRUE, "face");
    ResourceManager::LoadTexture("../res/textures/block.png", GL_FALSE, "block");
    ResourceManager::LoadTexture("../res/textures/block_solid.png", GL_FALSE, "block_solid");
    ResourceManager::LoadTexture("../res/textures/paddle.png", GL_TRUE, "paddle");
    ResourceManager::LoadTexture("../res/textures/particle.png", GL_TRUE, "particle");
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"),
                                      1000);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
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

//    Effects->Shake = GL_TRUE;
//    Effects->Confuse = GL_TRUE;
//    Effects->Chaos = GL_TRUE;
}

void Game::Update(GLfloat dt) {
    // 更新对象
    Ball->Move(dt, this->Width);
    // 检测碰撞
    this->DoCollisions();
    // 更新粒子
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2));
    // Reduce shake time
    if (ShakeTime > 0.0f) {
        ShakeTime -= dt;
        if (ShakeTime <= 0.0f) {
            Effects->Shake = GL_FALSE;
        }
    }
    // 检测失败
    if (Ball->Position.y >= this->Height) { // 球是否接触底部边界？
        this->ResetLevel();
        this->ResetPlayer();
    }
}

void Game::ProcessInput(GLfloat dt) {
    if (this->State == GAME_ACTIVE) {
        // 平衡速率
        GLfloat velocity = PLAYER_VELOCITY * dt;
        // 移动挡板
        if (this->Keys[GLFW_KEY_A]) {
            if (Player->Position.x >= 0) {
                Player->Position.x -= velocity;
                if (Ball->Stuck) {
                    Ball->Position.x -= velocity;
                }
            }
        }
        if (this->Keys[GLFW_KEY_D]) {
            if (Player->Position.x <= this->Width - Player->Size.x) {
                Player->Position.x += velocity;
                if (Ball->Stuck) {
                    Ball->Position.x += velocity;
                }
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
//        Effects->BeginRender();
        // 绘制背景
        Renderer->DrawSprite(ResourceManager::GetTexture("background"),
                             glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
        );
        // 绘制关卡
        this->Levels[this->Level].Draw(*Renderer);
        // 绘制挡板
        Player->Draw(*Renderer);
        // 绘制粒子
        Particles->Draw();
        // 绘制球
        Ball->Draw(*Renderer);
//        Effects->EndRender();
//        Effects->Render(glfwGetTime());
    }
}

void Game::ResetLevel() {
    if (this->Level == 0) {
        this->Levels[0].Load("../res/levels/1.lvl", this->Width, this->Height * 0.5f);
    } else if (this->Level == 1) {
        this->Levels[1].Load("../res/levels/2.lvl", this->Width, this->Height * 0.5f);
    } else if (this->Level == 2) {
        this->Levels[2].Load("../res/levels/3.lvl", this->Width, this->Height * 0.5f);
    } else if (this->Level == 3) {
        this->Levels[3].Load("../res/levels/4.lvl", this->Width, this->Height * 0.5f);
    }
}

void Game::ResetPlayer() {
    // Reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(static_cast<float>(this->Width) / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)),
                INITIAL_BALL_VELOCITY);
}

// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);

Collision CheckCollision(BallObject &one, GameObject &two);

Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions() {
    for (GameObject &box : this->Levels[this->Level].Bricks) {
        if (!box.Destroyed) {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) { // 如果collision 是 true
                // 如果砖块不是实心就销毁砖块
                if (!box.IsSolid) {
                    box.Destroyed = GL_TRUE;
                } else {   // if block is solid, enable shake effect
                    ShakeTime = 0.05f;
                    Effects->Shake = GL_TRUE;
                }
                // 碰撞处理
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT) { // 水平方向碰撞
                    Ball->Velocity.x = -Ball->Velocity.x; // 反转水平速度
                    // 重定位
                    GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
                    if (dir == LEFT) {
                        Ball->Position.x += penetration; // 将球右移
                    } else {
                        Ball->Position.x -= penetration; // 将球左移
                    }
                } else { // 垂直方向碰撞
                    Ball->Velocity.y = -Ball->Velocity.y; // 反转垂直速度
                    // 重定位
                    GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
                    if (dir == UP) {
                        Ball->Position.y -= penetration; // 将球上移
                    } else {
                        Ball->Position.y += penetration; // 将球下移
                    }
                }
            }
        }
    }
    Collision result = CheckCollision(*Ball, *Player);
    // 基于撞击挡板的点与（挡板）中心的距离来改变球的水平速度。撞击点距离挡板的中心点越远，则水平方向的速度就会越大。
    if (!Ball->Stuck && std::get<0>(result)) {
        // 检查碰到了挡板的哪个位置，并根据碰到哪个位置来改变速度
        GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
        GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        GLfloat percentage = distance / (Player->Size.x / 2);
        // 依据结果移动
        GLfloat strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        // Ball->Velocity.y = -Ball->Velocity.y;
        // 我们总是简单地返回正的y速度而不是反转y速度，这样当它被卡住时也可以立即脱离。
        Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
        // 速率不变
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
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

Collision CheckCollision(BallObject &one, GameObject &two) { // AABB - Circle collision
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
    if (glm::length(difference) <
        one.Radius) { // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
    } else {
        return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
    }
}

// Calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target) {
    glm::vec2 compass[] = {
            glm::vec2(0.0f, 1.0f),    // up
            glm::vec2(1.0f, 0.0f),    // right
            glm::vec2(0.0f, -1.0f),    // down
            glm::vec2(-1.0f, 0.0f)    // left
    };
    GLfloat max = 0.0f;
    GLuint best_match = -1;
    for (GLuint i = 0; i < 4; i++) {
        GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max) {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction) best_match;
}