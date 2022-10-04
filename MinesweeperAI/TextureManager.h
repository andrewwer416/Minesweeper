#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
using namespace std;
class TextureManager
{
	static unordered_map<string, sf::Texture> textures;
	static void LoadTexture(string name);
public:
	static sf::Texture& GetTexture(string name);
	static void Clear();
};

