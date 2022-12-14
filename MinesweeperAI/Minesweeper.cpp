#include <iostream>
#include "TextureManager.h"
#include <fstream>
#include <string>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include "Random.h"
using namespace sf;
using namespace std;

//function to take in score and check if positive or negative
bool checkScore(int score) {
    if (score >= 0) {
        return false;
    }
    else {
        return true;
    }

}
//function to take in score, display vector and resource vector to set display vector to curr score
void SetDigitMap(int score, vector<Sprite>& digitDisplay, vector<Sprite>& digitOptions) {
    digitDisplay.clear();
    int digs[3];
    bool sign = checkScore(score);
    int shift = 0;
    if (!sign) {
        digs[0] = score / 100;
        digs[1] = score / 10;
        if (digs[1] > 9) {
            digs[1] = digs[1] % 10;
        }
        digs[2] = score % 10;
    }
    else {
        shift = 1;
        digs[0] = score * -1 / 100;
        digs[1] = score * -1 / 10;
        if (digs[1] > 9) {
            digs[1] = digs[1] % 10;
        }
        digs[2] = score * -1 % 10;
        digitDisplay.push_back(digitOptions.at(10));
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            if (digs[i] == j) {
                digitDisplay.push_back(digitOptions[digs[i]]);
            }
        }
    }
}
//point class to hold a location of a tile in cartesian coordinates (x,y)
class Point {
public:
    int x;
    int y;
};
//tile class to hold tile status and functions to change the tiles
class Tile {
public:
    //constructor to initialize tile from coordinates
    Tile(int x, int y) {
        coordinates.x = x;
        coordinates.y = y;
        Reset();
        bg_sprite.setPosition((x) * 32, (y) * 32);
        fg_sprite.setPosition((x) * 32, (y) * 32);
        bounds = bg_sprite.getGlobalBounds();
    }
    //reset to return all class variables to default
    void Reset() {
        number = 0;
        is_flagged = false;
        is_revealed = false;
        is_mine = false;
        adj_mine = false;
        bg_sprite.setTexture(TextureManager::GetTexture("tile_hidden"));
        fg_sprite.setTexture(TextureManager::GetTexture("tile_hidden"));
    }
    //setters
    void SetAdjacent(Tile* neighbor) {
        adjacents.push_back(neighbor);
        if (neighbor->is_mine) {
            number++;
             adj_mine = true;
        }
    }
    void RevealFG() {
        if (number != 0 && !IsAMine()) {
            fg_sprite.setTexture(TextureManager::GetTexture("number_" + to_string(number)));
        }
        else if (IsAMine()) {
            fg_sprite.setTexture(TextureManager::GetTexture("mine"));
        }
        else {
            fg_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
        }
    }
    void RevealBG() {
        bg_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
    }
    void SetMine() {
        is_mine = true;
    }
    //actions
    void RightClick(int& score) {
        if (!is_revealed && !is_flagged) {
            is_flagged = true;
            fg_sprite.setTexture(TextureManager::GetTexture("flag"));
            score--;
        }
        else if(!is_revealed) {
            is_flagged = false;
            fg_sprite.setTexture(TextureManager::GetTexture("tile_hidden"));
            score++;
        }
    }
    void RevealTile(Tile& tile, int& curr_revealed, bool& mine) {
        if (!tile.is_revealed && !tile.is_mine && !tile.is_flagged) {
            tile.is_revealed = true;
            tile.RevealFG();
            tile.RevealBG();
            curr_revealed++;
            for (int i = 0; i < tile.adjacents.size(); i++) {
                if (!tile.adj_mine && !tile.adjacents[i]->is_revealed) {
                RevealTile(*tile.adjacents[i], curr_revealed, mine);
            }
        }
    }
        else if (!tile.is_revealed && !tile.is_flagged) {
            tile.is_revealed = true;
            tile.RevealFG();
            tile.fg_sprite.setTexture(TextureManager::GetTexture("mine"));
            tile.RevealBG();
            mine = true;
        }
        else {
        }
    }
    bool checkBounds(sf::Vector2i mousePos) {
        if (bounds.contains(mousePos.x, mousePos.y)) {
            return true;
        }
        else {
            return false;
        }
    }
    //getters
    bool IsARevealed() {
        return is_revealed;
    }
    bool IsAFlagged() {
        return is_flagged;
    }
    bool IsAMine() {
        return is_mine;
    }
    Point GetLocation() {
        return coordinates;
    }
    Sprite GetBGSprite() {
        return bg_sprite;
    }
    Sprite GetFGSprite() {
        return fg_sprite;
    }
private:
    int number;
    vector<Tile*> adjacents;
    Point coordinates;
    bool is_flagged;
    bool is_mine;
    bool is_revealed;
    bool adj_mine;
    Sprite fg_sprite;
    Sprite bg_sprite;
    FloatRect bounds;
};
//board class to hold tile vector and other properties of a board: dimensions, mines
class Board {
public:
    //board ctor reads infile, uses to set dimensions, creates a tile vector and reset class vars
    Board() {
        inFile.open("boards/config.cfg");
        ReadFile();
        inFile.close();
        CreateTiles();
        Reset();
    }
    //getters
    int Get_Cols() {
        return num_cols;
    }
    int Get_Rows() {
        return num_rows;
    }
    int Get_Mines() {
        return num_mines;
    }
    Tile Get_Tile(int x, int y) {
        for (int i = 0; i < board.size(); i++) {
            if (board[i].GetLocation().x == x && board[i].GetLocation().y == y) {
                return board[i];
            }
        }
    }
    vector<Tile>& Get_Board() { return board; }
    //reset board to plain state
    void Reset() {
        curr_mines = 0;
        num_revealed = 0;
        num_hidden = num_cols * num_rows - num_mines;
        for (int i = 0; i < board.size(); i++) {
            board.at(i).Reset();
        }
        while (curr_mines != num_mines) {
            int working = Random::Number(0, board.size() - 1);
            if (!board.at(working).IsAMine()) {
                board.at(working).SetMine();
                curr_mines++;
            }
        } 
        SetAdjacents();
    }
    //push back tiles with given locations within rows and cols
    void CreateTiles() {
        for (int i = 0; i < num_rows; i++) {
            for (int j = 0; j < num_cols; j++) {
                board.push_back(Tile(j, i)); // will have number 0, all specials false, hidden texture
            }
        }
    }
    //setter
    void SetAdjacents() {
        for (int i = 0; i < board.size(); i++) {
            //cout << "Tile at: (" << board[i].GetLocation().x << ", " << board[i].GetLocation().y << ") has adjacents: " << endl;
            for (int j = (i - num_cols - 1); j <= (i - num_cols + 1); j++) {
                if (j >= 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(j).GetLocation().y + 1)) {
                    //cout << "(" << board[j].GetLocation().x << ", " << board[j].GetLocation().y << ")" << endl;
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
            for (int j = (i - 1); j <= (i + 1); j += 2) {
                if (j >= 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(j).GetLocation().y)) {
                    //cout << "(" << board[j].GetLocation().x << ", " << board[j].GetLocation().y << ")" << endl;
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
            for (int j = (i + num_cols - 1); j <= (i + num_cols + 1); j++) {
                if (j >= 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(j).GetLocation().y - 1)) {
                    //cout << "(" << board[j].GetLocation().x << ", " << board[j].GetLocation().y << ")" << endl;
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
        }
    }
    //function to update tile at an index
    void UpdateTile(int index, bool& leftClicked, bool& mine, bool& reveal , int& score) {
        if (leftClicked) {
            board[index].RevealTile(board[index], num_revealed, mine); // mine becomes 1 when a mine is revealed
        }
        else {
            board[index].RightClick(score);
        }
        if ((num_rows * num_cols) - num_revealed == num_mines) {
             reveal = true;
        }
    }
    //troubleshooting function
    void print_mines() {
        for (int i = 0; i < board.size(); i++) {
            if (board[i].IsAMine()) {
                board[i].RevealFG();
                board[i].RevealBG();
            }
        }
    }
private:
    int curr_mines;
    int num_rows;
    int num_cols;
    int num_mines;
    int num_revealed;
    int num_hidden;
    ifstream inFile;
    vector<Tile> board;
    void ReadFile() {
        string s1, s2, s3;
        getline(inFile, s1);
        getline(inFile, s2);
        getline(inFile, s3);
        num_cols = stoi(s1);
        num_rows = stoi(s2);
        num_mines = stoi(s3);
    }
};
//minesweeper class is the game object, callable by a main process (for later use in an AI implementation)
class Minesweeper {
public:
    //ctor capable of UI handling and variable resets
    Minesweeper() {
        game_board = Board();
        width = game_board.Get_Cols() * 32;
        height = (game_board.Get_Rows() * 32) + 88;
        window.create(VideoMode(width, height), "Minesweeper");
        LoadAll();
        DrawMenu();
        faceBounds = face.getGlobalBounds();
        Reset();
    }
    //game loop of reloading the window and accepting input
    void GameLoop() {
        //until game is closed
        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                //check if game is closed
                if (event.type == Event::Closed) {
                    window.close();
                }
                bool leftClick;
                //check if user input
                if (event.type == sf::Event::MouseButtonPressed) {
                    //left click: either menu interaction or tile interaction
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        leftClick = true;
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        if (faceBounds.contains(mousePos.x, mousePos.y)) {
                            Reset();
                        }
                    }
                    // only update board if game state == 0: not win or loss
                    if (game_state == 0) {
                        //right click: update flag status of a tile
                        if (event.mouseButton.button == sf::Mouse::Right) {
                            leftClick = false;
                        }
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        // for each tile
                        for (int i = 0; i < game_board.Get_Board().size(); i++) {
                            //check if tile clicked
                            if (game_board.Get_Board()[i].checkBounds(mousePos)) {
                                //update clicked tile and works depending on right/left click
                                game_board.UpdateTile(i, leftClick, mine_pressed, all_revealed, score);
                                if (all_revealed) {
                                    game_state = 1;
                                }
                                if (mine_pressed == true) {
                                    game_state = -1;
                                }
                            }
                        }
                    }
                }
            }
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                window.clear(Color::White);
                //draw every tile
                for (int i = 0; i < game_board.Get_Rows(); i++) {
                    for (int j = 0; j < game_board.Get_Cols(); j++) {
                        window.draw(game_board.Get_Tile(j, i).GetBGSprite());
                        window.draw(game_board.Get_Tile(j, i).GetFGSprite());
                    }
                }
                //draw number menu
                for (int i = 0; i  < digitDisplay.size(); i++) {
                    digitDisplay[i].setPosition(21 * i, game_board.Get_Rows() * 32);
                    window.draw(digitDisplay[i]);
                }
                //if lose: change face to :(
                if (game_state == -1) {
                    face.setTexture(TextureManager::GetTexture("face_lose"));
                }
                //if win: change face to B)
                else if (game_state == 1) {
                    face.setTexture(TextureManager::GetTexture("face_win"));
                }
                //change numbers based on score
                SetDigitMap(score, digitDisplay, digitOptions);
                /*
                * FOR TESTING
                * game_board.print_mines();
                */
                window.draw(face);
                window.display();
        }
        //clear texturemanager
        TextureManager::Clear();
    }
    //reset function to set all game instance variables to default
    void Reset() {
        face.setTexture(TextureManager::GetTexture("face_happy"));
        //numbers.clear();
        game_state = 0;
        mine_pressed = false;
        all_revealed = false;
        game_board.Reset();
        score = game_board.Get_Mines();
        SetDigitMap(score, digitDisplay, digitOptions);
    }
private:
    int width;
    int height;
    int score;
    int game_state;
    int tilesRevealed;
    bool mine_pressed;
    bool all_revealed;
    vector<Sprite>digitOptions;
    vector<Sprite>digitDisplay;
    Sprite face, mine, flag;
    Sprite digits[12];
    Sprite numbers_textures[8];
    Board game_board;
    RenderWindow window;
    FloatRect faceBounds;
    void LoadNumbers() {
        for (int i = 0; i < 8; i++) {
            numbers_textures[i].setTexture(TextureManager::GetTexture("number_" + to_string(i + 1)));
        }
    }
    void LoadDigits() { // digitOptions idx 0-9 is 0-9, 10 is sign
        for (int i = 0; i < 12; i++) {
            digits[i].setTexture(TextureManager::GetTexture("digits"));
        }
        digits[11].setTexture(TextureManager::GetTexture("digits"));
        for (int i = 1; i < 12; i++) {
            digits[i].setTextureRect(IntRect((i - 1) * 21, 0, 21, 32));
        }
        for (int i = 1; i < 12; i++) {
            digitOptions.push_back(digits[i]);
        }
    }
    //load all member sprites
    void LoadAll() {
        LoadDigits();
        LoadNumbers();
        face.setTexture(TextureManager::GetTexture("face_happy"));
        mine.setTexture(TextureManager::GetTexture("mine"));
        flag.setTexture(TextureManager::GetTexture("flag"));
    }
    //draws menu
    void DrawMenu() {
        digits[0].setPosition(Vector2f(0, (game_board.Get_Rows() * 32)));
        face.setPosition(Vector2f(game_board.Get_Cols() * 32 / 2 - 16, (game_board.Get_Rows() * 32)));
    }
};


/*MAIN: create game and run the loop*/
int main() {
    Minesweeper game;
    game.GameLoop();
    return 0;
}