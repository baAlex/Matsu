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

#include <math.h>
#include <stdint.h>
#include <vector>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"

#define DR_WAV_IMPLEMENTATION
#include "thirdparty/dr_libs/dr_wav.h"
#pragma clang diagnostic pop

#else
#define DR_WAV_IMPLEMENTATION
#include "thirdparty/dr_libs/dr_wav.h"
#endif


#ifndef M_PI
#define M_PI 3.141592653589793238463
#endif

#define M_PI_TWO 6.283185307179586476925


// clang-format off
template <typename T> T Max(T a, T b)            { return (a > b) ? a : b; }
template <typename T> T Min(T a, T b)            { return (a < b) ? a : b; }
template <typename T> T Mix(T x, T y, T a)       { return x + (y - x) * a; }
template <typename T> T Sign(T x)                { return (x > 0.0) ? 1.0 : -1.0; }
template <typename T> T Clamp(T v, T min, T max) { return Max(min, Min(v, max)); }
// clang-format on


inline int MillisecondsToSamples(double milliseconds, double sampling_frequency)
{
	return static_cast<int>((milliseconds * sampling_frequency) / 1000.0);
}

inline double SamplesToMilliseconds(int samples, double sampling_frequency)
{
	return (static_cast<double>(samples) / sampling_frequency) * 1000.0;
}

inline double SemitoneDetune(double x)
{
	if (x > 0.0)
		return pow(2.0, x / 12.0);

	return 1.0 / pow(2.0, fabs(x) / 12.0);
}

inline uint64_t Random(uint64_t* state)
{
	// https://en.wikipedia.org/wiki/Xorshift#xorshift*
	uint64_t x = *state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	*state = x;

	return x * static_cast<uint64_t>(0x2545F4914F6CDD1D);
}

inline double ExponentialEasing(double x, double a)
{
	return ((exp(a * fabs(x)) - 1.0) / (exp(a) - 1.0)) * Sign(x);
}

inline double Distortion(double x, double d, double asymmetry)
{
	if (x > 0.0)
		return (exp(x * d) - 1.0) / (exp(d) - 1.0);

	return -((exp(-x * d * (1.0 / asymmetry)) - 1.0) / (exp(d * (1.0 / asymmetry)) - 1.0)) * asymmetry;
}


enum class FilterType
{
	Lowpass,
	Highpass
};

template <FilterType TYPE> class OnePoleFilter
{
  public:
	OnePoleFilter(double cutoff, double sampling_frequency)
	{
		m_s = 0.0;
		m_c = 1.0 - exp((-M_PI * 2.0) * (cutoff / sampling_frequency));
	}

	double Step(double x)
	{
		m_s += (x - m_s) * m_c;
		return (TYPE == FilterType::Lowpass) ? (m_s) : (x - m_s);
	}

  private:
	double m_s;
	double m_c;
};

template <FilterType TYPE> class TwoPolesFilter
{
	static constexpr size_t X1 = 0; // To use as indices
	static constexpr size_t Y1 = 1;
	static constexpr size_t X2 = 2;
	static constexpr size_t Y2 = 3;

	static constexpr size_t B1 = X1; // Arranged to make a future
	static constexpr size_t A1 = Y1; // Simd rewrite easier
	static constexpr size_t B2 = X2;
	static constexpr size_t A2 = Y2;

  public:
	TwoPolesFilter(double cutoff, double q, double sampling_frequency)
	{
		// Cookbook formulae for audio equalizer biquad filter coefficients
		// Robert Bristow-Johnson
		// https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html

		const double wo = (2.0 * M_PI) * (cutoff / sampling_frequency);
		const double alpha = sin(wo) / (2.0 * q);
		double a0 = 1.0;

		m_s[0] = 0.0;
		m_s[1] = 0.0;
		m_s[2] = 0.0;
		m_s[3] = 0.0;

		switch (TYPE)
		{
		case FilterType::Lowpass:
		{
			m_c_b0 = (1.0 - cos(wo)) / 2.0;
			m_c[B1] = 1.0 - cos(wo);
			m_c[B2] = (1.0 - cos(wo)) / 2.0;

			a0 = 1.0 + alpha;
			m_c[A1] = -2.0 * cos(wo);
			m_c[A2] = 1.0 - alpha;

			break;
		}
		case FilterType::Highpass:
		{
			m_c_b0 = (1.0 + cos(wo)) / 2.0;
			m_c[B1] = -(1.0 + cos(wo));
			m_c[B2] = (1.0 + cos(wo)) / 2.0;

			a0 = 1.0 + alpha;
			m_c[A1] = -2.0 * cos(wo);
			m_c[A2] = 1.0 - alpha;

			break;
		}
		}

		// Optimization, division used to be in loop body [a]
		m_c_b0 /= a0;
		m_c[B1] /= a0;
		m_c[B2] /= a0;
		m_c[A1] /= a0;
		m_c[A2] /= a0;

		// Flip, in [a] used to be subtractions
		m_c[A1] = -m_c[A1];
		m_c[A2] = -m_c[A2];
	}

	double Step(double x)
	{
		const double y = (m_c_b0 * x)                                //
		                 + (m_c[B1] * m_s[X1]) + (m_c[B2] * m_s[X2]) // [a]
		                 + (m_c[A1] * m_s[Y1]) + (m_c[A2] * m_s[Y2]);

		m_s[Y2] = m_s[Y1];
		m_s[X2] = m_s[X1];
		m_s[Y1] = y;
		m_s[X1] = x;

		return y;
	}

  private:
	double m_c[4];
	double m_s[4];
	double m_c_b0;
};


