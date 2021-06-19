#include "Random.h"

#include <random>

#include "Utils/Leaks.h"

static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
static std::default_random_engine generator;

float Random() {
	return distribution(generator);
}
