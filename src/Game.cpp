#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>

Game::Game()
    :window(sf::VideoMode(1280,720),"Platformer"),
    gravity(700.0f),
    currentLevel("level1.tmx"),
    level(nullptr),
    camera(sf::FloatRect(0.f,0.f,1280.f,720.f)),
    cameraLookAhead(0.f),
    transitionState(TransitionState::None),
    transitionAlpha(0.f),
    damageStunTimer(0.f),
    deathTransitionState(DeathTransitionState::None),
    deathTransitionAlpha(0.f),
    cameraShakeTimer(0.f),
    cameraShakeDuration(0.f),
    cameraShakeMagnitude(0.f)
{
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    camera.zoom(0.5f);
    transitionOverlay.setSize(
        window.getDefaultView().getSize()
    );

    transitionOverlay.setPosition(0.f, 0.f);

    transitionOverlay.setFillColor(
        sf::Color(0, 0, 0, 0)
    );
    sf::Vector2f viewSize =window.getDefaultView().getSize();

    assets.loadShader("dissolve","assets/shaders/dissolve.frag");
    assets.loadShader("hit","assets/shaders/hit.frag");
    assets.loadTexture("player_idle","assets/textures/player_idle.png");
    assets.loadTexture("player_run", "assets/textures/player_run.png");
    assets.loadTexture("player_jump","assets/textures/player_jump.png");
    assets.loadTexture("player_attack","assets/textures/player_attack1.png");
    assets.loadTexture("snail_walk","assets/textures/snail_walk.png");
    assets.loadTexture("snail_death","assets/textures/snail_death.png");
    assets.loadTexture("fly_idle","assets/textures/fly_idle.png");
    assets.loadTexture("fly_death","assets/textures/fly_death.png");
    assets.loadTexture("heart", "assets/textures/heart.png");
    assets.loadTexture("empty_heart","assets/textures/empty_heart.png");
    assets.loadTexture("heart_loss","assets/textures/heart_loss.png");

    player.setTextures(
        assets.getTexture("player_idle"),
        assets.getTexture("player_run"),
        assets.getTexture("player_jump"),
        assets.getTexture("player_attack")
    );

    player.setHitShader(
        assets.getShader("hit")
    );

    healthBar.setTextures(
        assets.getTexture("heart"),
        assets.getTexture("empty_heart"),
        assets.getTexture("heart_loss")
    );

    healthBar.setPosition(30.f, 30.f);
    healthBar.setMaxHealth(player.getMaxHealth());
    healthBar.setHealth(player.getHealth());

    saveData = SaveManager::load();

    if(SaveManager::hasSave())
    {
        currentLevel = saveData.currentRoom;
    }

    level = std::make_unique<Leveltmx>(
        "assets/levels/" + currentLevel
    );

    if(SaveManager::hasSave())
    {
        player.setPosition(saveData.checkpoint);
    }
    else
    {
        player.setPosition(level->getPlayerSpawn("Start"));
    }

    sf::Vector2f spawn=player.getPosition();
    lastSafePosition=spawn;

    enemyManager.loadFromLevel(*level, assets);
    camera.setCenter(
        spawn.x + 200.f,
        spawn.y + 100.f
    );
}

