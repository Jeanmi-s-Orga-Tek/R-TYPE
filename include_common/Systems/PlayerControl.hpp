/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** PlayerControl
*/

#ifndef PLAYERCONTROL_HPP_
#define PLAYERCONTROL_HPP_

#include <functional>
#include <memory>
#include <bitset>
#include <unordered_map>

#include "System.hpp"
#include "Mediator.hpp"
#include "NetworkManager.hpp"
#include "Event.hpp"

#include "Components/Transform.hpp"
#include "Components/RigidBody.hpp"
#include "Components/PlayerInfo.hpp"
#include "Components/Sprite.hpp"
#include "Components/ShootingCooldown.hpp"

namespace Engine {
    namespace Systems {
        class PlayerControl : public System {
            public:
                void init(std::shared_ptr<Mediator> mediator, const std::function<void(float, float)> &projCreator) {
                    mediator->addEventListener(static_cast<EventId>(EventsIds::PLAYER_INPUT), [this](Event &event) { this->inputListener(event); });

                    playerProjectileCreator = projCreator;
                }

                void update(std::shared_ptr<NetworkManager> networkManager, float dt) {
                    std::shared_ptr<Engine::Mediator> mediator = networkManager->mediator;
                    
                    if (game_over_state) {
                        game_over_time += dt;
                    }

                    bool local_player_exists = false;
                    for (const auto &entity : entities) {
                        auto &player_info = mediator->getComponent<Components::PlayerInfo>(entity);
                        if (player_info.player_id == networkManager->player_id) {
                            local_player_exists = true;
                            local_player_has_ever_existed = true;
                            last_known_score = player_info.score;
                            break;
                        }
                    }

                    if (!local_player_exists && local_player_has_ever_existed && !game_over_state) {
                        game_over_state = true;
                        game_over_time = 0.0f;
                    }
                    
                    for (const auto &entity : entities) {
                        auto &player_info = mediator->getComponent<Components::PlayerInfo>(entity);
                        auto &player_cooldown = mediator->getComponent<Components::ShootingCooldown>(entity);

                        if (player_cooldown.cooldown > 0)
                            player_cooldown.cooldown--;

                        uint32_t player_id = player_info.player_id;

                        if (player_id == networkManager->player_id) {
                            last_known_score = player_info.score;
                            if (player_info.health <= 0 && !game_over_state) {
                                game_over_state = true;
                                game_over_time = 0.0f;
                            }
                        }

                        auto it = player_id_to_buttons.find(player_id);
                        if (it == player_id_to_buttons.end())
                            continue;

                        const auto &buttons = it->second;

                        auto &transform = mediator->getComponent<Components::Transform>(entity);

                        const float accel_rate = 200.0f;

                        bool has_moved = false;

                        if (buttons.test(static_cast<std::size_t>(InputButtons::LEFT))) {
                            transform.pos.x -= accel_rate * dt;
                            has_moved = true;
                        }
                        if (buttons.test(static_cast<std::size_t>(InputButtons::RIGHT))) {
                            transform.pos.x += accel_rate * dt;
                            has_moved = true;
                        }
                        if (buttons.test(static_cast<std::size_t>(InputButtons::UP))) {
                            transform.pos.y -= accel_rate * dt;
                            has_moved = true;
                        }
                        if (buttons.test(static_cast<std::size_t>(InputButtons::DOWN))) {
                            transform.pos.y += accel_rate * dt;
                            has_moved = true;
                        }
                        if (buttons.test(static_cast<std::size_t>(InputButtons::SHOOT))) {
                            if (player_cooldown.cooldown <= 0) {
                                if (networkManager->getRole() == NetworkManager::Role::SERVER) {
                                    // networkManager->createPlayerProjectile(transform.pos.x, transform.pos.y);
                                    playerProjectileCreator(transform.pos.x, transform.pos.y);
                                }
                                player_cooldown.cooldown = player_cooldown.cooldown_time;
                            }
                        }

                        if (has_moved) {
                            networkManager->sendComponent(entity, transform);
                        }

                        // auto &rigidbody = mediator->getComponent<Components::RigidBody>(entity);
                        // auto &transform = mediator->getComponent<Components::Transform>(entity);

                        // const float accel_rate = 5000.0f;
                        // const float friction = 1500.0f;
                        // const float max_speed = 400.0f;

                        // Utils::Vec2 input_accel;

                        // if (buttons.test(static_cast<std::size_t>(InputButtons::LEFT))) {
                        //     input_accel.x -= accel_rate;
                        // }
                        // if (buttons.test(static_cast<std::size_t>(InputButtons::RIGHT))) {
                        //     input_accel.x += accel_rate;
                        // }
                        // if (buttons.test(static_cast<std::size_t>(InputButtons::UP))) {
                        //     input_accel.y -= accel_rate;
                        // }
                        // if (buttons.test(static_cast<std::size_t>(InputButtons::DOWN))) {
                        //     input_accel.y += accel_rate;
                        // }

                        // rigidbody.acceleration = input_accel;

                        // if (input_accel.x == 0) {
                        //     if (rigidbody.velocity.x > 0) {
                        //         rigidbody.acceleration.x -= friction;
                        //         if (rigidbody.velocity.x < 0)
                        //             rigidbody.velocity.x = 0;
                        //     } else if (rigidbody.velocity.x < 0) {
                        //         rigidbody.acceleration.x += friction;
                        //         if (rigidbody.velocity.x > 0)
                        //             rigidbody.velocity.x = 0;
                        //     }
                        // }
                        // if (input_accel.y == 0) {
                        //     if (rigidbody.velocity.y > 0) {
                        //         rigidbody.acceleration.y -= friction;
                        //         if (rigidbody.velocity.y < 0)
                        //             rigidbody.velocity.y = 0;
                        //     } else if (rigidbody.velocity.y < 0) {
                        //         rigidbody.acceleration.y += friction;
                        //         if (rigidbody.velocity.y > 0)
                        //             rigidbody.velocity.y = 0;
                        //     }
                        // }

                        // rigidbody.velocity += rigidbody.acceleration * dt;

                        // if (rigidbody.velocity.x > max_speed)
                        //     rigidbody.velocity.x = max_speed;
                        // if (rigidbody.velocity.x < -max_speed)
                        //     rigidbody.velocity.x = -max_speed;
                        // if (rigidbody.velocity.y > max_speed)
                        //     rigidbody.velocity.y = max_speed;
                        // if (rigidbody.velocity.y < -max_speed)
                        //     rigidbody.velocity.y = -max_speed;

                        // transform.pos += rigidbody.velocity * dt;
                    }
                }

