#include <cmath>
#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

int mouseToBoard(pair<int, int> &move, Player player, int mouseX, int mouseY) {
    int boardX = floor(mouseX / 40);
    int boardY = floor(mouseY / 40);
    // just in case
    if (boardX == 16) boardX --;
    if (boardY == 16) boardY --;

    move.first = boardX;
    move.second = boardY;
    // cout << "Made a move at: (" << boardX << ", " << boardY << ")" << endl;

    if (!player.makeMove(make_pair(boardX, boardY)))
        return 0;
    else
        return 1;
}

int main() {
    // Gomoku Logic
    Gomoku game(1);
    Player p1(&game, 1);
    GomokuAI ai(&game, 1);  // Use strategy 1 for best performance.

    // Board information
    int boardStart = 20, boardEnd = 580;
    int pieceWidth = 40;
    string message;
    string fontPath = "../asset/Arial.ttf";
    pair<int, int> move = make_pair(-1, -1);

    // GUI starts
    sf::RenderWindow window(sf::VideoMode(600, 600), "Gomoku", sf::Style::Titlebar | sf::Style::Close);

    // Background Texture
    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("../asset/background.png")) {
        fprintf(stderr, "Failed to load board texture.\n");
        return -1;
    }

    // Stones
    sf::CircleShape black(20, 100);
    sf::CircleShape white(20, 100);
    black.setFillColor(sf::Color::Black);
    white.setFillColor(sf::Color::White);
    sf::Font font;
    if (!font.loadFromFile(fontPath)) {
        cout << "Unable to load font file." << endl;
        return -1;
    }

    // Game background Sprite
    sf::Sprite bgSprite;
    bgSprite.setTexture(bgTexture);

    // Draw the board. This includes the lines and the circles.
    // This (line) can be initialized to a random value since it will be reset.
    sf::RectangleShape line(sf::Vector2f(1, 1)); 
    line.setFillColor(sf::Color::Black);

    sf::CircleShape tengen(4, 30);
    tengen.setFillColor(sf::Color::Black);

    // Draw the board
    window.clear();
    window.draw(bgSprite);
    for (int i = 0; i <= game.board_size-1; ++i) {
        // Horizontal
        line.setSize(sf::Vector2f(boardEnd-boardStart, 1)); // Reset to horizontal line
        line.setPosition(boardStart, boardStart + i * pieceWidth);
        window.draw(line);

        // Vertical
        line.setSize(sf::Vector2f(1, boardEnd-boardStart)); // Reset to vertical line
        line.setPosition(boardStart + i * pieceWidth, boardStart);
        window.draw(line);
    }

    // Draw Tangens ((4, 4), (12, 4), (8, 8), (4, 12), (12, 12))
    vector<pair<int, int>> tangens = {{4, 4}, {12, 4}, {8, 8}, {4, 12}, {12, 12}};
    for (auto t: tangens) {
        tengen.setPosition(boardStart + (t.first-1) * pieceWidth - 3, boardStart + (t.second-1) * pieceWidth - 3);
        window.draw(tengen);
    }

    while (window.isOpen() && !game.state) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int mouseX = event.mouseButton.x;
                    int mouseY = event.mouseButton.y;
                    //cout << "Mouse click at: (" << mouseX << ", " << mouseY << ")" << endl;
                    if (!mouseToBoard(move, p1, mouseX, mouseY)) {
                        // draw the stone of player's color here.
                        black.setPosition(move.first * pieceWidth, move.second * pieceWidth);
                        window.draw(black);
                        game.record.push_back(move);

                        if (game.state == 1) {
                            message = "Lucked out, didn't you?";
                            sf::Text gameOverText(message, font, 30);
                            gameOverText.setFillColor(sf::Color::Red);
                            gameOverText.setPosition(200, 50);

                            window.draw(gameOverText);
                            window.display();

                            sf::sleep(sf::seconds(2));
                            
                            window.close();
                            break;
                        }
                    }
                    
                    // AI makes a move.
                    pair<int, int> aiMove= ai.findBestMove();
                    ai.makeMove(aiMove);
                    game.record.push_back(aiMove);
                    //cout << "ai made a move at " << aiMove.first << " , " << aiMove.second << endl;
                    white.setPosition(aiMove.first * pieceWidth, aiMove.second * pieceWidth);
                    window.draw(white);

                    if (game.state == 1) {
                        message = "Suckkkkkker!!!!";
                        sf::Text gameOverText(message, font, 30);
                        gameOverText.setFillColor(sf::Color::Red);
                        gameOverText.setPosition(200, 50);

                        window.draw(gameOverText);
                        window.display();
                        sf::sleep(sf::seconds(2));
                        
                        window.close();
                        break;
                    }
                }
            }
        }
        window.display();
    }

    return 0;
}
