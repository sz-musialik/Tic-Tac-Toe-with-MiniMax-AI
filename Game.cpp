#include "Game.hpp"
#include "SFML/System/Err.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

Game::Game()
    : window(sf::VideoMode::getFullscreenModes()[0], "Tic-Tac-Toe",
             sf::Style::Default, sf::State::Fullscreen),
      currentState(GameState::MENU), boardSize(3), menuText(font),
      gameOverText(font), turn_text(font), turnSelectText(font),
      inputText(font), currentPlayer(Player::HUMAN), winner(Player::NONE),
      gameEnded(false), humanFirst(true), inputString(""), inputActive(false),
      mousePressed(false), aiThinking(false), aiThinkTime(0.0),
      submitButtonText(font), firstButtonText(font), secondButtonText(font) {

  windowWidth = window.getSize().x;
  windowHeight = window.getSize().y;

  green1 = sf::Color(0x60, 0x6c, 0x38);     // #606c38
  green2 = sf::Color(0x28, 0x36, 0x18);     // #283618
  background = sf::Color(0xfe, 0xfa, 0xe0); // #fefae0
  red1 = sf::Color(0xc1, 0x12, 0x1f);       // #c1121f
  red2 = sf::Color(0x78, 0x00, 0x00);       // #780000

  resultsFile.open("results.txt", std::ios::app);
}

Game::~Game() {
  if (resultsFile.is_open()) {
    resultsFile.close();
  }
}

bool Game::initialize() {
  if (!font.openFromFile("arial.ttf")) {
    std::cerr << "Blad wczytywania czcionki!" << std::endl;
    return false;
  }

  setupMenu();

  return true;
}

void Game::setupMenu() {
  menuText.setString("Podaj rozmiar planszy:");
  menuText.setCharacterSize(48);
  menuText.setFillColor(green2);

  sf::FloatRect textBounds = menuText.getLocalBounds();
  menuText.setPosition(
      {(windowWidth - textBounds.size.x) / 2, windowHeight * 0.2f});

  // Input box
  inputBox.setSize(sf::Vector2f(200, 60));
  inputBox.setPosition({(windowWidth - 200) / 2, windowHeight * 0.4f});
  inputBox.setFillColor(sf::Color::White);
  inputBox.setOutlineThickness(3);
  inputBox.setOutlineColor(green1);

  // Input text
  inputText.setCharacterSize(36);
  inputText.setFillColor(green2);

  // Submit button
  submitButton.setSize(sf::Vector2f(150, 60));
  submitButton.setPosition({(windowWidth - 150) / 2, windowHeight * 0.55f});
  submitButton.setFillColor(green1);
  submitButton.setOutlineThickness(2);
  submitButton.setOutlineColor(green2);

  submitButtonText.setString("Start");
  submitButtonText.setCharacterSize(32);
  submitButtonText.setFillColor(background);

  sf::FloatRect buttonTextBounds = submitButtonText.getLocalBounds();

  submitButtonText.setPosition(
      {submitButton.getPosition().x + (150 - buttonTextBounds.size.x) / 2,
       submitButton.getPosition().y + (60 - buttonTextBounds.size.y) / 2 -
           buttonTextBounds.position.y});

  inputActive = true;
}

void Game::setupTurnSelection() {
  turnSelectText.setString("Wybor kolejki");
  turnSelectText.setCharacterSize(48);
  turnSelectText.setFillColor(green2);

  sf::FloatRect textBounds = turnSelectText.getLocalBounds();
  turnSelectText.setPosition(
      {(windowWidth - textBounds.size.x) / 2, windowHeight * 0.3f});

  // Przycisk (Gracz)
  firstButton.setSize(sf::Vector2f(200, 80));
  firstButton.setPosition({windowWidth * 0.3f - 100, windowHeight * 0.5f});
  firstButton.setFillColor(green1);
  firstButton.setOutlineThickness(3);
  firstButton.setOutlineColor(green2);

  firstButtonText.setString("Ja zaczynam");
  firstButtonText.setCharacterSize(32);
  firstButtonText.setFillColor(background);
  sf::FloatRect firstTextBounds = firstButtonText.getLocalBounds();
  firstButtonText.setPosition(
      {firstButton.getPosition().x + (200 - firstTextBounds.size.x) / 2,
       firstButton.getPosition().y + (80 - firstTextBounds.size.y) / 2 -
           firstTextBounds.position.y});

  // Przycisk (AI)
  secondButton.setSize(sf::Vector2f(200, 80));
  secondButton.setPosition({windowWidth * 0.7f - 100, windowHeight * 0.5f});
  secondButton.setFillColor(red1);
  secondButton.setOutlineThickness(3);
  secondButton.setOutlineColor(red2);

  secondButtonText.setString("AI zaczyna");
  secondButtonText.setCharacterSize(32);
  secondButtonText.setFillColor(background);
  sf::FloatRect secondTextBounds = secondButtonText.getLocalBounds();
  secondButtonText.setPosition(
      {secondButton.getPosition().x + (200 - secondTextBounds.size.x) / 2,
       secondButton.getPosition().y + (80 - secondTextBounds.size.y) / 2 -
           secondTextBounds.position.y});
}

