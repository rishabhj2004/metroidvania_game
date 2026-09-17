#pragma once
#include "Enemy.hpp"
#include "Leveltmx.hpp"

class Fly : public Enemy
{
    private:
        float moveSpeed;
        float verticalSpeed;
        float detectionRange;
        Animation walkAnimation;
        void resolveVerticalCollision(const Leveltmx& level);
    public:
        Fly();
        void setTexture(const sf::Texture& fly,
                const sf::Texture& death
                );
        void update(
            float dt,
            const sf::Vector2f& playerPosition
        ) override;
        void turnAround() override;
        bool usesGravity() const override;
};



