/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#ifndef MATSU_HPP
#define MATSU_HPP

#include <functional>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

namespace matsu
{

// clang-format off
template <typename T> T Max(T a, T b)            { return (a > b) ? a : b; }
template <typename T> T Min(T a, T b)            { return (a < b) ? a : b; }
template <typename T> T Mix(T x, T y, T a)       { return x + (y - x) * a; }
template <typename T> T Sign(T x)                { return (x > 0.0) ? 1.0 : -1.0; }
template <typename T> T Clamp(T v, T min, T max) { return Max(min, Min(v, max)); }
// clang-format on


void ExportAudioS24(const double* input, double sampling_frequency, size_t length, const char* filename);
void ExportAudioF64(const double* input, double sampling_frequency, size_t length, const char* filename);


size_t MillisecondsToSamples(double sampling_frequency, double milliseconds);
double SamplesToMilliseconds(double sampling_frequency, size_t samples);

uint64_t Random(uint64_t* state);
double RandomFloat(uint64_t* state);

double ExponentialEasing(double x, double e);
double Distortion(double x, double e, double asymmetry);


class NoiseGenerator
{
  public:
	NoiseGenerator(uint64_t seed = 1);
	double Step();

  private:
	uint64_t m_state;
};


class AdEnvelope
{
  public:
	AdEnvelope(double sampling_frequency, double attack_duration, double decay_duration, double attack_shape = -2.0,
	           double decay_shape = 8.0);
	size_t GetTotalSamples() const;
	double Step();

  private:
	double m_attack;
	double m_decay;
	double m_attack_shape;
	double m_decay_shape;
	size_t m_x;
};


class Oscillator
{
  public:
	Oscillator(double sampling_frequency, double frequency, double feedback_level = 0.0, double sweep_duration = 0.0,
	           double sweep_multiply = 0.0, double sweep_shape = -8.0);
	double Step();

  private:
	double m_phase;
	double m_phase_delta_a;
	double m_phase_delta_b;

	double m_sweep;
	double m_sweep_delta;
	double m_sweep_shape;

	double m_feedback;
	double m_feedback_level;
};


class SquareOscillator
{
  public:
	SquareOscillator(double sampling_frequency, double frequency, double phase = 0.0);
	double Step();

  private:
	double m_phase;
	double m_phase_delta;
};


enum class FilterType
{
	Lowpass,
	Highpass
};

class OnePoleFilter
{
  public:
	OnePoleFilter(double sampling_frequency, FilterType type, double cutoff);
	double Step(double x);

  private:
	double m_s;
	double m_c;
	int m_type;
};

class TwoPolesFilter
{
  public:
	TwoPolesFilter(double sampling_frequency, FilterType type, double cutoff, double q = 0.5);
	double Step(double x);

  private:
	double m_c[4];
	double m_s[4];
	double m_c_b0;
};


class Analyser
{
  public:
	struct Output
	{
		size_t windows;
		double difference;
	};

	Analyser(size_t window_length, size_t overlaps_no);
	~Analyser();

	Output Analyse(std::function<size_t(size_t, float*)> input_callback,
	               std::function<size_t(size_t, float*)> input_callback2,
	               std::function<void(size_t, size_t, const float*)> output_callback);

  private:
	size_t m_window_length;
	size_t m_to_read_length;

	void* m_pffft;

	float* m_buffer_a;
	float* m_buffer_b;
	float* m_window_a;
	float* m_window_b;

	float* m_work_area;
};

} // namespace matsu
#endif