void Game::setupGame() {
  resetBoard();

  gameOverText.setCharacterSize(48);

  turn_text.setCharacterSize(48);
  turn_text.setFillColor(green2);
  turn_text.setPosition({20, 20});

  float cellSize = getCellSize();
  cellShape.setSize(sf::Vector2f(cellSize - 4, cellSize - 4));
  cellShape.setFillColor(sf::Color::Transparent);
  cellShape.setOutlineThickness(3);
  cellShape.setOutlineColor(green2);
}

float Game::getCellSize() {
  float availableHeight = windowHeight * 0.8;

  return availableHeight / boardSize;
}

sf::Vector2f Game::getBoardStartPosition() {
  float cellSize = getCellSize();
  float boardWidth = cellSize * boardSize;

  float startX = (windowWidth - boardWidth) / 2;
  float startY = windowHeight * 0.1;

  return sf::Vector2f(startX, startY);
}

void Game::run() {
  if (!initialize()) {
    return;
  }

  while (window.isOpen()) {
    handleEvents();
    update();
    render();
  }
}

void Game::handleEvents() {
  while (const std::optional event = window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      window.close();
    }

    if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
      if (keyPressed->code == sf::Keyboard::Key::Escape) {
        window.close();
      }
    }

    switch (currentState) {
    case GameState::MENU:
      handleMenuEvents(*event);
      break;
    case GameState::TURN_SELECT:
      handleTurnSelectEvents(*event);
      break;
    case GameState::PLAYING:
      handleGameEvents(*event);
      break;
    case GameState::GAME_OVER:
      if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Space) {

          currentState = GameState::MENU;
          setupMenu();
        }
      }
      break;
    }
  }
}

void Game::handleMenuEvents(const sf::Event &event) {
  handleTextInput(event);

  if (const auto *mouseButtonPresed =
          event.getIf<sf::Event::MouseButtonPressed>()) {
    if (mouseButtonPresed->button == sf::Mouse::Button::Left) {
      sf::Vector2i mousePos = sf::Mouse::getPosition(window);

      if (submitButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        try {
          int size = std::stoi(inputString);

          if (size >= 3) {
            boardSize = size;
            currentState = GameState::TURN_SELECT;
            setupTurnSelection();
          }
        } catch (const std::exception &) {
        }
      }
    }
  }
}

void Game::handleTurnSelectEvents(const sf::Event &event) {
  if (const auto *mouseButtonPressed =
          event.getIf<sf::Event::MouseButtonPressed>()) {
    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
      sf::Vector2i mousePos = sf::Mouse::getPosition(window);

      if (firstButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        humanFirst = true;

        currentPlayer = Player::HUMAN;
        currentState = GameState::PLAYING;

        setupGame();
      }

      if (secondButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        humanFirst = false;

        currentPlayer = Player::AI;
        currentState = GameState::PLAYING;

        setupGame();

        aiThinking = true;
        aiStartTime = std::chrono::steady_clock::now();
      }
    }
  }
}

void Game::handleTextInput(const sf::Event &event) {
  if (!inputActive) {
    return;
  }

  if (const auto *textEntered = event.getIf<sf::Event::TextEntered>()) {
    // Backspace
    if (textEntered->unicode == 8) {
      if (!inputString.empty()) {
        inputString.pop_back();
      }
    }

    // Liczby 0 - 9
    if (textEntered->unicode >= 48 && textEntered->unicode <= 57) {
      if (inputString.length() < 2) {
        inputString += static_cast<char>(textEntered->unicode);
      }
    }

    inputText.setString(inputString);

    sf::FloatRect inputTextBounds = inputText.getLocalBounds();

    inputText.setPosition(
        {inputBox.getPosition().x + (200 - inputTextBounds.size.x) / 2,
         inputBox.getPosition().y + (60 - inputTextBounds.size.y) / 2 -
             inputTextBounds.position.y});
  }

  if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
    if (keyPressed->code == sf::Keyboard::Key::Enter) {
      try {
        int size = std::stoi(inputString);

        if (size >= 3) {
          boardSize = size;
          currentState = GameState::TURN_SELECT;
          setupTurnSelection();
        }
      } catch (const std::exception &) {
        // Niepoprawny input
      }
    }
  }
}

