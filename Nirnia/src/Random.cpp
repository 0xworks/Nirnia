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
	return static_cast<float>(m_Distribution(m_RandomEngine)) / static_cast<float>(std::numeric_limits<uint32_t>::max());
}


float Random::Uniform(float min, float max) {
	return min + ((static_cast<float>(m_Distribution(m_RandomEngine)) / static_cast<float>(std::numeric_limits<uint32_t>::max())) * (max - min));
}


int Random::UniformInt(int min, int max) {
	return min + static_cast<int>((static_cast<float>(m_Distribution(m_RandomEngine)) / static_cast<float>(std::numeric_limits<uint32_t>::max())) * (max - min));
}

