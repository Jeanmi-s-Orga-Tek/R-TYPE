/*
** EPITECH PROJECT, 2025
** G-CPP-500-BDX-5-1-rtype-1
** File description:
** Leaderboard
*/

#include "Leaderboard.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

Leaderboard::Leaderboard(Engine::Utils::Vec2UInt windowSize) : windowSize(windowSize)
{
}

bool Leaderboard::loadResources()
{
    std::ifstream verifyLeaderboardFile("Client/UserLeaderboard.md");

    if (!trophyTexture.loadFromFile("assets/sprites/trophy.png")) {
        std::cerr << "Erreur" << std::endl;
        return false;
    }
    if (verifyLeaderboardFile.fail()) {
        std::ofstream leaderboardFile("Client/UserLeaderboard.md", std::ios::out);
        if (leaderboardFile.fail()) {
            std::cerr << "Erreur creation de fichier" << std::endl;
            return false;
        }
        leaderboardFile.close();
    }

    if (!font.loadFromFile("assets/r-type.otf")) {
        std::cerr << "Error loading font" << std::endl;
        return false;
    }

    trophySprite.setTexture(trophyTexture);
    trophyRect = sf::IntRect(0, 0, 141, 143);

    trophySprite.setTextureRect(trophyRect);
    trophySprite.setScale(0.3f, 0.3f);

    centerImage();
    loadScores();

    return true;
}

