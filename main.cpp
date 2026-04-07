#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "card.h"



//Loads card textures from assets folder
void loadCardTextures(sf::Texture &back, std::vector<sf::Texture> &faces){
    const std::string backPaths[] = {"assets/cards/back.png", "../assets/cards/back.png"};
    bool backLoaded = false;
    for (const std::string &path : backPaths) {
        if (back.loadFromFile(path)) {
            backLoaded = true;
            break;
        }
    }
    if (!backLoaded) {
        std::cerr << "Error loading back texture" << std::endl;
    }

    for (int i = 0; i < static_cast<int>(faces.size()); ++i) {
        const std::string facePaths[] = {
            "assets/cards/" + std::to_string(i + 2) + "_of_clubs.png",
            "../assets/cards/" + std::to_string(i + 2) + "_of_clubs.png"
        };
        bool faceLoaded = false;
        for (const std::string &path : facePaths) {
            if (faces[i].loadFromFile(path)) {
                faceLoaded = true;
                break;
            }
        }
        if (!faceLoaded) {
            std::cerr << "Error loading face texture " << i << std::endl;
        }
    }
}

//Shuffles creates deck of cards from textures from faces and back and then shuffles them
void buildDeck(sf::Texture &back, std::vector<sf::Texture> &faces, std::vector<Card> &cards, int size) {
    cards.clear();
    cards.reserve(size * 2);

    for (int i = 0; i < size; ++i) {
        Card card(i, &faces[i], &back);
        cards.push_back(card);
        cards.push_back(card);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(cards.begin(), cards.end(), gen);
}

//Sets the layout and spacing for memory game grid
void setLayout(sf::Texture &back, std::vector<Card> &cards, unsigned int size) {
    const float cardWidth = static_cast<float>(back.getSize().x);
    const float cardHeight = static_cast<float>(back.getSize().y);
    const float startX = (800.f - static_cast<float>(size) * cardWidth - static_cast<float>(size - 1) * 10.f) / 2.f;
    const float startY = 80.f;

    for (unsigned int i = 0; i < cards.size(); ++i) {
        cards.at(i).setPosition(startX + (i % size) * (cardWidth + 10.f), startY + (i / size) * (cardHeight + 10.f));
    }
}
int main() {
    //Create game window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Memory Match");

    // Load textures
    sf::Texture back;
    std::vector<sf::Texture> faces(8);
    loadCardTextures(back, faces);

    // Build deck (16 cards: 8 pairs)
    std::vector<Card> cards;
    buildDeck(back, faces, cards, 8);

    // Layout (4x4 grid)
    setLayout(back, cards, 4);

    //Initialize game logic
    std::vector<int> flipped;
    sf::Clock mismatchClock;
    sf::Clock elapsedClock;
    bool twoCardsFlipped = false;
    int strikes = 0;
    int matchedCards = 0;

    sf::Font font;
    const std::string fontPaths[] = {"assets/arial.ttf", "../assets/arial.ttf"};
    bool fontLoaded = false;
    for (const std::string &path : fontPaths) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }
    if (!fontLoaded) {
        std::cerr << "Error loading font" << std::endl;
    }

    sf::Text hud(font);
    hud.setCharacterSize(20);
    hud.setFillColor(sf::Color::White);
    hud.setPosition(sf::Vector2f(20.f, 20.f));

    while (window.isOpen()) {
        while (std::optional<sf::Event> event = window.pollEvent()) {
            // Close
            if (event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }


            // Left-click to flip
            if (!twoCardsFlipped) {
                if (const auto *mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left) {
                        //If mouse button clicked, game logic begins
                        for (int i = 0; i < 16; ++i) {
                            if (!cards[i].isUp() && !cards[i].isMatched() && cards[i].contains(sf::Vector2f(mouse->position))) {
                                cards[i].reveal();
                                flipped.push_back(i);

                                if (flipped.size() == 2) {
                                    twoCardsFlipped = true;
                                    int firstCardIndex = flipped[0];
                                    int secondCardIndex = flipped[1];
                                    if (cards[firstCardIndex].getValue() == cards[secondCardIndex].getValue()) {
                                        cards[firstCardIndex].match();
                                        cards[secondCardIndex].match();
                                        matchedCards += 2;
                                        flipped.clear();
                                        twoCardsFlipped = false;

                                    } else {
                                        ++strikes;
                                        mismatchClock.restart(); // start mismatch delay
                                    }
                                }
                                break; // only one card per click
                            }
                        }
                    }
                }
            }
        }

        if (twoCardsFlipped && mismatchClock.getElapsedTime().asMilliseconds() > 700) {
            cards[flipped[0]].hide();
            cards[flipped[1]].hide();
            flipped.clear();
            twoCardsFlipped = false;
        }

        std::ostringstream hudStream;
        hudStream << std::fixed << std::setprecision(1)
                  << "Time: " << elapsedClock.getElapsedTime().asSeconds() << "s"
                  << "    Strikes: " << strikes
                  << "    Completion: " << (matchedCards * 100 / 16) << "%";
        hud.setString(hudStream.str());

        //draw background and cards
        window.clear(sf::Color::Black);
        window.draw(hud);
        for (auto &c: cards) c.draw(window);

        window.display();
    }
}




