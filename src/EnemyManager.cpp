#include "EnemyManager.hpp"
#include <iostream>
#include <algorithm>
#include "enemies/Snail.hpp"
#include "enemies/Fly.hpp"

EnemyManager::EnemyManager()
{
}

void EnemyManager::loadFromLevel(
    const Leveltmx& level,
    assetManager& assets
)
{
    auto enemySpawns = level.getEnemySpawns();

    for (const auto& spawn : enemySpawns)
    {
        if (spawn.type == "snail")
        {
            auto snail = std::make_unique<Snail>();

            snail->setId(enemies.size());

            snail->setTexture(
                assets.getTexture("snail_walk"),
                assets.getTexture("snail_death")
            );

            snail->setDissolveShader(
                assets.getShader("dissolve")
            );

            snail->setHitShader(
                assets.getShader("hit")
            );

            snail->setPosition(spawn.position);

            enemies.push_back(std::move(snail));
        }
        else if (spawn.type == "fly")
        {
            auto fly = std::make_unique<Fly>();

            fly->setId(enemies.size());

            fly->setTexture(
                assets.getTexture("fly_idle"),
                assets.getTexture("fly_death")
            );

            fly->setDissolveShader(
                assets.getShader("dissolve")
            );

            fly->setHitShader(
                assets.getShader("hit")
            );

            fly->setPosition(spawn.position);

            enemies.push_back(std::move(fly));
        }
    }
}

void EnemyManager::render(sf::RenderWindow& window) const
{
    for (const auto& enemy : enemies)
    {
        if (enemy->isDying() && enemy->getDissolveProgress() > 0.f)
        {
            sf::Shader* shader = enemy->getDissolveShader();

            if (shader)
            {
                shader->setUniform(
                    "progress",
                    enemy->getDissolveProgress()
                );

                window.draw(enemy->getSprite(), shader);
            }
            else
            {
                window.draw(enemy->getSprite());
            }
        }
        else if (enemy->isHitFlashing())
        {
            sf::Shader* shader = enemy->getHitShader();

            if (shader)
            {
                window.draw(enemy->getSprite(), shader);
            }
            else
            {
                window.draw(enemy->getSprite());
            }
        }
        else
        {
            window.draw(enemy->getSprite());
        }
    }
}

void EnemyManager::update(
    float dt,
    const Leveltmx& level,
    float gravity,
    const Player& player
)
{
    for (auto& enemy : enemies)
    {
        enemy->update(dt,player.getPosition());
        checkDamageTiles(*enemy, level);
        if (enemy->isDying())
        {
            enemy->updateKnockback(dt);
            enemy->applyGravity(dt, gravity);
            enemy->moveVertical(dt);
            resolveHorizontalCollisions(
                *enemy,
                level
            );
            resolveVerticalCollisions(
                *enemy,
                level
            );
            enemy->updateSpritePosition();
            continue;
        }

        resolveHorizontalCollisions(*enemy, level);

        if (enemy->usesGravity())
        {
            enemy->applyGravity(dt, gravity);
            enemy->moveVertical(dt);
        }
        resolveVerticalCollisions(*enemy, level);

        if (enemy->usesGravity() &&
            enemy->isOnGround() &&
            !hasGroundAhead(*enemy, level))
        {
            enemy->turnAround();
        }
    }

    enemies.erase(
        std::remove_if(
            enemies.begin(),
            enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy)
            {
                return enemy->isDeathAnimationFinished();
            }
        ),
        enemies.end()
    );
}

