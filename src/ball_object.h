//
// Created by 李梦凡 on 2019/10/21.
//

#ifndef BALL_OBJECT_H
#define BALL_OBJECT_H

#include <glad.h>
#include <glm.hpp>

#include "texture2d.h"
#include "sprite_renderer.h"
#include "game_object.h"

// BallObject holds the state of the Ball object inheriting
// relevant state data from GameObject. Contains some extra
// functionality specific to Breakout's ball object that
// were too specific for within GameObject alone.
class BallObject : public GameObject {
public:
    // Ball state
    GLfloat Radius;
    GLboolean Stuck;
    GLboolean Sticky, PassThrough;

    // Constructor(s)
    BallObject();

    BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite);

    // Moves the ball, keeping it constrained within the window bounds (except bottom edge); returns new position
    glm::vec2 Move(GLfloat dt, GLuint window_width);

    // Resets the ball to original state with given position and velocity
    void Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif
