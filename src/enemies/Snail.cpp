#include "enemies/Snail.hpp"

Snail::Snail()
    : moveSpeed(50.f)
{
    velocity.x = moveSpeed;

    shape.setSize(sf::Vector2f(25.f, 20.f));
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(100.f, 100.f);

    previousPosition = shape.getPosition();
}

void Snail::setTexture(
    const sf::Texture& walk,
    const sf::Texture& death
)
{
    walkAnimation.setTexture(walk);
    walkAnimation.setFrames(32, 32, 3, 0.15f);

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

void Snail::turnAround()
{
    facingRight = !facingRight;

    velocity.x =
        facingRight ? moveSpeed : -moveSpeed;
}

void Snail::update(float dt,
        const sf::Vector2f& playerPosition
        )
{
    previousPosition = shape.getPosition();


    updateHitFlash(dt);

    if (updateDeath(dt))
        return;

    walkAnimation.update(dt);

    sprite.setTextureRect(
        walkAnimation.getTextureRect()
    );

    if (!updateKnockback(dt))
    {
        velocity.x =
            facingRight ? moveSpeed : -moveSpeed;

        shape.move(
            velocity.x * dt,
            0.f
        );
    }

    updateSpritePosition();
}

bool Snail::usesGravity() const
{
    return true;
}
