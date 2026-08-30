#include <iostream>
#include <random>
#include <thread>
#include <array>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>

class TicTacToe {
  private:
  std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
  char current_player; // Jogador atual ('X' ou 'O')
  bool game_over; // Estado do jogo
  char winner; // Vencedor do jogo

  std::mutex mtx;
  std::condition_variable cv;
  
  public:
  TicTacToe() {
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        board[i][j] = ' ';
      }
    }

    winner = '-';
    game_over = false;

    // Sorteia jogador inicial entre 'X' e 'O'
    static std::mt19937 sorteiaJogador(
      static_cast<unsigned int>(time(0))
    );

    static std::uniform_int_distribution<int> distr(0, 1);

    current_player =
      distr(sorteiaJogador) == 0 ? 'X' : 'O';
  }
  
  void display_board() {
    // Exibir o tabuleiro no console
    std::lock_guard<std::mutex> lock(mtx);

    std::system("clear");

    for(int i = 0; i < 3; i++){
      std::cout << board[i][0]
                << "|"
                << board[i][1]
                << "|"
                << board[i][2]
                << std::endl;

      if(i != 2){
        std::cout << "-----" << std::endl;
      }
    }
  }
  
  bool make_move(char player, int row, int col) {
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock, [this, player]() {
      return current_player == player || game_over;
    });

    if(game_over){
      return false;
    }

    if(board[row][col] != 'X' &&
       board[row][col] != 'O'){

        board[row][col] = player;

      std::cout << "Jogador "
                << player
                << " jogou em ("
                << row
                << ", "
                << col
                << ")"
                << std::endl;

      if(check_win(player)){
        game_over = true;
        winner = player;

        lock.unlock();
        cv.notify_all();

        display_board();
        return true;
      }

      if(check_draw()){
        game_over = true;
        winner = 'D';

        lock.unlock();
        cv.notify_all();

        display_board();

        return true;
      }

      if(player == 'O'){
        current_player = 'X';
      }else{
        current_player = 'O';
      }

      lock.unlock();
      cv.notify_all();

      display_board();

      return true;
    }
    return false;
  }
  
  bool check_win(char player) {
    for(int i = 0; i < 3; i++){
      if(player == board[i][0] &&
         player == board[i][1] &&
         player == board[i][2]){

        return true;
      }
    }

    for(int i = 0; i < 3; i++){
      if(player == board[0][i] &&
         player == board[1][i] &&
         player == board[2][i]){

        return true;
      }
    }

    if(player == board[0][0] &&
       player == board[1][1] &&
       player == board[2][2]){

      return true;
    }

    if(player == board[0][2] &&
       player == board[1][1] &&
       player == board[2][0]){

      return true;
    }

    return false;
  }
  
  bool check_draw() {
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){

        if(board[i][j] == ' '){
          return false;
        }
      }
    }

    return true;
  }
  
  bool is_game_over() {
    std::lock_guard<std::mutex> lock(mtx);
    return game_over;
  }
  
  char get_winner() {
    std::lock_guard<std::mutex> lock(mtx);
    return winner;
  }
};

class Player {
  private:
  TicTacToe& game; // Referência para a instância do jogo
  char symbol; // Símbolo do jogador ('X' ou 'O')
  std::string strategy; // Estratégia do jogador
  
  public:
  Player(TicTacToe& g, char s, std::string strat)
  : game(g), symbol(s), strategy(strat) {}
  
  void play() {
    while(!game.is_game_over()){

      std::this_thread::sleep_for(
        std::chrono::milliseconds(200)
      );

      if(strategy == "sequential"){
        play_sequential();
      }else{
        play_random();
      }
    }
  }
  
  private:

  void play_sequential() {
    // Estratégia sequencial
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(game.make_move(symbol, i, j)){
          return;
        }
        if(game.is_game_over()){
          return;
        }
      }
    }
  }
  
  void play_random() {
    static thread_local std::mt19937 gen(
      std::random_device{}()
    );

    static thread_local std::uniform_int_distribution<> distr(
      0, 2
    );

    while(!game.is_game_over()){

      int l = distr(gen);
      int c = distr(gen);

      // Se a posição estiver ocupada, make_move
      // retorna false e tentamos outra posição.
      if(game.make_move(symbol, l, c)){
        return;
      }
    }
  }
};


// Função principal
int main() {

  // Inicializar o jogo
  TicTacToe tabuleiro;

  tabuleiro.display_board();

  // Criar os jogadores
  Player X(tabuleiro, 'X', "sequential");
  Player O(tabuleiro, 'O', "random");
  
  // Criar as threads
  std::thread Jogador1(&Player::play, &X);
  std::thread Jogador2(&Player::play, &O);
  
  // Aguardar o término das threads
  Jogador1.join();
  Jogador2.join();
  
  // Exibir resultado final
  char vencedor = tabuleiro.get_winner();

  if(vencedor == 'D'){
    std::cout << "Empate!\n";
  }else{
    std::cout << "Vencedor: "
              << vencedor
              << "\n";
  }
  
  return 0;
}