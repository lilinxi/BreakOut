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
#include "text_renderer.h"

#include "irrKlang.h"

using namespace irrklang;

#include <sstream>

// Game-related State data
SpriteRenderer *Renderer;

GameObject *Player;

BallObject *Ball;

ParticleGenerator *Particles;

PostProcessor *Effects;

GLfloat ShakeTime = 0.0f;

TextRenderer *Text;

ISoundEngine *SoundEngine = createIrrKlangDevice();

Game::Game(GLuint width, GLuint height)
        : State(GAME_MENU), Keys(), Width(width), Height(height), Lives(3) {

}

Game::~Game() {
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Effects;
    delete Text;
    SoundEngine->drop();
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
    ResourceManager::LoadTexture("../res/textures/powerup_speed.png", GL_TRUE, "powerup_speed");
    ResourceManager::LoadTexture("../res/textures/powerup_sticky.png", GL_TRUE, "powerup_sticky");
    ResourceManager::LoadTexture("../res/textures/powerup_increase.png", GL_TRUE, "powerup_increase");
    ResourceManager::LoadTexture("../res/textures/powerup_confuse.png", GL_TRUE, "powerup_confuse");
    ResourceManager::LoadTexture("../res/textures/powerup_chaos.png", GL_TRUE, "powerup_chaos");
    ResourceManager::LoadTexture("../res/textures/powerup_passthrough.png", GL_TRUE, "powerup_passthrough");
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"),
                                      1000);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width * 2, this->Height * 2);
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("../res/fonts/OCRAEXT.TTF", 24);
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
    // Audio
    SoundEngine->play2D("../res/audio/breakout.wav", GL_TRUE);
}

void Game::Update(GLfloat dt) {
    // 更新对象
    Ball->Move(dt, this->Width);
    // 检测碰撞
    this->DoCollisions();
    // Update PowerUps
    this->UpdatePowerUps(dt);
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
        --this->Lives;
        // 玩家是否已失去所有生命值? : 游戏结束
        if (this->Lives == 0) {
            this->ResetLevel();
            this->State = GAME_MENU;
        }
        this->ResetPlayer();
    }
    // 检测成功
    if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted()) {
        this->ResetLevel();
        this->ResetPlayer();
        Effects->Chaos = GL_TRUE;
        this->State = GAME_WIN;
    }
}

void Game::ProcessInput(GLfloat dt) {
    if (this->State == GAME_MENU) {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) {
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
        }
        if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W]) {
            this->Level = (this->Level + 1) % 4;
            this->KeysProcessed[GLFW_KEY_W] = GL_TRUE;
        }
        if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S]) {
            if (this->Level > 0) {
                --this->Level;
            } else {
                this->Level = 3;
            }
            this->KeysProcessed[GLFW_KEY_S] = GL_TRUE;
        }
    }
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
    if (this->State == GAME_WIN) {
        if (this->Keys[GLFW_KEY_ENTER]) {
            this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
            Effects->Chaos = GL_FALSE;
            this->State = GAME_MENU;
        }
    }
}

void Game::Render() {
    if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN) {
        Effects->BeginRender();
        // 绘制背景
        Renderer->DrawSprite(ResourceManager::GetTexture("background"),
                             glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
        );
        // 绘制关卡
        this->Levels[this->Level].Draw(*Renderer);
        // 绘制挡板
        Player->Draw(*Renderer);
        // Draw PowerUps
        for (PowerUp &powerUp : this->PowerUps) {
            if (!powerUp.Destroyed) {
                powerUp.Draw(*Renderer);
            }
        }
        // 绘制粒子
        Particles->Draw();
        // 绘制球
        Ball->Draw(*Renderer);
        Effects->EndRender();
        Effects->Render(glfwGetTime());
        // 绘制生命值
        std::stringstream ss;
        ss << this->Lives;
        Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
    }
    if (this->State == GAME_MENU) {
        Text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);
    }
    if (this->State == GAME_WIN) {
        Text->RenderText(
                "You WON!!!", 320.0, Height / 2 - 20.0, 1.0, glm::vec3(0.0, 1.0, 0.0)
        );
        Text->RenderText(
                "Press ENTER to retry or ESC to quit", 130.0, Height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0)
        );
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
    this->Lives = 3;
}

void Game::ResetPlayer() {
    // Reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(static_cast<float>(this->Width) / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)),
                INITIAL_BALL_VELOCITY);
    // Also disable all active powerups
    Effects->Chaos = Effects->Confuse = GL_FALSE;
    Ball->PassThrough = Ball->Sticky = GL_FALSE;
    Player->Color = glm::vec3(1.0f);
    Ball->Color = glm::vec3(1.0f);
}

// PowerUps
GLboolean IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type) {
    // Check if another PowerUp of the same type is still active
    // in which case we don't disable its effect (yet)
    for (const PowerUp &powerUp : powerUps) {
        if (powerUp.Activated) {
            if (powerUp.Type == type) {
                return GL_TRUE;
            }
        }
    }
    return GL_FALSE;
}