void Game::run()
{
    sf::Clock clock;

    while(window.isOpen())
    {
        float dt=clock.restart().asSeconds();

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    sf::Event event;

    while(window.pollEvent(event))
    {
        if(event.type==sf::Event::Closed)
        {
            window.close();
        }

        if(event.type==sf::Event::KeyPressed)
        {
            if(event.key.code==sf::Keyboard::Space)
            {
                player.startJump();
            }

            if(event.key.code==sf::Keyboard::D && !player.isInvincible())
            {
                player.startAttack();
            }

            if(event.key.code==sf::Keyboard::F5)
            {
                saveGame();
            }
        }

        if(event.type==sf::Event::KeyReleased)
        {
            if(event.key.code==sf::Keyboard::Space)
            {
                player.stopJump();
            }
        }
    }
}

void Game::update(float dt)
{
    if (damageStunTimer > 0.f)
    {
        damageStunTimer -= dt;

        if (damageStunTimer < 0.f)
        {
            damageStunTimer = 0.f;
        }
    }

    updateTransition(dt);

    if (transitionState != TransitionState::None)
        return;

    updateDeathTransition(dt);

    if (player.isDead())
    {
        if (deathTransitionState == DeathTransitionState::None)
        {
            deathTransitionState = DeathTransitionState::FadeOut;
            deathTransitionAlpha = 0.f;
        }

        healthBar.update(dt);

        return;
    }

    if (damageStunTimer <= 0.f &&
        !(player.isAttacking() && player.isOnGround()))
    {
        player.stopHorizontalMovement();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            player.moveRight();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            player.moveLeft();
        }
    }

    player.moveHorizontal(dt);
    resolveHorizontalCollisions();

    player.updateTimers(dt);
    player.updateAttackCooldown(dt);
    player.updateInvincibility(dt);

    player.applyGravity(dt, gravity);
    player.moveVertical(dt);
    resolveVerticalCollisions();

    checkLevelExit();
    checkDamageTiles();

    updateCamera(dt);

    player.updateSpritePosition();
    player.updateAttackHitbox();
    player.updateAnimation(dt);

    if (damageStunTimer <= 0.f)
    {
        enemyManager.update(
            dt,
            *level,
            gravity,
            player
        );

        enemyManager.checkPlayerAttack(player);
        enemyManager.checkEnemyPlayerCollision(player);
    }

    healthBar.setHealth(player.getHealth());
    healthBar.update(dt);
    if(player.didTakeDamage())
    {
        startCameraShake(0.2f, 20.f);
        player.clearDamageFlag();
    }
    updateCameraShake(dt);
}

void Game::render()
{
    window.clear();

    //World
    window.setView(camera);

    level->draw(window);

    if(player.isHitFlashing())
    {
        sf::Shader* shader = player.getHitShader();

        if(shader)
        {
            window.draw(player.getSprite(), shader);
        }
        else
        {
            window.draw(player.getSprite());
        }
    }
    else
    {
        window.draw(player.getSprite());
    }

    enemyManager.render(window);
    level->drawForeground(window);
    float totalTime = absoluteClock.getElapsedTime().asSeconds();
    level->drawPlatforms(window, totalTime);

    //UI
    window.setView(window.getDefaultView());
    healthBar.draw(window);
    window.draw(transitionOverlay);
    window.display();
}

void Game::resolveHorizontalCollisions()
{
    for(const auto& collision : level->getCollisions())
    {
        if(player.getBounds().intersects(collision.bounds))
        {
            if(player.getPreviousPosition().x + player.getBounds().width
                <= collision.bounds.left)
            {
                player.setPosition(
                    sf::Vector2f(
                        collision.bounds.left - player.getBounds().width - 0.1,
                        player.getPosition().y
                    )
                );

                player.stopHorizontalMovement();
                break;
            }
            else if(player.getPreviousPosition().x
                >= collision.bounds.left + collision.bounds.width)
            {
                player.setPosition(
                    sf::Vector2f(
                        collision.bounds.left + collision.bounds.width,
                        player.getPosition().y
                    )
                );

                player.stopHorizontalMovement();
                break;
            }
        }
    }
}

