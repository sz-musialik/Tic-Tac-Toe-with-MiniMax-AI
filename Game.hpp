#pragma once
#include <SFML/Graphics.hpp>
#include <chrono>
#include <fstream>
#include <vector>

enum class GameState { MENU, TURN_SELECT, PLAYING, GAME_OVER };

enum class Player { NONE = 0, HUMAN = 1, AI = 2 };

class Game {
private:
  sf::RenderWindow window;

  GameState currentState;

  std::vector<std::vector<Player>> board;

  int boardSize;

  sf::Font font;
  sf::Text menuText;
  sf::Text gameOverText;
  sf::Text turn_text;
  sf::Text turnSelectText;
  sf::Text inputText;
  sf::RectangleShape cellShape;

  Player currentPlayer;
  Player winner;

  bool gameEnded;
  bool humanFirst;

  sf::Vector2i lastMousePos;

  std::chrono::steady_clock::time_point aiStartTime;

  std::string inputString;

  bool inputActive;
  bool mousePressed;

  bool aiThinking;
  double aiThinkTime;

  sf::RectangleShape inputBox;
  sf::RectangleShape submitButton;
  sf::Text submitButtonText;

  sf::RectangleShape firstButton;
  sf::RectangleShape secondButton;
  sf::Text firstButtonText;
  sf::Text secondButtonText;

  sf::Color green1;     // #606c38
  sf::Color green2;     // #283618
  sf::Color background; // #fefae0
  sf::Color red1;       // #780000
  sf::Color red2;       // #c1121f

  float windowWidth;
  float windowHeight;

  std::ofstream resultsFile;

public:
  Game();
  ~Game();

  void run();

private:
  bool initialize();
  void setupMenu();
  void setupTurnSelection();
  void setupGame();
  void initColors();

  void handleEvents();
  void update();
  void render();

  void handleMenuEvents(const sf::Event &event);
  void handleTurnSelectEvents(const sf::Event &event);
  void handleGameEvents(const sf::Event &event);
  void handleTextInput(const sf::Event &event);

  void resetBoard();
  bool isValidMove(int row, int col);
  void makeMove(int row, int col, Player player);
  Player checkWinner();
  bool isBoardFull();

  sf::Vector2i getBoardPosition(sf::Vector2i mousePos);

  int countPossibleMoves();
  void printAITime(int possibleMoves, double time);

  void makeAIMove();

  int minimax(std::vector<std::vector<Player>> &board, int depth,
              bool isMaximizing, int alpha, int beta, int maxDepth);

  int evaluateBoard(const std::vector<std::vector<Player>> &board);

  std::vector<std::pair<int, int>>
  getAvailableMoves(const std::vector<std::vector<Player>> &board);

  void renderMenu();
  void renderTurnSelection();
  void renderGame();
  void renderGameOver();
  void drawBoard();
  void drawSymbol(int row, int col, Player player);

  float getCellSize();
  sf::Vector2f getBoardStartPosition();
};
