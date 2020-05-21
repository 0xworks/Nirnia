#include "Random.h"

#include <limits>

Random::Random() {
	m_RandomEngine.seed(std::random_device()());
}


Random::Random(const unsigned int seed) {
	m_RandomEngine.seed(seed);
}


Random::Random(std::seed_seq seq) {
	m_RandomEngine.seed(seq);
}


float Random::Uniform0_1() {
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	return distribution(m_RandomEngine);
}


float Random::Uniform(float min, float max) {
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(m_RandomEngine);
}


int Random::UniformInt(int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(m_RandomEngine);
}