void Game::handleGameEvents(const sf::Event &event) {
  if (gameEnded || currentPlayer == Player::AI || aiThinking) {
    return;
  }

  if (const auto *mouseButtonPressed =
          event.getIf<sf::Event::MouseButtonPressed>()) {
    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
      sf::Vector2i mousePos = sf::Mouse::getPosition(window);
      sf::Vector2i boardPos = getBoardPosition(mousePos);

      if (boardPos.x >= 0 && boardPos.x < boardSize && boardPos.y >= 0 &&
          boardPos.y < boardSize) {
        if (isValidMove(boardPos.y, boardPos.x)) {
          makeMove(boardPos.y, boardPos.x, Player::HUMAN);

          winner = checkWinner();

          if (winner != Player::NONE || isBoardFull()) {
            gameEnded = true;
            currentState = GameState::GAME_OVER;
          } else {
            currentPlayer = Player::AI;
            aiThinking = true;

            aiStartTime = std::chrono::steady_clock::now();
          }
        }
      }
    }
  }
}

void Game::update() {
  if (currentState == GameState::PLAYING && currentPlayer == Player::AI &&
      !gameEnded) {
    if (aiThinking) {
      int possibleMoves = countPossibleMoves();

      makeAIMove();

      // Pomiar czasu na ruch AI
      auto endTime = std::chrono::steady_clock::now();

      auto actualDuration =
          std::chrono::duration_cast<std::chrono::milliseconds>(endTime -
                                                                aiStartTime);
      double actualTime = actualDuration.count() / 1000.0;

      printAITime(possibleMoves, actualTime);

      aiThinking = false;

      winner = checkWinner();

      if (winner != Player::NONE || isBoardFull()) {
        gameEnded = true;
        currentState = GameState::GAME_OVER;
      } else {
        currentPlayer = Player::HUMAN;
      }
    }
  }
}

void Game::render() {
  window.clear(background);

  switch (currentState) {
  case GameState::MENU:
    renderMenu();
    break;
  case GameState::TURN_SELECT:
    renderTurnSelection();
    break;
  case GameState::PLAYING:
    drawBoard();
    break;
  case GameState::GAME_OVER:
    renderGameOver();
    break;
  }

  window.display();
}

void Game::renderMenu() {
  window.draw(menuText);
  window.draw(inputBox);
  window.draw(inputText);
  window.draw(submitButton);
  window.draw(submitButtonText);
}

void Game::renderTurnSelection() {
  window.draw(turnSelectText);
  window.draw(firstButton);
  window.draw(firstButtonText);
  window.draw(secondButton);
  window.draw(secondButtonText);
}

void Game::renderGameOver() {
  drawBoard();

  std::string resultText;

  sf::Color textColor;
  sf::Color backgroundColor;
  sf::Color outlineColor;

  if (winner == Player::HUMAN) {
    resultText = "Wygrana!";
    textColor = background;
    backgroundColor = green2;
    outlineColor = green1;
  } else if (winner == Player::AI) {
    resultText = "Przegrana!";
    textColor = background;
    backgroundColor = red2;
    outlineColor = red1;
  } else {
    resultText = "Remis!";
    textColor = sf::Color::Black;
    backgroundColor = background;
    outlineColor = green2;
  }

  resultText += "\nWcisnij spacje zeby powrocic do menu";
  gameOverText.setString(resultText);
  gameOverText.setFillColor(textColor);

  sf::FloatRect textBounds = gameOverText.getLocalBounds();
  float padding = 20;

  sf::RectangleShape backgroundBox;
  backgroundBox.setSize(sf::Vector2f(textBounds.size.x + 2 * padding,
                                     textBounds.size.y + 2 * padding));
  backgroundBox.setFillColor(backgroundColor);
  backgroundBox.setOutlineThickness(4);
  backgroundBox.setOutlineColor(outlineColor);

  backgroundBox.setPosition({(windowWidth - backgroundBox.getSize().x) / 2,
                             (windowHeight - backgroundBox.getSize().y) / 2});

  gameOverText.setPosition(
      {backgroundBox.getPosition().x - textBounds.position.x + padding,
       backgroundBox.getPosition().y - textBounds.position.y + padding});

  window.draw(backgroundBox);
  window.draw(gameOverText);
}

