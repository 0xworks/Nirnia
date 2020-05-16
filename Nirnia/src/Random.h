#pragma once

#include <random>

class Random {
public:
	Random();
	Random(const unsigned int seed);
	Random(std::seed_seq seq);

	float Uniform0_1();

	float Uniform(float min, float max);

	int UniformInt(int min, int max);

private:
	std::mt19937 m_RandomEngine;
	std::uniform_int_distribution<std::mt19937::result_type> m_Distribution;
};