void EnemyManager::resolveHorizontalCollisions(
    Enemy& enemy,
    const Leveltmx& level
)
{
    sf::FloatRect currentBounds = enemy.getBounds();

    sf::FloatRect previousBounds(
        enemy.getPreviousPosition().x,
        enemy.getPreviousPosition().y,
        currentBounds.width,
        currentBounds.height
    );

    for (const auto& collision : level.getCollisions())
    {
        if (!currentBounds.intersects(collision.bounds))
            continue;
        if (previousBounds.top + previousBounds.height
            <= collision.bounds.top + 1.f)
        {
            continue;
        }
        if (previousBounds.top
            >= collision.bounds.top
            + collision.bounds.height - 1.f)
        {
            continue;
        }
        if (previousBounds.left + previousBounds.width
            <= collision.bounds.left)
        {
            enemy.setPosition(
                sf::Vector2f(
                    collision.bounds.left
                    - currentBounds.width
                    - 0.1f,
                    enemy.getPosition().y
                )
            );

            if (enemy.isDying())
            {
                enemy.stopKnockback();
            }
            else
            {
                enemy.turnAround();
            }

            break;
        }
        else if (previousBounds.left
                 >= collision.bounds.left
                 + collision.bounds.width)
        {
            enemy.setPosition(
                sf::Vector2f(
                    collision.bounds.left
                    + collision.bounds.width,
                    enemy.getPosition().y
                )
            );

            if (enemy.isDying())
            {
                enemy.stopKnockback();
            }
            else
            {
                enemy.turnAround();
            }

            break;
        }
    }
}

void EnemyManager::resolveVerticalCollisions(
    Enemy& enemy,
    const Leveltmx& level
)
{
    bool grounded = false;

    for (const auto& collision : level.getCollisions())
    {
        if (!enemy.getBounds().intersects(collision.bounds))
            continue;

        if (enemy.getPreviousPosition().y
            + enemy.getBounds().height
            <= collision.bounds.top + 5.f)
        {
            grounded = true;

            enemy.setPosition(
                sf::Vector2f(
                    enemy.getPosition().x,
                    collision.bounds.top
                    - enemy.getBounds().height
                )
            );

            enemy.stopVerticalMovement();
            break;
        }

        else if (enemy.getPreviousPosition().y
                 >= collision.bounds.top
                 + collision.bounds.height)
        {
            enemy.setPosition(
                sf::Vector2f(
                    enemy.getPosition().x,
                    collision.bounds.top
                    + collision.bounds.height
                )
            );

            enemy.stopVerticalMovement();
            break;
        }
    }

    if (grounded)
        enemy.land();
    else
        enemy.leaveGround();
}

bool EnemyManager::hasGroundAhead(
    const Enemy& enemy,
    const Leveltmx& level
)
{
    sf::FloatRect bounds = enemy.getBounds();

    float checkX;

    if (enemy.getVelocity().x > 0.f)
    {
        checkX = bounds.left + bounds.width + 2.f;
    }
    else
    {
        checkX = bounds.left - 2.f;
    }

    float checkY = bounds.top + bounds.height + 2.f;

    sf::Vector2f checkPoint(checkX, checkY);

    for (const auto& collision : level.getCollisions())
    {
        if (collision.bounds.contains(checkPoint))
        {
            return true;
        }
    }

    return false;
}

void EnemyManager::checkPlayerAttack(Player& player)
{
    if (!player.isAttacking())
        return;

    sf::FloatRect attackBounds =
        player.getAttackHitbox().getGlobalBounds();

    for (auto& enemy : enemies)
    {
        if (enemy->isDead())
            continue;

        if (player.hasHitEnemy(enemy->getId()))
            continue;

        if (attackBounds.intersects(enemy->getBounds()))
        {
            if (player.isFacingRight())
            {
                enemy->applyKnockback(20.f);
            }
            else
            {
                enemy->applyKnockback(-20.f);
            }

            enemy->takeDamage(1);

            player.addHitEnemy(
                enemy->getId()
            );
        }
    }
}

void EnemyManager::checkEnemyPlayerCollision(Player& player)
{
    if (player.isInvincible())
        return;

    for (auto& enemy : enemies)
    {
        if (enemy->isDead())
            continue;

        if (player.getBounds().intersects(enemy->getBounds()))
        {
            player.takeDamage(1);

            if (player.getPosition().x < enemy->getPosition().x)
            {
                player.applyKnockback(-20.f);
            }
            else
            {
                player.applyKnockback(20.f);
            }
        }
    }
}

void EnemyManager::reset(
    const Leveltmx& level,
    assetManager& assets
)
{
    enemies.clear();

    loadFromLevel(level, assets);
}

void EnemyManager::checkDamageTiles(
    Enemy& enemy,
    const Leveltmx& level
)
{
    if (enemy.isDead())
        return;

    for (const auto& damage : level.getDamageTiles())
    {
        if (enemy.getBounds().intersects(damage.bounds))
        {
            enemy.takeDamage(999);
            return;
        }
    }
}
