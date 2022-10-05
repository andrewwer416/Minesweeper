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
        CheckAdjacents();
    }
    //setters
    void SetAdjacent(Tile* neighbor) {
        if (!(find(adjacents.begin(), adjacents.end(), neighbor) != adjacents.end())) {
            adjacents.push_back(neighbor);
            if (neighbor->is_mine) {
                adj_mine = true;
            }
        }
    }
    void SetFG() {
        if (number != 0) {
            fg_sprite.setTexture(TextureManager::GetTexture("number_" + to_string(number)));
        }
        else {
            fg_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
        }
    }
    void SetBG() {
        bg_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
    }
    void SetMine() {
        is_mine = true;
        bg_sprite.setTexture(TextureManager::GetTexture("mine"));
    }
    //actions
    void RightClick() {
        if (!is_flagged) {
            is_flagged = true;
            fg_sprite.setTexture(TextureManager::GetTexture("flag"));
        }
        else {
            is_flagged = false;
            fg_sprite.setTexture(TextureManager::GetTexture("tile_hidden"));
        }
    }
    int RevealTile(Tile& tile, int& curr_revealed) {
        if (!is_revealed && !is_mine && !is_flagged) {
            bg_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
            is_revealed = true;
            SetFG();
            SetBG();
            curr_revealed++;
            if (!adj_mine){
                for (int i = 0; i < adjacents.size(); i++) {
                    RevealTile(*adjacents[i], curr_revealed);
                }
            }
            return 0;
        }
        else if (!is_revealed && !is_flagged) {
            bg_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
            is_revealed = true;
            fg_sprite.setTexture(TextureManager::GetTexture("mine"));
            return 1;
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
    void CheckAdjacents() {
        for (int i = 0; i < adjacents.size(); i++) {
            if (adjacents[i]->is_mine) {
                cout << GetLocation().x << ", " << GetLocation().y << endl;
                cout << adjacents[i]->GetLocation().x << ", " << adjacents[i]->GetLocation().y << endl << endl;;
                number++;
                adj_mine = true;
            }
        }
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
            for (int j = (i - num_cols - 1); j <= (i - num_cols + 1); j++) {
                if (j >= 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(j).GetLocation().y + 1)) {
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
            for (int j = (i - 1); j <= (i + 1); j += 2) {
                if (j >= 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(j).GetLocation().y)) {
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
            for (int j = (i + num_cols - 1); j <= (i + num_cols + 1); j++) {
                if (j >= 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(j).GetLocation().y - 1)) {
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
        }
    }
    //function to update tile at an index
    void UpdateTile(int index, bool& leftClicked, bool& mine, bool& reveal) {
        if (leftClicked) {
            mine = board[index].RevealTile(board[index], num_revealed); // mine becomes 1 when a mine is revealed
        }
        else {
            board[index].RightClick();
        }
        if ((num_rows * num_cols) - num_revealed == num_mines) {
             reveal = true;
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
        Reset();
    }
    void GameLoop() {
        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }
                bool leftClick;
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        leftClick = true;
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        if (faceBounds.contains(mousePos.x, mousePos.y)) {
                            Reset();
                        }
                    }
                    if (event.mouseButton.button == sf::Mouse::Right) {
                        leftClick = false;
                    }
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    for (int i = 0; i < game_board.Get_Board().size(); i++) {
                        if (game_board.Get_Board()[i].checkBounds(mousePos)) {
                            game_board.UpdateTile(i, leftClick, mine_pressed, all_revealed);
                        }
                    }
                }
            }
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                window.clear(Color::White);
                for (int i = 0; i < game_board.Get_Rows(); i++) {
                    for (int j = 0; j < game_board.Get_Cols(); j++) {
                        window.draw(game_board.Get_Tile(j, i).GetBGSprite());
                        window.draw(game_board.Get_Tile(j, i).GetFGSprite());
                    }  
                }
                for (int i = 0; i < mines.size(); i++) {
                    window.draw(mines.at(i));
                }
                for (int i = 0; i  < digitDisplay.size(); i++) {
                    digitDisplay[i].setPosition(21 * i, game_board.Get_Rows() * 32);
                    window.draw(digitDisplay[i]);
                }
                window.draw(face);
                //window.draw(debug);
                window.display();
        }
        TextureManager::Clear();
    }
    void PressTile(int action[], int size) {
        
    }
    void Reset() {
        face.setTexture(TextureManager::GetTexture("face_happy"));
        //numbers.clear();
        mines.clear();
        flags.clear();
        mine_pressed = false;
        all_revealed = false;
        game_board.Reset();
        score = game_board.Get_Mines();
        SetDigitMap(score, digitDisplay, digitOptions);
    }
private:
    bool sign;
    int dig1;
    int dig2;
    int dig3;
    int nextMines;
    int width;
    int height;
    int score;
    int tilesRevealed;
    bool mine_pressed;
    bool all_revealed;
    bool isDebugMode;
    ifstream inFile;
    vector<Sprite> mines;
    vector<Sprite>digitOptions;
    map<int, Sprite> numbers;
    map<int, Sprite> flags;
    vector<Sprite>digitDisplay;
    vector<int> unrevealed;
    Sprite face, debug, mine, flag;
    Sprite digits[12];
    Sprite numbers_textures[8];
    Board game_board;
    RenderWindow window;
    //FloatRect debugBounds;
    FloatRect faceBounds;
    void LoadNumbers() {
        for (int i = 0; i < 8; i++) {
            numbers_textures[i].setTexture(TextureManager::GetTexture("number_" + to_string(i + 1)));
        }
    }
    void LoadDigits() { // digitOptions idx 0-9 is 0-9, 10 is sign
        for (int i = 0; i < 11; i++) {
            digits[i].setTexture(TextureManager::GetTexture("digits"));
        }
        digits[11].setTexture(TextureManager::GetTexture("digits"));
        for (int i = 1; i < 11; i++) {
            digits[i].setTextureRect(IntRect((i - 1) * 21, 0, 21, 32));
        }
        for (int i = 1; i < 11; i++) {
            digitOptions.push_back(digits[i]);
        }
    }
    void LoadAll() {
        LoadDigits();
        LoadNumbers();
        face.setTexture(TextureManager::GetTexture("face_happy"));
        //debug.setTexture(TextureManager::GetTexture("debug"));
        mine.setTexture(TextureManager::GetTexture("mine"));
        flag.setTexture(TextureManager::GetTexture("flag"));
    }
    void DrawMenu() {
        digits[0].setPosition(Vector2f(0, (game_board.Get_Rows() * 32)));
        face.setPosition(Vector2f(game_board.Get_Cols() * 32 / 2 - 16, (game_board.Get_Rows() * 32)));
        //debug.setPosition(Vector2f(numCols * 32 - (32 * 8), (numRows * 32)));
    }
};

int main() {
    Minesweeper game;
    game.GameLoop();
}