#pragma once

#include <random>

class Random {
public:
	Random();
	Random(const unsigned int seed);
	Random(std::seed_seq seq);

	// uniformly distributed float random number in range [0.0, 1.0)   (exclusive of 1.0f)
	float Uniform0_1();

	// uniformly distributed float random number in range [min, max)   (exclusive of max)
	float Uniform(float min, float max);

	// uniformly distributed integer in range [min, max]   (inclusive of max)
	int UniformInt(int min, int max);

private:
	std::mt19937 m_RandomEngine;
};