std::string Leaderboard::getCurrentDate()
{
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Leaderboard::saveScore(const std::string& username, int score)
{
    LeaderboardEntry newEntry;
    newEntry.username = username;
    newEntry.score = score;
    newEntry.date = getCurrentDate();
    
    entries.push_back(newEntry);
    std::sort(entries.begin(), entries.end(), std::greater<LeaderboardEntry>());
    
    if (entries.size() > MAX_ENTRIES) {
        entries.resize(MAX_ENTRIES);
    }
    
    std::ofstream file("Client/UserLeaderboard.md", std::ios::out | std::ios::trunc);
    if (file.is_open()) {
        file << "# R-Type Leaderboard\n\n";
        file << "| Rank | Username | Score | Date |\n";
        file << "|------|----------|-------|------|\n";
        
        for (size_t i = 0; i < entries.size(); ++i) {
            file << "| " << (i + 1) << " | " 
                 << entries[i].username << " | " 
                 << entries[i].score << " | " 
                 << entries[i].date << " |\n";
        }
        
        file.close();
    }
}

void Leaderboard::loadScores()
{
    entries.clear();
    std::ifstream file("Client/UserLeaderboard.md");
    
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    bool inTable = false;
    
    while (std::getline(file, line)) {
        if (line.find("| Rank |") != std::string::npos) {
            inTable = true;
            std::getline(file, line);
            continue;
        }
        
        if (inTable && line.find('|') != std::string::npos) {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;
            
            while (std::getline(iss, token, '|')) {
                token.erase(0, token.find_first_not_of(" \t\r\n"));
                token.erase(token.find_last_not_of(" \t\r\n") + 1);
                if (!token.empty()) {
                    tokens.push_back(token);
                }
            }
            
            if (tokens.size() >= 4) {
                LeaderboardEntry entry;
                entry.username = tokens[1];
                try {
                    entry.score = std::stoi(tokens[2]);
                } catch (...) {
                    continue;
                }
                entry.date = tokens[3];
                entries.push_back(entry);
            }
        }
    }
    
    file.close();
    std::sort(entries.begin(), entries.end(), std::greater<LeaderboardEntry>());
}

void Leaderboard::centerImage()
{
    sf::FloatRect trophyBounds = trophySprite.getGlobalBounds();

    float centerXS = (windowSize.x - trophyBounds.width) / 2.0f;
    float centerYS = (windowSize.y - trophyBounds.height) / 2.0f;


    trophySprite.setPosition(centerXS, centerYS);
}

void Leaderboard::drawRoundedRectangle(sf::RenderWindow& window)
{
    sf::Vector2f rectPos = leaderboardRectangle.getPosition();
    sf::Vector2f rectSize = leaderboardRectangle.getSize();
    float radius = 40.f;

    sf::Color yellow = sf::Color::Yellow;

    sf::RectangleShape top(sf::Vector2f(rectSize.x - 2 * radius, radius));
    top.setPosition(rectPos.x + radius, rectPos.y);
    top.setFillColor(yellow);
    top.setOutlineThickness(0);
    window.draw(top);

    sf::RectangleShape bottom(sf::Vector2f(rectSize.x - 2 * radius, radius));
    bottom.setPosition(rectPos.x + radius, rectPos.y + rectSize.y - radius);
    bottom.setFillColor(yellow);
    bottom.setOutlineThickness(0);
    window.draw(bottom);

    sf::RectangleShape left(sf::Vector2f(radius, rectSize.y - 2 * radius));
    left.setPosition(rectPos.x, rectPos.y + radius);
    left.setFillColor(yellow);
    left.setOutlineThickness(0);
    window.draw(left);

    sf::RectangleShape right(sf::Vector2f(radius, rectSize.y - 2 * radius));
    right.setPosition(rectPos.x + rectSize.x - radius, rectPos.y + radius);
    right.setFillColor(yellow);
    right.setOutlineThickness(0);
    window.draw(right);

    sf::CircleShape corner(radius);
    corner.setFillColor(yellow);
    corner.setOutlineThickness(0);

    corner.setPosition(rectPos.x, rectPos.y);
    window.draw(corner);
    corner.setPosition(rectPos.x + rectSize.x - 2 * radius, rectPos.y);
    window.draw(corner);
    corner.setPosition(rectPos.x, rectPos.y + rectSize.y - 2 * radius);
    window.draw(corner);
    corner.setPosition(rectPos.x + rectSize.x - 2 * radius, rectPos.y + rectSize.y - 2 * radius);
    window.draw(corner);

    sf::RectangleShape center(sf::Vector2f(rectSize.x - 2 * radius, rectSize.y - 2 * radius));
    center.setPosition(rectPos.x + radius, rectPos.y + radius);
    center.setFillColor(sf::Color::Black);
    center.setOutlineThickness(0);
    window.draw(center);
}

void Leaderboard::draw(sf::RenderWindow& window)
{
    sf::FloatRect rect = leaderboardRectangle.getGlobalBounds();
    
    leaderboardRectangle.setFillColor(sf::Color(40, 40, 60, 220));
    leaderboardRectangle.setOutlineThickness(2.f);
    leaderboardRectangle.setOutlineColor(sf::Color::Yellow);
    // drawRoundedRectangle(window);
    
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("LEADERBOARD");
    titleText.setCharacterSize(18);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setStyle(sf::Text::Bold);
    
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
    titleText.setPosition(rect.left + rect.width / 2.0f, rect.top + 30.0f);
    window.draw(titleText);
    
    if (entries.empty()) {
        sf::Text noScoresText;
        noScoresText.setFont(font);
        noScoresText.setString("No scores yet!\nPlay to set a record!");
        noScoresText.setCharacterSize(14);
        noScoresText.setFillColor(sf::Color::White);
        
        sf::FloatRect noScoresBounds = noScoresText.getLocalBounds();
        noScoresText.setOrigin(noScoresBounds.left + noScoresBounds.width / 2.0f, noScoresBounds.top + noScoresBounds.height / 2.0f);
        noScoresText.setPosition(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
        window.draw(noScoresText);
    } else {
        float startY = rect.top + 60.0f;
        float lineHeight = 28.0f;
        
        for (size_t i = 0; i < entries.size() && i < 10; ++i) {
            sf::Color rankColor = sf::Color::White;
            if (i == 0) rankColor = sf::Color(255, 215, 0);
            else if (i == 1) rankColor = sf::Color(192, 192, 192);
            else if (i == 2) rankColor = sf::Color(205, 127, 50);
            
            sf::Text rankText;
            rankText.setFont(font);
            rankText.setString(std::to_string(i + 1));
            rankText.setCharacterSize(14);
            rankText.setFillColor(rankColor);
            rankText.setPosition(rect.left + 25.0f, startY + i * lineHeight);
            window.draw(rankText);
            
            sf::Text usernameText;
            usernameText.setFont(font);
            usernameText.setString(entries[i].username);
            usernameText.setCharacterSize(12);
            usernameText.setFillColor(sf::Color::White);
            usernameText.setPosition(rect.left + 55.0f, startY + i * lineHeight);
            window.draw(usernameText);
            
            sf::Text scoreText;
            scoreText.setFont(font);
            scoreText.setString(std::to_string(entries[i].score));
            scoreText.setCharacterSize(12);
            scoreText.setFillColor(sf::Color::Yellow);
            scoreText.setPosition(rect.left + rect.width - 80.0f, startY + i * lineHeight);
            window.draw(scoreText);
        }
    }
}

void Leaderboard::handleEvent(const sf::Event& event, sf::RenderWindow& window)
{
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            window.close();
        }
    }

    if (event.type == sf::Event::Resized) {
        windowSize.x = event.size.width;
        windowSize.y = event.size.height;
        centerImage();
    }
}

void Leaderboard::update()
{
}

void Leaderboard::updateWindowSize(Engine::Utils::Vec2UInt newSize)
{
    windowSize = newSize;
    centerImage();
}
