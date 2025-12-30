/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** GameTypes
*/

#include <iostream>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>

#include "AudioPlayer.hpp"
#include "Components/Animation.hpp"
#include "Components/Sound.hpp"
#include "Components/Sprite.hpp"
#include "Entity.hpp"
#include "Renderer.hpp"
#include "Systems/Animate.hpp"
#include "Systems/DevConsole.hpp"
#include "Systems/SoundPlayer.hpp"
#include "DLLoader.hpp"
#include "GameTypes.hpp"
#include "Mediator.hpp"
#include "Systems/Collision.hpp"
#include "Systems/Enemy.hpp"
#include "Systems/PhysicsNoEngine.hpp"
#include "Systems/Render.hpp"
#include "Systems/PlayerControl.hpp"
#include "Utils.hpp"
#include "Components/LevelInfo.hpp"
#include "Components/GameState.hpp"
#include "editor/EditorState.hpp"

GameManager::GameManager(Engine::Utils::Vec2UInt windowSize, const std::string &serverIP, uint16_t serverPort)
    : launch(windowSize), parameters(windowSize), controlsConfig(windowSize), lobby(windowSize), errorServer(windowSize), player(windowSize),
      waitingPlayersCounter(1),
      gameMode(GameMode::SOLO),
      trophy(windowSize),
      particleSystem(windowSize, 300),
      currentState(State::LAUNCH),
      isConnected(ServerState::DEFAULT),
      UsernameGame(""),
      isDraggingVolume(false),
      isChooseMode(false),
      isConfiguringControls(false),
      currentFps(60),
      isEditingUsername(false),
      cursorPos(0),
      serverIP(serverIP),
      serverPort(serverPort)
{

    #if defined(_WIN32)
    const std::string libName = "libengine.dll";
    #else
    const std::string libName = "libengine.so";
    #endif

    Engine::DLLoader loader;
    createMediatorFunc = loader.getFunction<Engine::Mediator*(*)()>(libName, "createMediator");
    createNetworkManagerFunc = loader.getFunction<Engine::NetworkManager*(*)(Engine::NetworkManager::Role, const std::string &, uint16_t)>(libName, "createNetworkManager");

    if (!font.loadFromFile("assets/r-type.otf")) {
        std::cerr << "Unable to load font" << std::endl;
    }
    if (!username.loadFile()) {
        std::cerr << "Files not load for username" << std::endl;
    }

    // renderer = std::make_shared<Engine::Renderers::SFML>();
    // renderer->createWindow(800, 600, "R du TYPE");

    // renderer->loadFont("basic", "assets/r-type.otf");

    const int mid_window_x = static_cast<int>(windowSize.x / 2);
    const int mid_window_y = static_cast<int>(windowSize.y / 2);

    username = Username(sf::Vector2f(mid_window_x - 100, windowSize.y - 150), sf::Vector2f(200, 50), UsernameGame, font);

    playButton = Button(sf::Vector2f(mid_window_x - 100, windowSize.y - 250), sf::Vector2f(200, 50), "Play", font);

    paramButton = ParamButton(sf::Vector2f(mid_window_x - 100, windowSize.y - 200), sf::Vector2f(125, 40), "Parameters", font);
    fps30Button = ParamButton(sf::Vector2f(mid_window_x - 90, windowSize.y - 60), sf::Vector2f(80, 40), "FPS 30", font);
    fps60Button = ParamButton(sf::Vector2f(mid_window_x + 10, windowSize.y - 60), sf::Vector2f(80, 40), "FPS 60", font);
    backButton = Button(sf::Vector2f(50, windowSize.y - 100), sf::Vector2f(100, 40), "Back", font);
    float buttonWidth = std::min(120.0f, windowSize.x * 0.15f);
    float buttonX = std::min((float)(windowSize.x - buttonWidth - 20), (float)(windowSize.x * 0.75f));
    
    resolutionButton = Button(sf::Vector2f(buttonX, 200), sf::Vector2f(buttonWidth, 30), "Change", font);
    displayModeButton = Button(sf::Vector2f(buttonX, 250), sf::Vector2f(buttonWidth, 30), "Change", font);
    graphicsQualityButton = Button(sf::Vector2f(buttonX, 300), sf::Vector2f(buttonWidth, 30), "Change", font);
    colorBlindModeButton = Button(sf::Vector2f(buttonX, 350), sf::Vector2f(buttonWidth, 30), "Change", font);
    controlsButton = Button(sf::Vector2f(buttonX, 400), sf::Vector2f(buttonWidth, 30), "Controls", font);
    controlsButton.setCharacterSize(12);
    
    float applyButtonWidth = std::min(150.0f, windowSize.x * 0.25f);
    applyResolutionButton = Button(sf::Vector2f(mid_window_x - applyButtonWidth/2, 450), sf::Vector2f(applyButtonWidth, 35), "Apply", font);
    
    if (!launch.loadResources() || !parameters.loadResources() || !controlsConfig.loadResources() || !player.loadResources() || !errorServer.loadResources() || !trophy.loadResources()) {
        std::cerr << "Erreur lors du chargement des ressources" << std::endl;
    }

    soloButton = Button(sf::Vector2f(mid_window_x - 350, windowSize.y - 350), sf::Vector2f(100, 40), "Solo",font);
    duoButton = Button(sf::Vector2f(mid_window_x - 350, windowSize.y - 300), sf::Vector2f(100, 40), "Duo",font);
    trioButton = Button(sf::Vector2f(mid_window_x - 350, windowSize.y - 250), sf::Vector2f(100, 40), "Trio",font);
    squadButton = Button(sf::Vector2f(mid_window_x - 350, windowSize.y - 200), sf::Vector2f(100, 40), "Squad",font);
    modeButton = Button(sf::Vector2f(mid_window_x - 350, windowSize.y - 150), sf::Vector2f(100, 40), "Mode", font);
    playButton = Button(sf::Vector2f(mid_window_x + 250, windowSize.y - 100), sf::Vector2f(100, 40), "Play", font);

    numberPlayerToWait.setFont(font);
    numberPlayerToWait.setCharacterSize(10);
    numberPlayerToWait.setFillColor(sf::Color::Yellow);
    sf::FloatRect numberPlayerToWaitBounds = numberPlayerToWait.getLocalBounds();
    numberPlayerToWait.setPosition(mid_window_x - numberPlayerToWaitBounds.width / 2, 20);

    lockerButton = Button(sf::Vector2f(mid_window_x + 250, windowSize.y - 100), sf::Vector2f(100, 40), "Locker", font);
    leftButtonSelection = Button(sf::Vector2f(mid_window_x - 50, windowSize.y + 50), sf::Vector2f(100, 40), "<", font);
    rightButtonSelection = Button(sf::Vector2f(mid_window_x - 50, windowSize.y + 50), sf::Vector2f(100, 40), ">", font);
    applyButtonLocker = Button(sf::Vector2f(mid_window_x - 50, windowSize.y - 200), sf::Vector2f(100, 40), "Apply", font);

    leaderboard = Button(sf::Vector2f(mid_window_x - 350, windowSize.y - 100), sf::Vector2f(200, 50), "Leaderboard", font);
    editorButton = Button(sf::Vector2f(mid_window_x - 120, windowSize.y - 100), sf::Vector2f(200, 50), "Editor", font);
    trophy.leaderboardRectangle.setSize(sf::Vector2f(mid_window_x, mid_window_y + 150));
    trophy.leaderboardRectangle.setPosition(sf::Vector2f(mid_window_x - 200, mid_window_y - 200));

    statusText.setFont(font);
    statusText.setCharacterSize(10);
    statusText.setFillColor(sf::Color::Yellow);
    
    fpsDisplay.setFont(font);
    fpsDisplay.setCharacterSize(10);
    fpsDisplay.setFillColor(sf::Color::Green);
    fpsDisplay.setPosition(10, 10);

    insertCoinText.setFont(font);
    insertCoinText.setString("INSERT COIN");
    insertCoinText.setCharacterSize(38);
    insertCoinText.setFillColor(sf::Color::Yellow);
    sf::FloatRect bounds = insertCoinText.getLocalBounds();
    insertCoinText.setPosition(mid_window_x - bounds.width/2, mid_window_y - bounds.height/2 + 50);

    paramButton.setupVolumeBar(sf::Vector2f(windowSize.x - 220, windowSize.y - 80), 200.f);
    particleSystem.setParameters(&parameters);
}

