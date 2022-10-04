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

bool checkScore(int score) {
    if (score >= 0) {
        return false;
    }
    else {
        return true;
    }

}
void SetDigitMap(int score, map<int, Sprite>& digit, vector<Sprite>& digitOptions) {
    digit.clear();
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
        digit.emplace(0, digitOptions.at(10));
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            if (digs[i] == j) {
                digit.emplace(i + shift, digitOptions[i]);
            }
        }
    }
}
class Point {
public:
    int x;
    int y;
};
class Tile {
public:
    Tile(int x, int y) {
        coordinates.x = x;
        coordinates.y = y;
        Reset();
        state_sprite.setPosition((x) * 32, (y) * 32);
        num_sprite.setPosition((x) * 32, (y) * 32);
        bounds = state_sprite.getGlobalBounds();
    }
    Point GetLocation() {
        return coordinates;
    }
    void Reset() {
        number = 0;
        is_flagged = false;
        is_revealed = false;
        is_mine = false;
        adj_mine = false;
        state_sprite.setTexture(TextureManager::GetTexture("tile_hidden"));
        num_sprite.setTexture(TextureManager::GetTexture("tile_hidden"));
        CheckAdjacents();
    }
    void SetAdjacent(Tile* neighbor) {
        if (!(find(adjacents.begin(), adjacents.end(), neighbor) != adjacents.end())) {
            adjacents.push_back(neighbor);
            if (neighbor->is_mine) {
                adj_mine = true;
            }
        }
    }
    void SetNumber() {
        num_sprite.setTexture(TextureManager::GetTexture("number_"+to_string(number)));
    }
    void SetMine() {
        is_mine = true;
    }
    bool IsARevealed() {
        return is_revealed;
    }
    void IsRightClicked() {
        if (!is_flagged) {
            is_flagged = true;
        }
        else {
            is_flagged = false;
        }
    }
    int RevealTile(Tile& tile, int& curr_revealed) {
        if (!is_revealed && !is_mine && !is_flagged) {
            state_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
            is_revealed = true;
            SetNumber();
            curr_revealed++;
            if (!adj_mine){
                for (int i = 0; i < adjacents.size(); i++) {
                    RevealTile(*adjacents[i], curr_revealed);
                }
            }
            return 0;
        }
        else if (!is_revealed && !is_flagged) {
            state_sprite.setTexture(TextureManager::GetTexture("tile_revealed"));
            is_revealed = true;
            num_sprite.setTexture(TextureManager::GetTexture("mine"));
            return 1;
        }
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
                number++;
            }
        }
    }
    Sprite GetStateSprite() {
        return state_sprite;
    }
    Sprite GetNumSprite() {
        return num_sprite;
    }
private:
    int number;
    vector<Tile*> adjacents;
    Point coordinates;
    bool is_flagged;
    bool is_mine;
    bool is_revealed;
    bool adj_mine;
    Sprite state_sprite;
    Sprite num_sprite;
    FloatRect bounds;
};
class Board {
public:
    Board() {
        inFile.open("boards/config.cfg");
        ReadFile();
        inFile.close();
        CreateTiles();
        Reset();
    }
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
    void Reset() {
        curr_mines = 0;
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
    }
    void CreateTiles() {
        for (int i = 0; i < num_cols; i++) {
            for (int j = 0; j < num_rows; j++) {
                board.push_back(Tile(i, j)); // will have number 0, all specials false, hidden texture
            }
        }
    }
    void SetAdjacents() {
        for (int i = 0; i < board.size(); i++) {
            for (int j = (i - num_cols - 1); j < (i - num_cols + 1); j++) {
                if (j > 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(i).GetLocation().y + 1)) {
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
            for (int j = (i - 1); j < (i + 1); j += 2) {
                if (j > 0 && (board.at(i).GetLocation().x == board.at(i).GetLocation().x)) {
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
            for (int j = (i + num_cols - 1); j < (i + num_cols + 1); j++) {
                if (j > 0 && j < board.size() && (board.at(i).GetLocation().y == board.at(i).GetLocation().y - 1)) {
                    board.at(i).SetAdjacent(&board.at(j));
                }
            }
        }
    }
    void UpdateTile(int action[], int size) {https://draftlol.dawe.gg/T_cAvtlI
        for (int i = 0; i < size; i++) {
            if (action[i] == 1) {
                board[i].RevealTile(board[i], size); // change size to actual variable later
            }
        }
    }
    void UpdateTile(int index, bool& mine, bool& reveal) {
         mine = board[index].RevealTile(board[index], num_revealed); // mine becomes 1 when a mine is revealed
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
    vector<int> playState;
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
class Minesweeper {
public:
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
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        if (faceBounds.contains(mousePos.x, mousePos.y)) {
                            Reset();
                        }
                    }
                    else if (!mine_pressed && !all_revealed) {
                        for (int i = 0; i < game_board.Get_Board().size(); i++) {
                            game_board.UpdateTile(i, mine_pressed, all_revealed);
                        }
                    }
                }
            }
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                window.clear(Color::White);
                for (int i = 0; i < game_board.Get_Cols(); i++) {
                    for (int j = 0; j < game_board.Get_Rows(); j++) {
                        window.draw(game_board.Get_Tile(i, j).GetStateSprite());
                    }  
                }
                /*auto iter = flags.begin();
                for (; iter != flags.end(); iter++) {
                    window.draw(iter->second);
                }
                */
                for (int i = 0; i < mines.size(); i++) {
                    window.draw(mines.at(i));
                }
                /*
                auto iter2 = numbers.begin();
                for (; iter2 != numbers.end(); iter2++) {
                    window.draw(iter2->second);
                }
                */
                auto iter3 = digitCount.begin();
                for (; iter3 != digitCount.end(); iter3++) {
                    iter3->second.setPosition(21 * iter3->first, game_board.Get_Rows() * 32);
                    window.draw(iter3->second);
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
        SetDigitMap(score, digitCount, digitDisplay);
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
    vector<Sprite>digitDisplay;
    map<int, Sprite> numbers;
    map<int, Sprite> flags;
    map<int, Sprite> digitCount;
    vector<int> unrevealed;
    Sprite face, debug, mine, flag;
    Sprite digits[12];
    Sprite numbers_textures[8];
    Sprite digitSign;
    Board game_board;
    RenderWindow window;
    //FloatRect debugBounds;
    FloatRect faceBounds;
    void LoadNumbers() {
        for (int i = 0; i < 8; i++) {
            numbers_textures[i].setTexture(TextureManager::GetTexture("number_" + to_string(i + 1)));
        }
    }
    void LoadDigits() { // digitDisplay idx 0-9 is 0-9, 10 is sign
        for (int i = 0; i < 11; i++) {
            digits[i].setTexture(TextureManager::GetTexture("digits"));
        }
        digits[11].setTexture(TextureManager::GetTexture("digits"));
        for (int i = 1; i < 11; i++) {
            digits[i].setTextureRect(IntRect((i - 1) * 21, 0, 21, 32));
        }
        for (int i = 1; i < 11; i++) {
            digitDisplay.push_back(digits[i]);
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