void Game::resolveVerticalCollisions()
{
    bool grounded = false;

    for(const auto& collision : level->getCollisions())
    {
        if(!player.getBounds().intersects(collision.bounds))
        {
            continue;
        }

        if(player.getPreviousPosition().y + player.getBounds().height
            <= collision.bounds.top + 5.f)
        {
            grounded = true;

            player.setPosition(
                sf::Vector2f(
                    player.getPosition().x,
                    collision.bounds.top - player.getBounds().height
                )
            );

            break;
        }
        else if(player.getPreviousPosition().y
            >= collision.bounds.top + collision.bounds.height)
        {
            player.setPosition(
                sf::Vector2f(
                    player.getPosition().x,
                    collision.bounds.top + collision.bounds.height
                )
            );

            player.stopVerticalMovement();
            break;
        }
    }

    if(grounded)
    {
        player.stopVerticalMovement();
        player.land();
        lastSafePosition = player.getPosition();
    }
    else
    {
        player.leaveGround();
    }
}

void Game::updateCamera(float dt)
{
    float targetLookAhead;

    if(player.isFacingRight())
        targetLookAhead = 200.f;
    else
        targetLookAhead = -200.f;

    float lookAheadSpeed = 300.f;

    if(cameraLookAhead < targetLookAhead)
    {
        cameraLookAhead += lookAheadSpeed * dt;

        if(cameraLookAhead > targetLookAhead)
            cameraLookAhead = targetLookAhead;
    }
    else if(cameraLookAhead > targetLookAhead)
    {
        cameraLookAhead -= lookAheadSpeed * dt;

        if(cameraLookAhead < targetLookAhead)
            cameraLookAhead = targetLookAhead;
    }

    float targetX = player.getPosition().x + cameraLookAhead;

    float currentX = camera.getCenter().x;
    float currentY = camera.getCenter().y;

    float cameraSpeed = 1.5f;

    float horizontalDeadZone = 100.f;
    float verticalDeadZone = 100.f;

    if(targetX > currentX + horizontalDeadZone)
    {
        currentX +=
            (targetX - (currentX + horizontalDeadZone))
            * cameraSpeed * dt;
    }
    else if(targetX < currentX - horizontalDeadZone)
    {
        currentX +=
            (targetX - (currentX - horizontalDeadZone))
            * cameraSpeed * dt;
    }

    float targetY = player.getPosition().y + 50.f;

    if(targetY > currentY)
    {
        currentY +=
            (targetY - currentY)
            * cameraSpeed * dt;
    }
    else if(targetY < currentY)
    {
        currentY +=
            (targetY - currentY)
            * cameraSpeed * dt;
    }

    float halfWidth = camera.getSize().x / 2.f;

    if(currentX < halfWidth)
    {
        currentX = halfWidth;
    }

    if(currentX > level->getWidth() - halfWidth)
    {
        currentX = level->getWidth() - halfWidth;
    }

    float halfHeight = camera.getSize().y / 2.f;

    if(currentY < halfHeight)
    {
        currentY = halfHeight;
    }

    if(currentY > level->getHeight() - halfHeight)
    {
        currentY = level->getHeight() - halfHeight;
    }

    camera.setCenter(currentX, currentY);
}

void Game::saveGame()
{
    saveData.currentRoom = currentLevel;
    saveData.checkpoint = player.getPosition();

    SaveManager::save(saveData);
}

void Game::checkLevelExit()
{
    sf::FloatRect playerBounds =
        player.getBounds();

    for(const auto& exit : level->getLevelExits())
    {
        if(playerBounds.intersects(exit.bounds))
        {
            startLevelTransition(
                exit.nextLevel,
                exit.spawnPoint
            );

            break;
        }
    }
}

void Game::loadLevel(
    const std::string& levelName,
    const std::string& spawnPoint
)
{
    currentLevel = levelName;
    level = std::make_unique<Leveltmx>(
        "assets/levels/" + currentLevel
    );
    sf::Vector2f spawn =
        level->getPlayerSpawn(spawnPoint);
    player.setPosition(spawn);
    lastSafePosition = spawn;
    enemyManager.reset(*level, assets);
    camera.setCenter(
        spawn.x + 200.f,
        spawn.y + 100.f
    );
}

void Game::startLevelTransition(
    const std::string& levelName,
    const std::string& spawnPoint
)
{
    if(transitionState != TransitionState::None)
        return;

    nextLevel = levelName;
    nextSpawnPoint = spawnPoint;

    transitionAlpha = 0.f;

    transitionState = TransitionState::FadeOut;
}