void GameManager::startLevelWipe(uint32_t newLevel)
{
    levelWipeTarget = newLevel;
    levelWipeMidApplied = false;
    levelWipePreloadMusicId = "background_music_next_" + std::to_string(newLevel);
    levelWipePreloadTextureId = "bg_preload_tex_" + std::to_string(newLevel);
    levelWipePreloadSpriteId = "bg_preload_sprite_" + std::to_string(newLevel);

    std::string music_path;
    switch (newLevel) {
        case 1: music_path = "assets/sound/Start_sound.mp3"; break;
        case 2: music_path = "assets/sound/Stage2_sound.mp3"; break;
        case 3: music_path = "assets/sound/Stage3_sound.mp3"; break;
        case 4: music_path = "assets/sound/Win_level_sound.mp3"; break;
        default: music_path = "assets/sound/Stage3_sound.mp3"; break;
    }
    if (sound_system && audio_player) {
        sound_system->addSound(audio_player, levelWipePreloadMusicId, music_path);
    }
    if (render_system && renderer) {
        if (newLevel == 2) {
            const std::string bg_path = "assets/sprites/stage2_background.png";
            render_system->addTexture(renderer, levelWipePreloadTextureId, bg_path);
            render_system->addSprite(renderer, levelWipePreloadSpriteId, levelWipePreloadTextureId, {1226, 207}, {0, 0}, 1, 1);
        } else {
            const std::string bg_path = "assets/sprites/space_background.gif";
            render_system->addTexture(renderer, levelWipePreloadTextureId, bg_path);
            render_system->addSprite(renderer, levelWipePreloadSpriteId, levelWipePreloadTextureId, {1226, 207}, {0, 0}, 1, 1);
        }
    }
    levelWipeStart = std::chrono::high_resolution_clock::now();
    levelWipeActive = true;
}

void GameManager::applyLevelMusic(uint32_t level)
{
    if (audio_player && !currentPlayingMusicId.empty()) {
        if (currentPlayingMusicId != levelWipePreloadMusicId) {
            audio_player->stopAudio(currentPlayingMusicId);
            audio_player->unloadAudio(currentPlayingMusicId);
        }
    }
    if (sound_system && audio_player && !levelWipePreloadMusicId.empty()) {
        audio_player->playAudio(levelWipePreloadMusicId, true);
        currentPlayingMusicId = levelWipePreloadMusicId;
    } else {
        std::string music_path;
        switch (level) {
            case 1: music_path = "assets/sound/Start_sound.mp3"; break;
            case 2: music_path = "assets/sound/Stage2_sound.mp3"; break;
            case 3: music_path = "assets/sound/Stage3_sound.mp3"; break;
            case 4: music_path = "assets/sound/Win_level_sound.mp3"; break;
            default: music_path = "assets/sound/Stage3_sound.mp3"; break;
        }
        if (sound_system && audio_player) {
            sound_system->addSound(audio_player, "background_music", music_path);
            audio_player->playAudio("background_music", true);
            currentPlayingMusicId = "background_music";
        }
    }
}

void GameManager::applyBackgroundSwap(uint32_t level)
{
    if (!(render_system && renderer)) return;
    const std::string spriteIdToUse = !levelWipePreloadSpriteId.empty() ? levelWipePreloadSpriteId : (level == 2 ? "stage2_background" : "space_background");
    for (const auto &ent : render_system->entities) {
        if (!networkManager->mediator->hasComponent<Engine::Components::Sprite>(ent))
            continue;
        auto &sp = networkManager->mediator->getComponent<Engine::Components::Sprite>(ent);
        if (sp.is_background) {
            std::strncpy(sp.sprite_name.data(), spriteIdToUse.c_str(), sp.sprite_name.size() - 1);
            sp.sprite_name[sp.sprite_name.size() - 1] = '\0';
        }
    }
}
GameManager::~GameManager() = default;

void GameManager::updatePositions(Engine::Utils::Vec2UInt windowSize)
{
    const int mid_window_x = static_cast<int>(windowSize.x / 2);
    const int mid_window_y = static_cast<int>(windowSize.y / 2);

    paramButton.updatePositionAndSize(sf::Vector2f(50, windowSize.y - 100), sf::Vector2f(125, 40));
    username.updatePositionAndSize(sf::Vector2f(50, windowSize.y - 100), sf::Vector2f(125, 40));
    fps30Button.updatePositionAndSize(sf::Vector2f(mid_window_x - 90, windowSize.y - 60), sf::Vector2f(80, 40));
    fps60Button.updatePositionAndSize(sf::Vector2f(mid_window_x + 10, windowSize.y - 60), sf::Vector2f(80, 40));
    backButton.updatePositionAndSize(sf::Vector2f(50, windowSize.y - 100), sf::Vector2f(100, 40));

    float leftColX = 50;
    float baseY = windowSize.y - 350;
    float spacing = 50;
    soloButton.updatePositionAndSize(sf::Vector2f(leftColX, baseY), sf::Vector2f(100, 40));
    duoButton.updatePositionAndSize(sf::Vector2f(leftColX, baseY + spacing), sf::Vector2f(100, 40));
    trioButton.updatePositionAndSize(sf::Vector2f(leftColX, baseY + 2 * spacing), sf::Vector2f(100, 40));
    squadButton.updatePositionAndSize(sf::Vector2f(leftColX, baseY + 3 * spacing), sf::Vector2f(100, 40));
    modeButton.updatePositionAndSize(sf::Vector2f(leftColX, baseY + 4 * spacing), sf::Vector2f(100, 40));
    playButton.updatePositionAndSize(sf::Vector2f(windowSize.x - 150, windowSize.y - 100), sf::Vector2f(100, 40));

    lockerButton.updatePositionAndSize(sf::Vector2f(50, 50), sf::Vector2f(100, 40));

    leftButtonSelection.updatePositionAndSize(sf::Vector2f(50, mid_window_y - 20),sf::Vector2f(100, 40));
    rightButtonSelection.updatePositionAndSize(sf::Vector2f(windowSize.x - 150, mid_window_y - 20),
                                               sf::Vector2f(100, 40));
    applyButtonLocker.updatePositionAndSize(sf::Vector2f(mid_window_y - 50, windowSize.y - 100),
                                            sf::Vector2f(100, 40));
    leaderboard.updatePositionAndSize(sf::Vector2f(50,  100),
                                            sf::Vector2f(125, 40));
    editorButton.updatePositionAndSize(sf::Vector2f(50, 160),
                                            sf::Vector2f(125, 40));
    trophy.leaderboardRectangle.setPosition(sf::Vector2f(mid_window_x - 200,mid_window_y - 200));

    float buttonWidth = std::min(120.0f, windowSize.x * 0.15f);
    float buttonX = std::min((float)(windowSize.x - buttonWidth - 20), (float)(windowSize.x * 0.75f));
    resolutionButton.updatePositionAndSize(sf::Vector2f(buttonX, 200), sf::Vector2f(buttonWidth, 30));
    displayModeButton.updatePositionAndSize(sf::Vector2f(buttonX, 250), sf::Vector2f(buttonWidth, 30));
    graphicsQualityButton.updatePositionAndSize(sf::Vector2f(buttonX, 300), sf::Vector2f(buttonWidth, 30));
    colorBlindModeButton.updatePositionAndSize(sf::Vector2f(buttonX, 350), sf::Vector2f(buttonWidth, 30));
    controlsButton.updatePositionAndSize(sf::Vector2f(buttonX, 400), sf::Vector2f(buttonWidth, 30));
    float applyButtonWidth = std::min(150.0f, windowSize.x * 0.25f);
    applyResolutionButton.updatePositionAndSize(sf::Vector2f(mid_window_x - applyButtonWidth/2, 450),
                                                sf::Vector2f(applyButtonWidth, 35));

    sf::FloatRect numberPlayerToWaitBounds = numberPlayerToWait.getLocalBounds();
    numberPlayerToWait.setPosition(mid_window_x - numberPlayerToWaitBounds.width / 2, 20);
    sf::FloatRect insertCoinTextBounds = insertCoinText.getLocalBounds();
    insertCoinText.setPosition(mid_window_x - insertCoinTextBounds.width/2, mid_window_y - insertCoinTextBounds.height/2 + 50);
}

