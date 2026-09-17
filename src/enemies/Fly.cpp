#include "enemies/Fly.hpp"
#include <cmath>

Fly::Fly()
    : moveSpeed(50.f),
    detectionRange(200.f),
    verticalSpeed(50.f)

{
    velocity.x = 0.f;
    velocity.y = 0.f;
}

void Fly::setTexture(
    const sf::Texture& fly,
    const sf::Texture& death
)
{
    walkAnimation.setTexture(fly);
    walkAnimation.setFrames(32, 32, 6, 0.07f);

    deathAnimation.setTexture(death);
    deathAnimation.setFrames(32, 32, 3, 0.15f);

    sprite.setTexture(
        walkAnimation.getTexture()
    );

    sprite.setTextureRect(
        walkAnimation.getTextureRect()
    );

    sprite.setOrigin(16.f, 32.f);
}

void Fly::update(
    float dt,
    const sf::Vector2f& playerPosition
)
{
    previousPosition = shape.getPosition();
    updateHitFlash(dt);

    if (updateDeath(dt))
        return;
    float dx = playerPosition.x - getPosition().x;
    float dy = playerPosition.y - getPosition().y;

    float distance = std::sqrt(dx * dx + dy * dy);
    if (distance <= detectionRange)
    {
        if (distance > 0.f)
        {
            sf::Vector2f direction(
                dx / distance,
                dy / distance
            );

            velocity = direction * moveSpeed;

            if (dx > 0.f)
                facingRight = true;
            else if (dx < 0.f)
                facingRight = false;
        }
        else
        {
            velocity = sf::Vector2f(0.f, 0.f);
        }
    }
    else
    {
        velocity = sf::Vector2f(0.f, 0.f);
    }

    walkAnimation.update(dt);

    sprite.setTextureRect(
        walkAnimation.getTextureRect()
    );

    if (!updateKnockback(dt))
    {
        shape.move(
            velocity.x * dt,
            velocity.y * dt
        );
    }

    updateSpritePosition();
}

void Fly::turnAround()
{
    velocity.x = -velocity.x;
    facingRight = !facingRight;
}

bool Fly::usesGravity() const
{
    return false;
}