void Game::drawBoard() {
  float cellSize = getCellSize();
  sf::Vector2f startPos = getBoardStartPosition();

  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      cellShape.setPosition(
          {startPos.x + j * cellSize + 2, startPos.y + i * cellSize + 2});
      window.draw(cellShape);

      if (board[i][j] != Player::NONE) {
        drawSymbol(i, j, board[i][j]);
      }
    }
  }
}

void Game::drawSymbol(int row, int col, Player player) {
  float cellSize = getCellSize();

  sf::Vector2f startPos = getBoardStartPosition();

  sf::Text symbol(font);

  symbol.setCharacterSize(cellSize * 0.6f);

  if (player == Player::HUMAN) {
    symbol.setString("X");
    symbol.setFillColor(green1);
  } else {
    symbol.setString("O");
    symbol.setFillColor(red1);
  }

  sf::FloatRect textBounds = symbol.getLocalBounds();

  symbol.setPosition(
      {startPos.x + col * cellSize + (cellSize - textBounds.size.x) / 2,
       startPos.y + row * cellSize + (cellSize - textBounds.size.y) / 2 -
           textBounds.position.y});

  window.draw(symbol);
}

void Game::resetBoard() {
  board.assign(boardSize, std::vector<Player>(boardSize, Player::NONE));

  currentPlayer = humanFirst ? Player::HUMAN : Player::AI;
  winner = Player::NONE;

  gameEnded = false;

  aiThinking = false;
}

bool Game::isValidMove(int row, int col) {
  if (row >= 0 && row < boardSize && col >= 0 && col < boardSize &&
      board[row][col] == Player::NONE) {
    return true;
  }

  return false;
}

void Game::makeMove(int row, int col, Player player) {
  if (isValidMove(row, col)) {
    board[row][col] = player;
  }
}

sf::Vector2i Game::getBoardPosition(sf::Vector2i mousePos) {
  float cellSize = getCellSize();

  sf::Vector2f startPos = getBoardStartPosition();

  if (mousePos.x < startPos.x || mousePos.y < startPos.y) {
    return sf::Vector2i(-1, -1);
  }

  int col = (mousePos.x - startPos.x) / cellSize;
  int row = (mousePos.y - startPos.y) / cellSize;

  return sf::Vector2i(col, row);
}

int Game::countPossibleMoves() {
  int count = 0;

  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      if (board[i][j] == Player::NONE) {
        count++;
      }
    }
  }

  return count;
}

void Game::printAITime(int possibleMoves, double time) {
  if (resultsFile.is_open()) {
    resultsFile << boardSize << ";" << possibleMoves << ";" << time << "\n";

    resultsFile.flush();
  }
}

Player Game::checkWinner() {
  // Check rows
  for (int i = 0; i < boardSize; i++) {
    bool rowWin = true;
    Player first = board[i][0];
    if (first == Player::NONE)
      continue;

    for (int j = 1; j < boardSize; j++) {
      if (board[i][j] != first) {
        rowWin = false;
        break;
      }
    }
    if (rowWin)
      return first;
  }

  // Check columns
  for (int j = 0; j < boardSize; j++) {
    bool colWin = true;
    Player first = board[0][j];
    if (first == Player::NONE)
      continue;

    for (int i = 1; i < boardSize; i++) {
      if (board[i][j] != first) {
        colWin = false;
        break;
      }
    }
    if (colWin)
      return first;
  }

  // Przekatne
  bool diagWin = true;

  Player first = board[0][0];

  if (first != Player::NONE) {
    for (int i = 1; i < boardSize; i++) {
      if (board[i][i] != first) {
        diagWin = false;
        break;
      }
    }

    if (diagWin) {
      return first;
    }
  }

  diagWin = true;

  first = board[0][boardSize - 1];

  if (first != Player::NONE) {
    for (int i = 1; i < boardSize; i++) {
      if (board[i][boardSize - 1 - i] != first) {
        diagWin = false;
        break;
      }
    }

    if (diagWin) {
      return first;
    }
  }

  return Player::NONE;
}

bool Game::isBoardFull() {
  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      if (board[i][j] == Player::NONE) {
        return false;
      }
    }
  }

  return true;
}