void GameManager::handleEvents(sf::RenderWindow& window)
{
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            currentState = State::QUIT;
            continue;
        }

        if (currentState == State::EDITOR) {
            if (editorState) {
                State previousState = currentState;
                editorState->handleEvent(event, currentState);
                if (previousState == State::EDITOR && currentState != State::EDITOR) {
                    editorState->onExit();
                    updateStatusTextPosition(false);
                    statusText.setString("");
                    statusText.setFillColor(sf::Color::Yellow);
                    Engine::Utils::Vec2UInt size(window.getSize().x, window.getSize().y);
                    updatePositions(size);
                }
            }
            continue;
        }

        if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event, window);
        }
        if (event.type == sf::Event::TextEntered && isEditingUsername) {
            if (event.text.unicode == '\b') {
                if (!UsernameGame.empty() && cursorPos > 0) {
                    UsernameGame.erase(cursorPos - 1, 1);
                    cursorPos--;
                }
            }
            else if (event.text.unicode == 127) {
                if (!UsernameGame.empty() && cursorPos < UsernameGame.size()) {
                    UsernameGame.erase(cursorPos, 1);
                }
            }
            else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                std::ofstream file("Username.txt");
                if (file.is_open()) {
                    file << UsernameGame;
                    file.close();
                }
                username = Username(username.getSize(), username.getSize(), UsernameGame, font);
                isEditingUsername = false;
            }
            else if (event.text.unicode < 128 && std::isprint(event.text.unicode)) {
                UsernameGame.insert(cursorPos, 1, static_cast<char>(event.text.unicode));
                cursorPos++;
            }
            std::ofstream file("Username.txt");
            if (file.is_open()) {
                file << UsernameGame;
                file.close();
            }
        }
        if (event.type == sf::Event::KeyPressed && isEditingUsername) {
            if (event.key.code == sf::Keyboard::Left && cursorPos > 0) {
                cursorPos--;
            }
            if (event.key.code == sf::Keyboard::Right && cursorPos < UsernameGame.size()) {
                cursorPos++;
            }
            if (event.key.code == sf::Keyboard::Delete) {
                if (!UsernameGame.empty() && cursorPos < UsernameGame.size()) {
                    UsernameGame.erase(cursorPos, 1);
                }
                std::ofstream file("Username.txt");
                if (file.is_open()) {
                    file << UsernameGame;
                    file.close();
                }
            }
        }
        if (event.type == sf::Event::Resized) {
            sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
            window.setView(sf::View(visibleArea));
            handleWindowResize(event);
        }
        if (event.type == sf::Event::MouseButtonPressed) {
            handleMouseClick(event, window);
        }
        if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                isDraggingVolume = false;
            }
        }
        if (event.type == sf::Event::MouseMoved) {
            handleMouseMove(window);
        } else {
            paramButton.setHovered(false);
            fps30Button.setHovered(false);
            fps60Button.setHovered(false);
            backButton.setHovered(false);
            editorButton.setHovered(false);
        }
        if (currentState != State::EDITOR) {
            launch.handleEvent(event, window);
            if (currentState == State::CONTROLS) {
                controlsConfig.handleEvent(event, window);
            }
            lobby.handleEvent(event, window);
            player.handleEvent(event, window);
            errorServer.handleEvent(event, window);
        }
    }
}

void GameManager::update()
{
    float deltaTime = deltaClock.restart().asSeconds();
    float coinTime = insertCoinClock.getElapsedTime().asSeconds();

    if (currentState == State::LAUNCH) {
        coinTime = static_cast<int>(128 + 127 * std::sin(coinTime * 2.5f));
        insertCoinText.setFillColor(sf::Color(255, 255, 0, coinTime));
        launch.update();
    } else if (currentState == State::MENU) {
        player.update();
    } else if (currentState == State::EDITOR) {
        if (editorState) {
            editorState->update(deltaTime);
        }
    } else if (currentState == State::LOCKER) {
        player.update();
    } else if (currentState == State::LOBBY) {
        if (networkManager && isConnected == ServerState::CONNECT) {
            if ((waitingPlayersCounter == 1 && gameMode == GameMode::SOLO) || (waitingPlayersCounter == 2 && gameMode == GameMode::DUO) ||
                (waitingPlayersCounter == 3 && gameMode == GameMode::TRIO) || (waitingPlayersCounter == 4 && gameMode == GameMode::SQUAD)) {
                currentState = State::GAME;
            }
        }
        player.update();
    } else if (currentState == State::ERRORSERVER) {
        errorServer.update();
    }
    if (currentState != State::LAUNCH) {
        fpsDisplay.setString("FPS: " + std::to_string(currentFps));
    }
    particleSystem.update(deltaTime);
}