void Game::updateTransition(float dt)
{
    const float fadeSpeed = 1000.f;

    if(transitionState == TransitionState::FadeOut)
    {
        transitionAlpha += fadeSpeed * dt;

        if(transitionAlpha >= 255.f)
        {
            transitionAlpha = 255.f;

            loadLevel(
                nextLevel,
                nextSpawnPoint
            );

            transitionAlpha = 0.f;
            transitionState = TransitionState::None;
        }
    }

    transitionOverlay.setFillColor(
        sf::Color(
            0,
            0,
            0,
            static_cast<sf::Uint8>(transitionAlpha)
        )
    );
}

void Game::checkDamageTiles()
{
    sf::FloatRect playerBounds = player.getBounds();

    for(const auto& damage : level->getDamageTiles())
    {
        if(!playerBounds.intersects(damage.bounds))
        {
            continue;
        }

        if(player.isInvincible())
        {
            player.setPosition(lastSafePosition);
            return;
        }

        player.takeDamage(1);

        if(!player.isDead())
        {
            player.setPosition(lastSafePosition);
            player.stopHorizontalMovement();
            damageStunTimer = player.getInvincibilityTime();
        }

        return;
    }
}

void Game::updateDeathTransition(float dt)
{
    const float fadeSpeed = 200.f;
    if (deathTransitionState == DeathTransitionState::FadeOut)
    {
        deathTransitionAlpha += fadeSpeed * dt;
        if (deathTransitionAlpha >= 255.f)
        {
            deathTransitionAlpha = 255.f;
            if (SaveManager::hasSave())
            {
                saveData = SaveManager::load();
                currentLevel = saveData.currentRoom;
                level = std::make_unique<Leveltmx>(
                    "assets/levels/" + currentLevel
                );
                sf::Vector2f spawn = saveData.checkpoint;
                player.respawn(spawn);
                lastSafePosition = spawn;
                enemyManager.reset(*level, assets);
                camera.setCenter(
                    spawn.x + 200.f,
                    spawn.y + 100.f
                );
            }
            else
            {
                sf::Vector2f spawn =
                    level->getPlayerSpawn("Start");
                player.respawn(spawn);
                lastSafePosition = spawn;
                enemyManager.reset(*level, assets);
                camera.setCenter(
                    spawn.x + 200.f,
                    spawn.y + 100.f
                );
            }

            deathTransitionState = DeathTransitionState::FadeIn;
        }
    }
    else if (deathTransitionState == DeathTransitionState::FadeIn)
    {
        deathTransitionAlpha -= fadeSpeed * dt;

        if (deathTransitionAlpha <= 0.f)
        {
            deathTransitionAlpha = 0.f;
            deathTransitionState = DeathTransitionState::None;
        }
    }

    transitionOverlay.setFillColor(
        sf::Color(
            0,
            0,
            0,
            static_cast<sf::Uint8>(deathTransitionAlpha)
        )
    );
}

void Game::startCameraShake(float duration, float magnitude)
{
    cameraShakeTimer = duration;
    cameraShakeDuration = duration;
    cameraShakeMagnitude = magnitude;
}

void Game::updateCameraShake(float dt)
{
    if (cameraShakeTimer <= 0.f)
        return;

    cameraShakeTimer -= dt;

    if (cameraShakeTimer < 0.f)
        cameraShakeTimer = 0.f;

    float progress =
        cameraShakeTimer / cameraShakeDuration;

    float currentMagnitude =
        cameraShakeMagnitude * progress;

    float offsetX =
        static_cast<float>(std::rand() % 200 - 100) / 100.f
        * currentMagnitude;

    float offsetY =
        static_cast<float>(std::rand() % 200 - 100) / 100.f
        * currentMagnitude;

    camera.move(offsetX, offsetY);
}

