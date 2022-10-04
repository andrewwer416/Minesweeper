#include "TextureManager.h"
unordered_map<string, sf::Texture>TextureManager::textures;

void TextureManager::LoadTexture(string name) {
	string path = "images/";
	path += name;
 	path += ".png";
	textures[name].loadFromFile(path);
}
sf::Texture& TextureManager::GetTexture(string name) {
	if (textures.find(name) == textures.end()) {
		LoadTexture(name);
	}
	return textures[name];
}
void TextureManager::Clear() {
	textures.clear();
}