class AdEnvelope
{
  public:
	AdEnvelope(double attack_duration, double decay_duration, double sampling_frequency)
	{
		m_attack = static_cast<double>(MillisecondsToSamples(attack_duration, sampling_frequency));
		m_decay = static_cast<double>(MillisecondsToSamples(decay_duration, sampling_frequency));
	}

	int GetTotalSamples() const
	{
		return static_cast<int>(ceil(m_attack + m_decay));
	}

	template <typename LAMBDA1, typename LAMBDA2> // 'std::function' incurs in lot of allocations
	double Get(int x, LAMBDA1 a_easing, LAMBDA2 d_easing)
	{
		const double dx = static_cast<double>(x);

		if (dx < m_attack)
			return a_easing(dx / m_attack);
		else if (dx < m_attack + m_decay)
			return d_easing(1.0 - (dx - m_attack) / m_decay);

		return 0.0;
	}

  private:
	double m_attack;
	double m_decay;
};


class NoiseGenerator
{
  public:
	NoiseGenerator(uint64_t initial_seed = 1)
	{
		m_state = Max(initial_seed, static_cast<uint64_t>(1));
	}

	double Step()
	{
		// https://prng.di.unimi.it/

		// Hexadecimal floating literals are a C++17 feature
		// return (static_cast<double>(x) * 0x1.0p-53) * 2.0 - 1.0;

		const uint64_t x = Random(&m_state) >> static_cast<uint64_t>(11);
		return (static_cast<double>(x) * 1.11022302462515654042363166809e-16) * 2.0 - 1.0;
	}

  private:
	uint64_t m_state;
};


class Oscillator
{
  public:
	Oscillator(double frequency_a, double frequency_b, double feedback_level_a, double feedback_level_b,
	           double duration, double sampling_frequency)
	{
		m_phase = 0.0;
		m_phase_delta_a = (frequency_a / sampling_frequency) * M_PI_TWO;
		m_phase_delta_b = (frequency_b / sampling_frequency) * M_PI_TWO;

		m_sweep = 0.0;
		m_sweep_delta = 1.0 / static_cast<double>(MillisecondsToSamples(duration, sampling_frequency));

		m_feedback = 0.0;
		m_feedback_level_a = feedback_level_a / (M_PI / 2.0); // For a maximum 'feedback_level' of 1
		m_feedback_level_b = feedback_level_b / (M_PI / 2.0); // Ditto
	}

	template <typename LAMBDA> double Step(LAMBDA s_easing)
	{
		const double s = Min(s_easing(m_sweep), 1.0);
		const double phase_delta = Mix(m_phase_delta_a, m_phase_delta_b, s);
		const double feedback_level = Mix(m_feedback_level_a, m_feedback_level_b, s);

		m_phase = fmod(m_phase + phase_delta, M_PI_TWO);
		m_sweep = Min(m_sweep + m_sweep_delta, 1.0);

		const double signal = sin(m_phase + m_feedback);
		m_feedback = (m_feedback + signal) * feedback_level;

		return signal;
	}

  private:
	double m_phase;
	double m_phase_delta_a;
	double m_phase_delta_b;

	double m_sweep;
	double m_sweep_delta;

	double m_feedback;
	double m_feedback_level_a;
	double m_feedback_level_b;
};


class SquareOscillator
{
  public:
	SquareOscillator(double frequency, double sampling_frequency)
	{
		m_phase = 0.0;
		m_phase_delta = (frequency / sampling_frequency) * M_PI_TWO;
	}

	double Step()
	{
		m_phase = fmod(m_phase + m_phase_delta, M_PI_TWO);
		return (m_phase > M_PI) ? -1.0 : 1.0;
	}

  private:
	double m_phase;
	double m_phase_delta;
};


inline void ExportS24(const double* input, double sampling_frequency, size_t length, const char* filename)
{
	auto export_buffer = reinterpret_cast<uint8_t*>(malloc(sizeof(uint8_t) * 3 * length));
	if (export_buffer == nullptr)
		return;

	// Convert to s24
	{
		uint8_t* out = export_buffer;
		uint32_t conversion;
		for (const double* in = input; in < (input + length); in += 1)
		{
			const auto v = static_cast<int32_t>((*in) * 127.0 * 8388607.0);
			memcpy(&conversion, &v, sizeof(int32_t));
			conversion >>= 7;

			*out++ = static_cast<uint8_t>((conversion >> 0) & 0xFF);
			*out++ = static_cast<uint8_t>((conversion >> 8) & 0xFF);
			*out++ = static_cast<uint8_t>((conversion >> 16) & 0xFF);
		}
	}

	// Encode
	{
		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_PCM;
		format.channels = 1;
		format.sampleRate = static_cast<drwav_uint32>(sampling_frequency);
		format.bitsPerSample = 24;

		drwav wav;
		drwav_init_file_write(&wav, filename, &format, nullptr);
		drwav_write_pcm_frames(&wav, static_cast<drwav_uint64>(length), export_buffer);
		drwav_uninit(&wav);
	}

	// Bye!
	free(export_buffer);
}


inline void ExportF64(const double* input, double sampling_frequency, size_t length, const char* filename)
{
	drwav_data_format format;
	format.container = drwav_container_riff;
	format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
	format.channels = 1;
	format.sampleRate = static_cast<drwav_uint32>(sampling_frequency);
	format.bitsPerSample = 64;

	drwav wav;
	drwav_init_file_write(&wav, filename, &format, nullptr);
	drwav_write_pcm_frames(&wav, static_cast<drwav_uint64>(length), input);
	drwav_uninit(&wav);
}

#endif