void Game::UpdatePowerUps(GLfloat dt) {
    for (PowerUp &powerUp : this->PowerUps) {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated) {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f) {
                // Remove powerup from list (will later be removed)
                powerUp.Activated = GL_FALSE;
                // Deactivate effects
                if (powerUp.Type == "sticky") {
                    if (!IsOtherPowerUpActive(this->PowerUps,
                                              "sticky")) {    // Only reset if no other PowerUp of type sticky is active
                        Ball->Sticky = GL_FALSE;
                        Player->Color = glm::vec3(1.0f);
                    }
                } else if (powerUp.Type == "pass-through") {
                    if (!IsOtherPowerUpActive(this->PowerUps,
                                              "pass-through")) {    // Only reset if no other PowerUp of type pass-through is active
                        Ball->PassThrough = GL_FALSE;
                        Ball->Color = glm::vec3(1.0f);
                    }
                } else if (powerUp.Type == "confuse") {
                    if (!IsOtherPowerUpActive(this->PowerUps,
                                              "confuse")) {    // Only reset if no other PowerUp of type confuse is active
                        Effects->Confuse = GL_FALSE;
                    }
                } else if (powerUp.Type == "chaos") {
                    if (!IsOtherPowerUpActive(this->PowerUps,
                                              "chaos")) {    // Only reset if no other PowerUp of type chaos is active
                        Effects->Chaos = GL_FALSE;
                    }
                }
            }
        }
    }
    // Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
    // Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
    // remove_if()将所有应该移除的元素都移动到了容器尾部并返回一个分界的迭代器。移除的所有元素仍然可以通过返回的迭代器访问到。为了实际移除元素。你必须对容器自行调用erase()以擦除需要移除的元素。
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
                                        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());
}

GLboolean ShouldSpawn(GLuint chance) {
    GLuint random = rand() % chance;
    return random == 0;
}

void Game::SpawnPowerUps(GameObject &block) {
    if (ShouldSpawn(75)) { // 1 in 75 chance
        this->PowerUps.emplace_back("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position,
                                    ResourceManager::GetTexture("powerup_speed"));
    }
    if (ShouldSpawn(75)) {
        this->PowerUps.emplace_back("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position,
                                    ResourceManager::GetTexture("powerup_sticky"));
    }
    if (ShouldSpawn(75)) {
        this->PowerUps.emplace_back("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position,
                                    ResourceManager::GetTexture("powerup_passthrough"));
    }
    if (ShouldSpawn(75)) {
        this->PowerUps.emplace_back("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position,
                                    ResourceManager::GetTexture("powerup_increase"));
    }
    if (ShouldSpawn(15)) { // Negative powerups should spawn more often
        this->PowerUps.emplace_back("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position,
                                    ResourceManager::GetTexture("powerup_confuse"));
    }
    if (ShouldSpawn(15)) {
        this->PowerUps.emplace_back("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position,
                                    ResourceManager::GetTexture("powerup_chaos"));
    }
}

void ActivatePowerUp(PowerUp &powerUp) {
    // 根据道具类型发动道具
    if (powerUp.Type == "speed") {
        Ball->Velocity *= 1.2;
    } else if (powerUp.Type == "sticky") {
        Ball->Sticky = GL_TRUE;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    } else if (powerUp.Type == "pass-through") {
        Ball->PassThrough = GL_TRUE;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    } else if (powerUp.Type == "pad-size-increase") {
        Player->Size.x += 50;
    } else if (powerUp.Type == "confuse") {
        if (!Effects->Chaos) {
            Effects->Confuse = GL_TRUE; // 只在chaos未激活时生效，chaos同理
        }
    } else if (powerUp.Type == "chaos") {
        if (!Effects->Confuse) {
            Effects->Chaos = GL_TRUE;
        }
    }
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
                    this->SpawnPowerUps(box);

                    SoundEngine->play2D("../res/audio/bleep1.wav", GL_FALSE);
                } else {   // if block is solid, enable shake effect
                    ShakeTime = 0.05f;
                    Effects->Shake = GL_TRUE;

                    SoundEngine->play2D("../res/audio/solid.wav", GL_FALSE);
                }
                // 碰撞处理
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                // PassThrough 效果
                if (!(Ball->PassThrough &&
                      !box.IsSolid)) { // don't do collision resolution on non-solid bricks if pass-through activated
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
        // Sticky效果
        Ball->Stuck = Ball->Sticky;

        SoundEngine->play2D("../res/audio/bleep.wav", GL_FALSE);
    }
    for (PowerUp &powerUp : this->PowerUps) {
        if (!powerUp.Destroyed) {
            if (powerUp.Position.y >= this->Height) {
                powerUp.Destroyed = GL_TRUE;
            }
            if (CheckCollision(*Player, powerUp)) { // 道具与挡板接触，激活它！
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = GL_TRUE;
                powerUp.Activated = GL_TRUE;

                SoundEngine->play2D("../res/audio/powerup.wav", GL_FALSE);
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