//
// Created by 李梦凡 on 2019/10/20.
//

#ifndef GAME_H
#define GAME_H

#include <glad.h>
#include <glfw3.h>

#include "game_level.h"

#include <vector>

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// 初始化挡板的大小
const glm::vec2 PLAYER_SIZE(100, 20);
// 初始化当班的速率
const GLfloat PLAYER_VELOCITY(500.0f);

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {
public:
    // Game state
    GameState State;
    GLboolean Keys[1024];
    GLuint Width, Height;

    std::vector<GameLevel> Levels;
    GLuint Level;

    // Constructor/Destructor
    Game(GLuint width, GLuint height);

    ~Game();

    // Initialize game state (load all shaders/textures/levels)
    void Init();

    // GameLoop
    void ProcessInput(GLfloat dt);

    void Update(GLfloat dt);

    void Render();
};

#endif