void GameManager::render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    particleSystem.render(window);

    if (currentState == State::LAUNCH) {
        launch.draw(window);
        window.draw(insertCoinText);
    } else if (currentState == State::MENU) {
        paramButton.updatePositionAndSize(sf::Vector2f(50, window.getSize().y - 100), sf::Vector2f(125, 40));
        paramButton.draw(window);
        leaderboard.draw(window);
        editorButton.draw(window);
        if (isChooseMode) {
            soloButton.draw(window);
            duoButton.draw(window);
            trioButton.draw(window);
            squadButton.draw(window);
        }
        modeButton.draw(window);
        playButton.draw(window);
        lockerButton.draw(window);
        sf::Vector2f playerPos = player.getPosition();
        std::string displayName = UsernameGame;
        static bool showCursor = true;
        static sf::Clock cursorClock;
        if (cursorClock.getElapsedTime().asSeconds() > 0.5f) {
            showCursor = !showCursor;
            cursorClock.restart();
        }
        if (isEditingUsername && showCursor) {
            displayName.insert(cursorPos, "|");
        }
        sf::Text tempText(displayName, font, 10);
        sf::FloatRect textBounds = tempText.getLocalBounds();
        float padding = 20.f;
        sf::Vector2f usernameSize(
            std::max(60.f, textBounds.width + padding),
            std::max(30.f, textBounds.height + padding)
        );
        sf::Vector2f usernamePos(
            playerPos.x + (player.getSize().x - usernameSize.x) / 2,
            playerPos.y - usernameSize.y - 10
        );
        username = Username(usernamePos, usernameSize, displayName, font);
        username.setCharacterSize(10);

        sf::Text& label = username.getLabel();
        sf::FloatRect labelTextBounds = label.getLocalBounds();
        label.setOrigin(labelTextBounds.left + labelTextBounds.width / 2.f, labelTextBounds.top + labelTextBounds.height / 2.f);
        label.setPosition(
            usernamePos.x + usernameSize.x / 2.f,
            usernamePos.y + usernameSize.y / 2.f
        );

        username.draw(window);
        player.draw(window);
    } else if (currentState == State::EDITOR) {
        if (editorState) {
            editorState->render();
        }
    } else if (currentState == State::LEADERBOARD) {
        trophy.draw(window);
    } else if (currentState == State::GAME) {
        if (networkManager && isConnected == ServerState::CONNECT) {
            gameDemo(window);
            currentState = State::MENU;
        } else {
            currentState = State::MENU;
            updateStatusTextPosition(false);
            statusText.setString("Connect to the server first");
            statusText.setFillColor(sf::Color::Red);
        }
    } else if (currentState == State::SETTINGS) {
        parameters.draw(window);
        backButton.draw(window);
        fps30Button.draw(window);
        fps60Button.draw(window);
        resolutionButton.draw(window);
        displayModeButton.draw(window);
        graphicsQualityButton.draw(window);
        colorBlindModeButton.draw(window);
        controlsButton.draw(window);
        applyResolutionButton.draw(window);
        paramButton.drawVolumeBar(window);
    } else if (currentState == State::CONTROLS) {
        controlsConfig.draw(window);
        backButton.draw(window);
    } else if (currentState == State::LOBBY) {
        player.draw(window);
        window.draw(numberPlayerToWait);
    } else if (currentState == State::LOCKER) {
        player.draw(window);
        leftButtonSelection.draw(window);
        rightButtonSelection.draw(window);
        applyButtonLocker.draw(window);
        paramButton.updatePositionAndSize(sf::Vector2f(50, window.getSize().y - 100), sf::Vector2f(125, 30));
        paramButton.draw(window);
    } else if (currentState == State::ERRORSERVER) {
        errorServer.draw(window);
        paramButton.updatePositionAndSize(sf::Vector2f(50, window.getSize().y - 100), sf::Vector2f(125, 30));
        paramButton.draw(window);
    } else if (currentState == State::QUIT) {
        window.close();
    }
    window.draw(statusText);
    if (currentState != State::LAUNCH)
        window.draw(fpsDisplay);
    if (isConnected == ServerState::CONNECT) {
        sf::CircleShape indicator(10);
        indicator.setFillColor(sf::Color::Green);
        indicator.setPosition(750, 20);
        window.draw(indicator);
    } else if (isConnected == ServerState::DISCONNECT){
        sf::CircleShape indicator(10);
        indicator.setFillColor(sf::Color::Red);
        indicator.setPosition(750, 20);
        window.draw(indicator);
    }
    // Apply a very transparent overlay based on the selected color-blind mode
    // and the current level/world so the whole screen gets a subtle tint.
    {
        sf::Color overlay = parameters.getOverlayColor(this->currentLevel);
        if (overlay.a > 0) {
            sf::RectangleShape overlayRect(sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
            overlayRect.setPosition(0.f, 0.f);
            overlayRect.setFillColor(overlay);
            window.draw(overlayRect);
        }
    }
    window.display();
}

bool GameManager::shouldClose() const
{
    return currentState == State::QUIT;
}

void GameManager::activateEditor(sf::RenderWindow& window)
{
    if (!editorState) {
        editorState = std::make_unique<rtype::editor::EditorState>(window);
    }

    if (currentState != State::EDITOR) {
        editorState->onEnter();
    }

    currentState = State::EDITOR;
    isChooseMode = false;
    isEditingUsername = false;
    statusText.setString("");
    statusText.setFillColor(sf::Color::Yellow);
    paramButton.setHovered(false);
    leaderboard.setHovered(false);
    editorButton.setHovered(false);
}

void GameManager::handleKeyPress(sf::Event& event, sf::RenderWindow&)
{
    if (currentState == State::EDITOR) {
        return;
    }

    if (event.key.code == sf::Keyboard::Escape) {
        if (currentState == State::SETTINGS) {
            currentState = State::MENU;
            updateStatusTextPosition(false);
            statusText.setString("");
            statusText.setFillColor(sf::Color::Yellow);
        } else {
            currentState = State::QUIT;
        }
    }

    if (event.type == sf::Event::KeyPressed && isEditingUsername) {
        if (event.key.code == sf::Keyboard::Left && cursorPos > 0) {
            cursorPos--;
        }
        if (event.key.code == sf::Keyboard::Right && cursorPos < UsernameGame.size()) {
            cursorPos++;
        }
        if (event.key.code == sf::Keyboard::Delete) {
            if (!UsernameGame.empty() && cursorPos < UsernameGame.size()) {
                UsernameGame.erase(cursorPos, 1);
            }
        }
    }
}

void GameManager::handleMouseClick(sf::Event& event, sf::RenderWindow& window) {
    if (event.mouseButton.button != sf::Mouse::Left) return;

    if (currentState == State::EDITOR) {
        return;
    }

    sf::Vector2i sfMousePos = sf::Mouse::getPosition(window);
    Engine::Utils::Vec2Int mousePos(sfMousePos.x, sfMousePos.y);

    if (currentState == State::LAUNCH) {
        currentState = State::MENU;
    } else if (currentState == State::MENU) {
        if (username.isClicked(mousePos)) {
            isEditingUsername = true;
            cursorPos = UsernameGame.size();
        }
        if (soloButton.isClicked(mousePos)) {
            gameMode = GameMode::SOLO;
            numberPlayerToWait.setString("Waiting players: " + std::to_string(getWaitingPlayersCount()) + "/1");
        } else if (duoButton.isClicked(mousePos)) {
            gameMode = GameMode::DUO;
            numberPlayerToWait.setString("Waiting players: " + std::to_string(getWaitingPlayersCount()) + "/2");
        } else if (trioButton.isClicked(mousePos)) {
            gameMode = GameMode::TRIO;
            numberPlayerToWait.setString("Waiting players: " + std::to_string(getWaitingPlayersCount()) + "/3");
        } else if (squadButton.isClicked(mousePos)) {
            gameMode = GameMode::SQUAD;
            numberPlayerToWait.setString("Waiting players: " + std::to_string(getWaitingPlayersCount()) + "/4");
        }
        if (modeButton.isClicked(mousePos)) {
            if (isChooseMode)
                isChooseMode = false;
            else
                isChooseMode = true;
        } else {
            isChooseMode = false;
        }
        if (leaderboard.isClicked(mousePos)) {
            currentState = State::LEADERBOARD;
            updateStatusTextPosition(true);
            statusText.setString("");
        }
        if (editorButton.isClicked(mousePos)) {
            #ifndef _WIN32
            activateEditor(window);
            #endif
            return;
        }
        if (lockerButton.isClicked(mousePos)) {
            currentState = State::LOCKER;
            updateStatusTextPosition(true);
            statusText.setString("");
        }
        if (paramButton.isClicked(mousePos)) {
            currentState = State::SETTINGS;
            updateStatusTextPosition(true);
            statusText.setString("");
        }
        if (playButton.isClicked(mousePos)) {
            if (isConnected == ServerState::CONNECT) {
                currentState = State::LOBBY;
                updateStatusTextPosition(true);
                statusText.setString("");
            } else {
                if (connectToServer(serverIP, serverPort)) {
                    statusText.setString("Connected to the server !");
                    statusText.setFillColor(sf::Color::Green);
                    isConnected = ServerState::CONNECT;
                    currentState = State::LOBBY;
                    updateStatusTextPosition(true);
                    statusText.setString("");
                } else {
                    statusText.setString("Connection failed");
                    statusText.setFillColor(sf::Color::Red);
                    isConnected = ServerState::DISCONNECT;
                }
            }
        }
    } else if (currentState == State::SETTINGS) {
        if (backButton.isClicked(mousePos)) {
            if (isConnected == ServerState::DISCONNECT) {
                currentState = State::ERRORSERVER;
            } else {
                currentState = State::MENU;
            }
            updateStatusTextPosition(false);
            statusText.setString("");
            statusText.setFillColor(sf::Color::Yellow);
        }
        if (fps30Button.isClicked(mousePos)) {
            currentFps = 30;
            window.setFramerateLimit(30);
        }
        if (fps60Button.isClicked(mousePos)) {
            currentFps = 60;
            window.setFramerateLimit(60);
        }
        if (resolutionButton.isClicked(mousePos)) {
            cycleResolution();
        }
        if (displayModeButton.isClicked(mousePos)) {
            cycleDisplayMode(window);
        }
        if (graphicsQualityButton.isClicked(mousePos)) {
            cycleGraphicsQuality();
        }
        if (colorBlindModeButton.isClicked(mousePos)) {
            cycleColorBlindMode();
        }
        if (controlsButton.isClicked(mousePos)) {
            currentState = State::CONTROLS;
        }
        if (applyResolutionButton.isClicked(mousePos)) {
            applyCurrentResolution(window);
        }
        if (paramButton.isVolumeBarClicked(mousePos)) {
            isDraggingVolume = true;
            paramButton.setVolumeFromMouse(mousePos.x);
        }
    } else if (currentState == State::CONTROLS) {
        if (backButton.isClicked(mousePos)) {
            currentState = State::SETTINGS;
        }
    } else if (currentState == State::LOCKER) {
        if (applyButtonLocker.isClicked(mousePos)) {
            currentState = State::MENU;
            updateStatusTextPosition(true);
            statusText.setString("");
        }
        if (leftButtonSelection.isClicked(mousePos)) {
            int newTop = player.starshipRect.top - 17;
            if (newTop < 0)
                newTop = 0;
            player.starshipRect.top = newTop;
            player.starshipSprite.setTextureRect(player.starshipRect);
        }
        if (rightButtonSelection.isClicked(mousePos)) {
            int maxTop = 17 * 4;
            int newTop = player.starshipRect.top + 17;
            if (newTop > maxTop)
                newTop = maxTop;
            player.starshipRect.top = newTop;
            player.starshipSprite.setTextureRect(player.starshipRect);
        }
    } else if (currentState == State::LOBBY) {
    } else if (currentState == State::ERRORSERVER) {
        if (paramButton.isClicked(mousePos)) {
            currentState = State::SETTINGS;
            updateStatusTextPosition(true);
            statusText.setString("");
        }
    }
}