                // void inputListener(Event &event) {
                //     buttons = event.getParam<std::bitset<5>>(0);
                // }

                void inputListener(Event &event) {
                    uint32_t player_id = event.getParam<uint32_t>(0);
                    std::bitset<5> buttons = event.getParam<std::bitset<5>>(1);
                    player_id_to_buttons[player_id] = buttons;
                }

                void setCurrentPlayerID(uint32_t playerID) {
                    current_player_id = playerID;
                }

                bool isGameOver() const {
                    return (game_over_state);
                }

                float getGameOverTime() const {
                    return (game_over_time);
                }

                void resetGameOver() {
                    game_over_state = false;
                    game_over_time = 0.0f;
                    local_player_has_ever_existed = false;
                    last_known_score = 0;
                }

                int getCurrentPlayerHealth(std::shared_ptr<NetworkManager> networkManager) const {
                    std::shared_ptr<Engine::Mediator> mediator = networkManager->mediator;
                    for (const auto &entity : entities) {
                        if (!mediator->hasComponent<Components::PlayerInfo>(entity))
                            continue;
                        const auto &player_info = mediator->getComponent<Components::PlayerInfo>(entity);
                        if (player_info.player_id == current_player_id) {
                            return (player_info.health);
                        }
                    }
                    return (-1);
                }

                int getLocalPlayerHealth(std::shared_ptr<NetworkManager> networkManager) const {
                    std::shared_ptr<Engine::Mediator> mediator = networkManager->mediator;
                    uint32_t localPlayerId = networkManager->player_id;
                    for (const auto &entity : entities) {
                        if (!mediator->hasComponent<Components::PlayerInfo>(entity))
                            continue;
                        const auto &player_info = mediator->getComponent<Components::PlayerInfo>(entity);
                        if (player_info.player_id == localPlayerId) {
                            return (player_info.health);
                        }
                    }
                    if (game_over_state) {
                        return (0);
                    }
                    return (-1);
                }

                int getLocalPlayerScore(std::shared_ptr<NetworkManager> networkManager) const {
                    std::shared_ptr<Engine::Mediator> mediator = networkManager->mediator;
                    uint32_t localPlayerId = networkManager->player_id;
                    for (const auto &entity : entities) {
                        if (!mediator->hasComponent<Components::PlayerInfo>(entity))
                            continue;
                        const auto &player_info = mediator->getComponent<Components::PlayerInfo>(entity);
                        if (player_info.player_id == localPlayerId) {
                            return (player_info.score);
                        }
                    }
                    if (game_over_state) {
                        return last_known_score;
                    }
                    return (0);
                }

            private:
                // std::bitset<5> buttons {};
                std::function<void(float, float)> playerProjectileCreator;
                std::unordered_map<uint32_t, std::bitset<5>> player_id_to_buttons;

                bool game_over_state = false;
                float game_over_time = 0.0f;
                uint32_t current_player_id = 0;
                bool local_player_has_ever_existed = false;
                mutable int last_known_score = 0;
        };
    };
};

#endif /* !PLAYERCONTROL_HPP_ */
