//
// Created by 李梦凡 on 2019/10/20.
//

#ifndef GAME_H
#define GAME_H

#include <glad.h>
#include <glfw3.h>

#include "game_level.h"

#include <vector>
#include <tuple>

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Represents the four possible (collision) directions
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

// Defines a Collision typedef that represents collision data
typedef std::tuple<GLboolean, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

// 初始化挡板的大小
const glm::vec2 PLAYER_SIZE(100, 20);
// 初始化挡板的速率
const GLfloat PLAYER_VELOCITY(500.0f);

// 初始化球的速度
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// 球的半径
const GLfloat BALL_RADIUS = 12.5f;

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

    void DoCollisions();

    // Reset
    void ResetLevel();

    void ResetPlayer();
};

#endif