void GameManager::handleMouseMove(sf::RenderWindow& window)
{
    sf::Vector2i sfMousePos = sf::Mouse::getPosition(window);
    Engine::Utils::Vec2Int mousePos(sfMousePos.x, sfMousePos.y);
    
    if (currentState == State::EDITOR) {
        return;
    }

    if (currentState == State::MENU) {
        paramButton.setHovered(paramButton.isClicked(mousePos));
        if (isChooseMode) {
            soloButton.setHovered(soloButton.isClicked(mousePos));
            duoButton.setHovered(duoButton.isClicked(mousePos));
            trioButton.setHovered(trioButton.isClicked(mousePos));
            squadButton.setHovered(squadButton.isClicked(mousePos));
        }
        leaderboard.setHovered(leaderboard.isClicked(mousePos));
        lockerButton.setHovered(lockerButton.isClicked(mousePos));
        modeButton.setHovered(modeButton.isClicked(mousePos));
        playButton.setHovered(playButton.isClicked(mousePos));
        editorButton.setHovered(editorButton.isClicked(mousePos));
    } else if (currentState == State::SETTINGS) {
        backButton.setHovered(backButton.isClicked(mousePos));
        fps30Button.setHovered(fps30Button.isClicked(mousePos));
        fps60Button.setHovered(fps60Button.isClicked(mousePos));
        if (isDraggingVolume) {
            paramButton.setVolumeFromMouse(mousePos.x);
        }
    } else if (currentState == State::LOCKER) {
        paramButton.setHovered(paramButton.isClicked(mousePos));
        applyButtonLocker.setHovered(applyButtonLocker.isClicked(mousePos));
        leftButtonSelection.setHovered(leftButtonSelection.isClicked(mousePos));
        rightButtonSelection.setHovered(rightButtonSelection.isClicked(mousePos));
    } else if (currentState == State::CONTROLS) {
        backButton.setHovered(backButton.isClicked(mousePos));
    } else if (currentState == State::LOBBY) {
        paramButton.setHovered(paramButton.isClicked(mousePos));
    } else if (currentState == State::ERRORSERVER) {
        paramButton.setHovered(paramButton.isClicked(mousePos));
    }
}

void GameManager::handleWindowResize(sf::Event& event)
{
    Engine::Utils::Vec2UInt newSize(event.size.width, event.size.height);

    if (currentState == State::LAUNCH) {
        launch.updateWindowSize(newSize);
    } else if (currentState == State::MENU) {
        player.updateWindowSize(newSize);
    } else if (currentState == State::LOBBY) {
        player.updateWindowSize(newSize);
    } else if (currentState == State::ERRORSERVER) {
        errorServer.updateWindowSize(newSize);
    }
    updatePositions(newSize);
    paramButton.setupVolumeBar(
        sf::Vector2f(newSize.x - 220, newSize.y - 80),
        200.f
    );
    particleSystem.updateWindowSize(newSize);
    float buttonWidth = std::min(120.0f, newSize.x * 0.15f);
    float buttonX = std::min((float)(newSize.x - buttonWidth - 20), (float)(newSize.x * 0.75f));
    
    resolutionButton.updatePositionAndSize(sf::Vector2f(buttonX, 200), sf::Vector2f(buttonWidth, 30));
    displayModeButton.updatePositionAndSize(sf::Vector2f(buttonX, 250), sf::Vector2f(buttonWidth, 30));
    graphicsQualityButton.updatePositionAndSize(sf::Vector2f(buttonX, 300), sf::Vector2f(buttonWidth, 30));
    colorBlindModeButton.updatePositionAndSize(sf::Vector2f(buttonX, 350), sf::Vector2f(buttonWidth, 30));
    
    float applyButtonWidth = std::min(150.0f, newSize.x * 0.25f);
    applyResolutionButton.updatePositionAndSize(sf::Vector2f(static_cast<float>(newSize.x) / 2.0f - applyButtonWidth/2, 450), sf::Vector2f(applyButtonWidth, 35));
    
    if (currentState == State::SETTINGS) {
        updateStatusTextPosition(true);
    } else {
        updateStatusTextPosition(false);
    }
}

void GameManager::updateStatusTextPosition(bool isParametersMode)
{
    if (isParametersMode) {
        statusText.setCharacterSize(60);
        statusText.setPosition(260, 100);
        statusText.setFillColor(sf::Color::White);
    } else {
        statusText.setCharacterSize(10);
        statusText.setPosition(600, 60);
    }
}