void Game::makeAIMove() {
  std::vector<std::pair<int, int>> availableMoves = getAvailableMoves(board);

  if (availableMoves.empty()) {
    return;
  }

  int maxDepth;
  int emptyCells = availableMoves.size();

  if (emptyCells < 10) {
    maxDepth = emptyCells;
  } else if (emptyCells < 15) {
    maxDepth = 8;
  } else if (emptyCells < 20) {
    maxDepth = 6;
  } else if (emptyCells < 25) {
    maxDepth = 4;
  } else {
    maxDepth = 2;
  }

  int bestScore = std::numeric_limits<int>::min();
  std::pair<int, int> bestMove = availableMoves[0];

  std::sort(availableMoves.begin(), availableMoves.end(),
            [this](const std::pair<int, int> &a, const std::pair<int, int> &b) {
              int centerRow = boardSize / 2;
              int centerCol = boardSize / 2;

              int distA = abs(a.first - centerRow) + abs(a.second - centerCol);
              int distB = abs(b.first - centerRow) + abs(b.second - centerCol);

              return distA < distB;
            });

  for (const auto &move : availableMoves) {
    std::vector<std::vector<Player>> tempBoard = board;

    tempBoard[move.first][move.second] = Player::AI;

    int score = minimax(tempBoard, 0, false, std::numeric_limits<int>::min(),
                        std::numeric_limits<int>::max(), maxDepth);

    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
    }
  }

  makeMove(bestMove.first, bestMove.second, Player::AI);
}

int Game::minimax(std::vector<std::vector<Player>> &board, int depth,
                  bool isMaximizing, int alpha, int beta, int maxDepth) {
  int score = evaluateBoard(board);

  if (score != 0 || depth >= maxDepth) {
    if (score > 0)
      return score - depth;

    if (score < 0)
      return score + depth;

    return 0;
  }

  std::vector<std::pair<int, int>> moves = getAvailableMoves(board);

  if (moves.empty()) {
    return 0;
  }

  if (isMaximizing) {
    int maxScore = std::numeric_limits<int>::min();

    for (const auto &move : moves) {
      board[move.first][move.second] = Player::AI;

      int currentScore =
          minimax(board, depth + 1, false, alpha, beta, maxDepth);

      board[move.first][move.second] = Player::NONE;

      maxScore = std::max(maxScore, currentScore);
      alpha = std::max(alpha, currentScore);

      if (beta <= alpha) {
        break;
      }
    }

    return maxScore;
  } else {
    int minScore = std::numeric_limits<int>::max();

    for (const auto &move : moves) {
      board[move.first][move.second] = Player::HUMAN;

      int currentScore = minimax(board, depth + 1, true, alpha, beta, maxDepth);

      board[move.first][move.second] = Player::NONE;

      minScore = std::min(minScore, currentScore);
      beta = std::min(beta, currentScore);

      if (beta <= alpha) {
        break;
      }
    }

    return minScore;
  }
}

int Game::evaluateBoard(const std::vector<std::vector<Player>> &evalBoard) {
  Player winner = Player::NONE;

  // Wiersze
  for (int i = 0; i < boardSize; i++) {
    bool rowWin = true;

    Player first = evalBoard[i][0];

    if (first == Player::NONE)
      continue;

    for (int j = 1; j < boardSize; j++) {
      if (evalBoard[i][j] != first) {
        rowWin = false;
        break;
      }
    }

    if (rowWin) {
      winner = first;
      break;
    }
  }

  // Kolumny
  if (winner == Player::NONE) {
    for (int j = 0; j < boardSize; j++) {
      bool colWin = true;

      Player first = evalBoard[0][j];

      if (first == Player::NONE)
        continue;

      for (int i = 1; i < boardSize; i++) {
        if (evalBoard[i][j] != first) {
          colWin = false;
          break;
        }
      }

      if (colWin) {
        winner = first;
        break;
      }
    }
  }

  // Przekatne
  if (winner == Player::NONE) {
    bool diagWin = true;

    Player first = evalBoard[0][0];

    if (first != Player::NONE) {
      for (int i = 1; i < boardSize; i++) {
        if (evalBoard[i][i] != first) {
          diagWin = false;
          break;
        }
      }

      if (diagWin) {
        winner = first;
      }
    }

    if (winner == Player::NONE) {
      diagWin = true;

      first = evalBoard[0][boardSize - 1];

      if (first != Player::NONE) {
        for (int i = 1; i < boardSize; i++) {
          if (evalBoard[i][boardSize - 1 - i] != first) {
            diagWin = false;
            break;
          }
        }

        if (diagWin) {
          winner = first;
        }
      }
    }
  }

  if (winner == Player::AI) {
    return 10;
  }

  if (winner == Player::HUMAN) {
    return -10;
  }

  return 0;
}

std::vector<std::pair<int, int>>
Game::getAvailableMoves(const std::vector<std::vector<Player>> &evalBoard) {
  std::vector<std::pair<int, int>> moves;

  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      if (evalBoard[i][j] == Player::NONE) {
        moves.push_back({i, j});
      }
    }
  }

  return moves;
}
