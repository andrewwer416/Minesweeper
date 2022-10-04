#include "Random.h"
#include <ctime>
std::mt19937 Random::random(time(0));

int Random::Number(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(random);
}