bool GameManager::connectToServer(const std::string& serverIP, unsigned short port)
{
    networkManager = std::shared_ptr<Engine::NetworkManager>(createNetworkManagerFunc(Engine::NetworkManager::Role::CLIENT, serverIP, port));

    luaLoader.setMediator(networkManager->mediator);
    luaLoader.setNetworkManager(networkManager);
    luaLoader.registerComponentECS<Engine::Components::Sprite>();
    luaLoader.registerComponentECS<Engine::Components::Transform>();
    luaLoader.registerComponentECS<Engine::Components::RigidBody>();
    luaLoader.registerComponentECS<Engine::Components::Gravity>();
    luaLoader.registerComponentECS<Engine::Components::PlayerInfo>();
    luaLoader.registerComponentECS<Engine::Components::ShootingCooldown>();
    luaLoader.registerComponentECS<Engine::Components::Hitbox>();
    luaLoader.registerComponentECS<Engine::Components::EnemyInfo>();
    luaLoader.registerComponentECS<Engine::Components::Sound>();
    luaLoader.registerComponentECS<Engine::Components::Animation>();

    // luaLoader.executeScript("lua_collision_system.lua");
    luaLoader.loadFolder("lua_scripts");

    physics_system = networkManager->mediator->registerSystem<Engine::Systems::PhysicsNoEngineSystem>();

    {
        Engine::Signature signature;
        signature.set(networkManager->mediator->getComponentType<Engine::Components::RigidBody>());
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Transform>());
        networkManager->mediator->setSystemSignature<Engine::Systems::PhysicsNoEngineSystem>(signature);
    }

    render_system = networkManager->mediator->registerSystem<Engine::Systems::RenderSystem>();

    {
        Engine::Signature signature;
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Transform>());
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Sprite>());
        networkManager->mediator->setSystemSignature<Engine::Systems::RenderSystem>(signature);
    }
    
    player_control_system = networkManager->mediator->registerSystem<Engine::Systems::PlayerControl>();
    player_control_system->init(networkManager->mediator, [](float, float) {});

    {
        Engine::Signature signature;
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Transform>());
        signature.set(networkManager->mediator->getComponentType<Engine::Components::PlayerInfo>());
        networkManager->mediator->setSystemSignature<Engine::Systems::PlayerControl>(signature);
    }

    // collision_system = networkManager->mediator->registerSystem<Engine::Systems::Collision>();

    // {
    //     Engine::Signature signature;
    //     signature.set(networkManager->mediator->getComponentType<Engine::Components::Transform>());
    //     signature.set(networkManager->mediator->getComponentType<Engine::Components::Hitbox>());
    //     networkManager->mediator->setSystemSignature<Engine::Systems::Collision>(signature);
    // }

    enemy_system = networkManager->mediator->registerSystem<Engine::Systems::EnemySystem>();
    enemy_system->init(networkManager);

    {
        Engine::Signature signature;
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Transform>());
        signature.set(networkManager->mediator->getComponentType<Engine::Components::RigidBody>());
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Hitbox>());
        signature.set(networkManager->mediator->getComponentType<Engine::Components::EnemyInfo>());
        networkManager->mediator->setSystemSignature<Engine::Systems::EnemySystem>(signature);
    }

    sound_system = networkManager->mediator->registerSystem<Engine::Systems::SoundSystem>();

    {
        Engine::Signature signature;
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Sound>());
        networkManager->mediator->setSystemSignature<Engine::Systems::SoundSystem>(signature);
    }

    dev_console_system = networkManager->mediator->registerSystem<Engine::Systems::DevConsole>();
    dev_console_system->init(networkManager->mediator);

    animate_system = networkManager->mediator->registerSystem<Engine::Systems::Animate>();

    {
        Engine::Signature signature;
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Sprite>());
        signature.set(networkManager->mediator->getComponentType<Engine::Components::Animation>());
        networkManager->mediator->setSystemSignature<Engine::Systems::Animate>(signature);
    }

    // TEMP
    networkManager->sendHello(UsernameGame, 12345);

    return (true);
}

void GameManager::cycleResolution()
{
    ResolutionMode currentRes = parameters.getCurrentResolution();
    ResolutionMode nextRes;
    
    switch (currentRes) {
        case ResolutionMode::RES_800x600:
            nextRes = ResolutionMode::RES_1280x720;
            break;
        case ResolutionMode::RES_1280x720:
            nextRes = ResolutionMode::RES_1920x1080;
            break;
        case ResolutionMode::RES_1920x1080:
            nextRes = ResolutionMode::RES_800x600;
            break;
        default:
            nextRes = ResolutionMode::RES_800x600;
            break;
    }
    
    parameters.setResolution(nextRes);
}

void GameManager::cycleDisplayMode(sf::RenderWindow& window)
{
    DisplayMode currentMode = parameters.getCurrentDisplayMode();
    DisplayMode nextMode;
    
    switch (currentMode) {
        case DisplayMode::WINDOWED:
            nextMode = DisplayMode::FULLSCREEN;
            break;
        case DisplayMode::FULLSCREEN:
            nextMode = DisplayMode::WINDOWED;
            break;
        default:
            nextMode = DisplayMode::WINDOWED;
            break;
    }
    
    parameters.setDisplayMode(nextMode);
    sf::Vector2u sfml_size = window.getSize();
    Engine::Utils::Vec2UInt currentSize(sfml_size.x, sfml_size.y);
    switch (nextMode) {
        case DisplayMode::WINDOWED:
            window.create(sf::VideoMode(currentSize.x, currentSize.y), "R-Type Client", sf::Style::Default);
            break;
        case DisplayMode::FULLSCREEN:
            window.create(sf::VideoMode::getDesktopMode(), "R-Type Client", sf::Style::Fullscreen);
            break;
    }
    window.setFramerateLimit(currentFps);
}

void GameManager::cycleGraphicsQuality()
{
    GraphicsQuality currentQuality = parameters.getCurrentGraphicsQuality();
    GraphicsQuality nextQuality;
    
    switch (currentQuality) {
        case GraphicsQuality::LOW:
            nextQuality = GraphicsQuality::MEDIUM;
            break;
        case GraphicsQuality::MEDIUM:
            nextQuality = GraphicsQuality::HIGH;
            break;
        case GraphicsQuality::HIGH:
            nextQuality = GraphicsQuality::ULTRA;
            break;
        case GraphicsQuality::ULTRA:
            nextQuality = GraphicsQuality::LOW;
            break;
        default:
            nextQuality = GraphicsQuality::MEDIUM;
            break;
    }
    
    parameters.setGraphicsQuality(nextQuality);
    switch (nextQuality) {
        case GraphicsQuality::LOW:
            particleSystem.setMaxParticles(100);
            break;
        case GraphicsQuality::MEDIUM:
            particleSystem.setMaxParticles(200);
            break;
        case GraphicsQuality::HIGH:
            particleSystem.setMaxParticles(300);
            break;
        case GraphicsQuality::ULTRA:
            particleSystem.setMaxParticles(500);
            break;
    }
}

void GameManager::cycleColorBlindMode()
{
    ColorBlindMode currentMode = parameters.getCurrentColorBlindMode();
    ColorBlindMode nextMode;
    
    switch (currentMode) {
        case ColorBlindMode::NORMAL:
            nextMode = ColorBlindMode::PROTANOPIA;
            break;
        case ColorBlindMode::PROTANOPIA:
            nextMode = ColorBlindMode::DEUTERANOPIA;
            break;
        case ColorBlindMode::DEUTERANOPIA:
            nextMode = ColorBlindMode::TRITANOPIA;
            break;
        case ColorBlindMode::TRITANOPIA:
            nextMode = ColorBlindMode::MONOCHROME;
            break;
        case ColorBlindMode::MONOCHROME:
            nextMode = ColorBlindMode::NORMAL;
            break;
        default:
            nextMode = ColorBlindMode::NORMAL;
            break;
    }
    
    parameters.setColorBlindMode(nextMode);
}

void GameManager::applyCurrentResolution(sf::RenderWindow& window)
{
    ResolutionMode currentRes = parameters.getCurrentResolution();
    Engine::Utils::Vec2UInt newSize = parameters.getResolutionSize(currentRes);
    
    DisplayMode currentMode = parameters.getCurrentDisplayMode();
    if (currentMode == DisplayMode::WINDOWED) {
        window.create(sf::VideoMode(newSize.x, newSize.y), "R-Type Client", sf::Style::Default);
    } else {
        window.create(sf::VideoMode::getDesktopMode(), "R-Type Client", sf::Style::Fullscreen);
    }
    
    window.setFramerateLimit(currentFps);
    sf::Event resizeEvent;
    resizeEvent.type = sf::Event::Resized;
    resizeEvent.size.width = window.getSize().x;
    resizeEvent.size.height = window.getSize().y;
    handleWindowResize(resizeEvent);
}

