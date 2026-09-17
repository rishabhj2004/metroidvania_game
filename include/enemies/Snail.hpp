#pragma once

#include "Enemy.hpp"

class Snail : public Enemy
{
private:
    float moveSpeed;
    Animation walkAnimation;

public:
    Snail();

    void setTexture(
        const sf::Texture& walk,
        const sf::Texture& death
    );

    void update(
        float dt,
        const sf::Vector2f& playerPosition
    ) override;

    void turnAround() override;
    bool usesGravity() const override;
};