void GameManager::gameDemo(sf::RenderWindow &window)
{
    if (window.isOpen())
        window.close();

    // renderer = std::make_shared<Engine::Renderers::SFML>();

    #if defined(_WIN32)
    const std::string renderer_lib_name = "librenderer.dll";
    const std::string audio_player_lib_name = "libaudioplayer.dll";
    #else
    const std::string renderer_lib_name = "librenderer.so";
    const std::string audio_player_lib_name = "libaudioplayer.so";
    #endif

    {
        auto createRendererFunc = renderer_loader.getFunction<Engine::Renderer*(*)()>(renderer_lib_name, "createRenderer");
        renderer = std::shared_ptr<Engine::Renderer>(createRendererFunc());
    }

    {
        auto createAudioPlayerFunc = audio_player_loader.getFunction<Engine::AudioPlayer*(*)()>(audio_player_lib_name, "createAudioPlayer");
        audio_player = std::shared_ptr<Engine::AudioPlayer>(createAudioPlayerFunc());
    }

    renderer->createWindow(800, 600, "R du TYPE");

    std::shared_ptr<Engine::Mediator> mediator = networkManager->mediator;

    render_system->addTexture(renderer, "players_sprite_sheet", "assets/sprites/vaisseaux.gif");
    render_system->addTexture(renderer, "base_player_sprite_sheet", "assets/sprites/r-typesheet1.gif");
    render_system->addTexture(renderer, "ground_enemy_sprite_sheet", "assets/sprites/r-typesheet7.gif");
    render_system->addTexture(renderer, "small_enemy_sprite_sheet", "assets/sprites/r-typesheet8.gif");
    render_system->addTexture(renderer, "space_background_texture", "assets/sprites/space_background.gif");
    render_system->addTexture(renderer, "stage2_background_texture", "assets/sprites/stage2_background.png");
    render_system->addTexture(renderer, "enemy_explosion_sheet", "assets/sprites/explosionEnemy1.gif");

    render_system->addSprite(renderer, "player_1", "players_sprite_sheet", {32, 17}, {0, 0}, 5, 1);
    render_system->addSprite(renderer, "player_2", "players_sprite_sheet", {32, 17}, {0, 17}, 5, 1);
    render_system->addSprite(renderer, "player_3", "players_sprite_sheet", {32, 17}, {0, 34}, 5, 1);
    render_system->addSprite(renderer, "player_4", "players_sprite_sheet", {32, 17}, {0, 51}, 5, 1);
    render_system->addSprite(renderer, "player_5", "players_sprite_sheet", {32, 17}, {0, 68}, 5, 1);
    render_system->addSprite(renderer, "weak_player_projectile", "base_player_sprite_sheet", {16, 4}, {249, 90}, 1, 1);
    render_system->addSprite(renderer, "ground_enemy", "ground_enemy_sprite_sheet", {33, 33}, {0, 0}, 1, 1);
    render_system->addSprite(renderer, "small_enemy", "small_enemy_sprite_sheet", {33, 33}, {0, 0}, 1, 1);
    render_system->addSprite(renderer, "space_background", "space_background_texture", {1226, 207}, {0, 0}, 1, 1);
    render_system->addSprite(renderer, "stage2_background", "stage2_background_texture", {1226, 207}, {0, 0}, 1, 1);
    render_system->addSprite(renderer, "enemy_explosion", "enemy_explosion_sheet", {32, 32}, {0, 0}, 5, 1);

    sound_system->addSound(audio_player, "background_music", "assets/sound/Start_sound.mp3");
    sound_system->addSound(audio_player, "projectile_shoot", "assets/sound/01LASER.BD_00000.wav");
    sound_system->addSound(audio_player, "explosion", "assets/sound/explosion.mp3");
    if (audio_player) {
        audio_player->playAudio("background_music", true);
        currentPlayingMusicId = "background_music";
    }

    const float TARGET_FPS = static_cast<float>(currentFps);
    const float FIXED_DT = 1.0f / TARGET_FPS;
    const auto FRAME_DURATION = std::chrono::microseconds(static_cast<long>(1000000.0f / TARGET_FPS));
    
    float accumulator = 0.0f;
    auto previous_time = std::chrono::high_resolution_clock::now();
    auto frame_start_time = previous_time;

    int frame_count = 0;
    float fps = 0.0f;
    float fps_timer = 0.0f;

    renderer->loadFont("dev", "assets/dev.ttf");
    renderer->loadFont("basic", "assets/r-type.otf");

    auto read_total_cpu = []() -> unsigned long long {
        std::ifstream file("/proc/stat");
        if (!file) return 0ULL;
        std::string line;
        std::getline(file, line);
        std::istringstream ss(line);
        std::string cpu_label;
        ss >> cpu_label; // skip "cpu"
        unsigned long long value = 0ULL;
        unsigned long long sum = 0ULL;
        while (ss >> value) sum += value;
        return sum;
    };

    auto read_proc_cpu = []() -> unsigned long long {
        std::ifstream file("/proc/self/stat");
        if (!file) return 0ULL;
        std::string content;
        std::getline(file, content);
        std::istringstream ss(content);
        std::string token;
        for (int i = 1; i <= 13; ++i) {
            if (!(ss >> token)) return 0ULL;
        }
        unsigned long long utime = 0ULL, stime = 0ULL;
        ss >> utime >> stime;
        return utime + stime;
    };

    auto read_proc_mem_kb = []() -> unsigned long long {
        std::ifstream file("/proc/self/status");
        if (!file) return 0ULL;
        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("VmRSS:", 0) == 0) {
                std::istringstream ss(line);
                std::string key, unit;
                unsigned long long val = 0ULL;
                ss >> key >> val >> unit;
                return val;
            }
        }
        return 0ULL;
    };

    unsigned long long prev_total_cpu = read_total_cpu();
    unsigned long long prev_proc_cpu = read_proc_cpu();
    double cpu_percent = 0.0;
    unsigned long long mem_kb = read_proc_mem_kb();

    uint32_t current_player_id = 0;
    bool is_player_id_set = false;

    while (renderer->isWindowOpen()) {
        frame_start_time = std::chrono::high_resolution_clock::now();
        
        renderer->clearWindow();
        renderer->handleEvents(networkManager);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float>(currentTime - previous_time).count();

        frameTime = std::min(frameTime, FIXED_DT * 5.0f);
        
        previous_time = currentTime;
        accumulator += frameTime;

        frame_count++;
        fps_timer += frameTime;
        if (fps_timer >= 1.0f) {
            fps = frame_count / fps_timer;
            frame_count = 0;
            fps_timer = 0.0f;
        }

        static bool clientGameOver = false;
        while (accumulator >= FIXED_DT && !clientGameOver) {
            player_control_system->update(networkManager, FIXED_DT);
            physics_system->update(mediator, FIXED_DT);
            enemy_system->update(networkManager, FIXED_DT);
            
            for (const auto &scriptName : luaLoader.getLoadedScriptNames()) {
                luaLoader.executeLuaFunctionInScript(scriptName, "update");
            }

            animate_system->update(mediator, FIXED_DT);
            accumulator -= FIXED_DT;
        }
        
        sound_system->update(audio_player, mediator);

        render_system->drawBackgrounds(renderer, mediator, frameTime);
        {
            sf::Color overlay = parameters.getOverlayColor(this->currentLevel);
            if (overlay.a > 0 && renderer) {
                float winW = static_cast<float>(renderer->getWindowWidth());
                float winH = static_cast<float>(renderer->getWindowHeight());
                Engine::Utils::Rect rect(0.0f, 0.0f, winW, winH);
                unsigned int packed = (static_cast<unsigned int>(overlay.r) << 24) |
                                      (static_cast<unsigned int>(overlay.g) << 16) |
                                      (static_cast<unsigned int>(overlay.b) << 8)  |
                                      (static_cast<unsigned int>(overlay.a));
                renderer->drawRectangle(rect, packed);
            }
        }
        render_system->drawEntities(renderer, mediator, frameTime);
        if (mediator->hasComponent<Engine::Components::GameState>(0)) {
            const auto &g = mediator->getComponent<Engine::Components::GameState>(0);
            if (g.state == static_cast<uint8_t>(Engine::Components::GameStateEnum::GAME_VICTORY)) {
                clientGameOver = true;
            }
        }

        bool found = false;
        for (uint32_t e = 0; e < MAX_ENTITIES; ++e) {
            if (!mediator->hasComponent<Engine::Components::PlayerInfo>(e))
                continue;
            if (!mediator->hasComponent<Engine::Components::LevelInfo>(e))
                continue;
            const auto &linfo = mediator->getComponent<Engine::Components::LevelInfo>(e);
            if (this->currentLevel != linfo.level) {
                if (!levelWipeActive) {
                    std::cout << "[HUD] Starting level wipe to " << linfo.level << " from entity " << e << std::endl;
                    startLevelWipe(linfo.level);
                } else {
                    std::cout << "[HUD] Level change requested while wipe active, ignoring: " << linfo.level << std::endl;
                }
            }
            found = true;
            break;
        }
        if (!found) {
            for (uint32_t e = 0; e < MAX_ENTITIES; ++e) {
                if (mediator->hasComponent<Engine::Components::GameState>(e)) {
                    const auto &g = mediator->getComponent<Engine::Components::GameState>(e);
                    if (g.state == static_cast<uint8_t>(Engine::Components::GameStateEnum::GAME_VICTORY)) {
                        clientGameOver = true;
                    }
                }
                if (!mediator->hasComponent<Engine::Components::LevelInfo>(e))
                    continue;
                const auto &linfo = mediator->getComponent<Engine::Components::LevelInfo>(e);
                if (this->currentLevel != linfo.level) {
                    if (!levelWipeActive) {
                        std::cout << "[HUD] Starting level wipe (fallback) to " << linfo.level << " from entity " << e << std::endl;
                        startLevelWipe(linfo.level);
                    } else {
                        std::cout << "[HUD] Level change requested while wipe active (fallback), ignoring: " << linfo.level << std::endl;
                    }
                }
                break;
            }
        }
        if (levelWipeActive && renderer) {
            auto now = std::chrono::high_resolution_clock::now();
            float elapsed = std::chrono::duration<float>(now - levelWipeStart).count();
            float total = levelWipeDuration;
            float hold = levelWipeHoldDuration;
            float pre = (total - hold) / 2.0f;
            float post = pre;
            float winW = static_cast<float>(renderer->getWindowWidth());
            float winH = static_cast<float>(renderer->getWindowHeight());
            float coverFrac = 0.0f;
            if (elapsed < 0.0f) elapsed = 0.0f;
            if (elapsed < pre) {
                coverFrac = (elapsed / pre);
            } else if (elapsed < pre + hold) {
                coverFrac = 1.0f;
                if (!levelWipeMidApplied) {
                    applyBackgroundSwap(levelWipeTarget);
                    applyLevelMusic(levelWipeTarget);
                    levelWipeMidApplied = true;
                    this->currentLevel = levelWipeTarget;
                }
            } else if (elapsed < pre + hold + post) {
                float after = elapsed - pre - hold;
                coverFrac = 1.0f - (after / post);
            } else {
                coverFrac = 0.0f;
            }
            if (coverFrac < 0.0f) coverFrac = 0.0f;
            if (coverFrac > 1.0f) coverFrac = 1.0f;
            float coverW = coverFrac * winW;
            Engine::Utils::Rect rect(0.0f, 0.0f, coverW, winH);
            renderer->drawRectangle(rect, levelWipeColor);
            if (elapsed >= total) {
                levelWipeActive = false;
                levelWipePreloadMusicId.clear();
                levelWipePreloadTextureId.clear();
                levelWipePreloadSpriteId.clear();
            }
        }

        if (renderer) {
            const unsigned int fontSize = 20;
            const float padding = 10.0f;
            float winW = static_cast<float>(renderer->getWindowWidth());
            float winH = static_cast<float>(renderer->getWindowHeight());
            float levelX = std::max(0.0f, winW - 250.0f - padding);
            float levelY = std::max(0.0f, winH - 40.0f - padding);
            if (!clientGameOver) {
                renderer->drawText("basic", std::string("Level : ") + std::to_string(this->currentLevel), levelX, levelY, fontSize, 0xFFFF00FF);
            } else {
                if (audio_player && !currentPlayingMusicId.empty()) {
                    audio_player->stopAudio(currentPlayingMusicId);
                    audio_player->unloadAudio(currentPlayingMusicId);
                    currentPlayingMusicId.clear();
                }
                float winW = static_cast<float>(renderer->getWindowWidth());
                float winH = static_cast<float>(renderer->getWindowHeight());
                Engine::Utils::Rect rect(0.0f, 0.0f, winW, winH);
                renderer->drawRectangle(rect, 0x000000FF);
                const unsigned int bigSize = 48;
                float textX = winW / 2.0f - 200.0f;
                float textY = winH / 2.0f - 24.0f;
                renderer->drawText("basic", "Winner", textX, textY, bigSize, 0xFFFFFFFF);
            }
        }
        if (player_control_system && !is_player_id_set) {
            current_player_id = networkManager->player_id;
            player_control_system->setCurrentPlayerID(current_player_id);
            is_player_id_set = true;
            player_control_system->resetGameOver();
        }

        if (player_control_system && is_player_id_set) {
            if (!player_control_system->isGameOver()) {
                int localHealth = player_control_system->getLocalPlayerHealth(networkManager);
                if (localHealth >= 0) {
                    renderer->drawText("basic", "Lives " + std::to_string(localHealth), 10.0f, 30.0f, 20, 0xFFFFFFFF);
                }
            } else {
                renderer->drawText("basic", "GAME OVER", 30.0f, 100.0f, 80, 0xFFFFFFFF);
                float game_over_time = player_control_system->getGameOverTime();
                renderer->drawText("basic", "Returning to menu..." + std::to_string(static_cast<int>(game_over_time)), 30.0f, 200.0f, 20, 0xFFFFFFFF);
                
                if (game_over_time >= 5.0f) {
                    player_control_system->resetGameOver();
                    return;
                }
            }
        }

        {
            unsigned long long total_cpu = read_total_cpu();
            unsigned long long proc_cpu = read_proc_cpu();
            if (total_cpu > prev_total_cpu) {
                cpu_percent = static_cast<double>(proc_cpu - prev_proc_cpu) * 100.0 / static_cast<double>(total_cpu - prev_total_cpu);
            } else {
                cpu_percent = 0.0;
            }
            prev_total_cpu = total_cpu;
            prev_proc_cpu = proc_cpu;

            mem_kb = read_proc_mem_kb();

            std::ostringstream hud_ss;
            hud_ss << mediator->getEntityCount() << " entites | FPS " << static_cast<int>(fps)
                   << " | CPU " << std::fixed << std::setprecision(1) << cpu_percent << "%"
                   << " | RAM " << std::fixed << std::setprecision(1) << (static_cast<double>(mem_kb) / 1024.0) << "MB";
            std::string hud_str = hud_ss.str();
            renderer->drawText("basic", hud_str, 0.0f, 0.0f, 20, 0x00FF00FF);
        }

        dev_console_system->update(networkManager, renderer);
        {
            sf::Color overlay = parameters.getOverlayColor(this->currentLevel);
            if (overlay.a > 0 && renderer) {
                float winW = static_cast<float>(renderer->getWindowWidth());
                float winH = static_cast<float>(renderer->getWindowHeight());
                Engine::Utils::Rect rect(0.0f, 0.0f, winW, winH);
                unsigned int packed = (static_cast<unsigned int>(overlay.r) << 24) |
                                      (static_cast<unsigned int>(overlay.g) << 16) |
                                      (static_cast<unsigned int>(overlay.b) << 8)  |
                                      (static_cast<unsigned int>(overlay.a));
                renderer->drawRectangle(rect, packed);
            }
        }
        renderer->displayWindow();

        auto frame_end_time = std::chrono::high_resolution_clock::now();
        auto frame_processing_time = frame_end_time - frame_start_time;
        
        if (frame_processing_time < FRAME_DURATION) {
            auto sleepTime = FRAME_DURATION - frame_processing_time;
            std::this_thread::sleep_for(sleepTime);
        }
    }
}
