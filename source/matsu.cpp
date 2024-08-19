/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


using namespace matsu;


size_t matsu::MillisecondsToSamples(double sampling_frequency, double milliseconds)
{
	return static_cast<size_t>((milliseconds * sampling_frequency) / 1000.0);
}

double matsu::SamplesToMilliseconds(double sampling_frequency, size_t samples)
{
	return (static_cast<double>(samples) / sampling_frequency) * 1000.0;
}


uint64_t matsu::Random(uint64_t* state)
{
	// https://en.wikipedia.org/wiki/Xorshift#xorshift*
	uint64_t x = *state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	*state = x;

	return x * static_cast<uint64_t>(0x2545F4914F6CDD1D);
}

double matsu::RandomFloat(uint64_t* state)
{
	// https://prng.di.unimi.it/

	// Hexadecimal floating literals are a C++17 feature
	// return (static_cast<double>(x) * 0x1.0p-53);

	const uint64_t x = Random(state) >> static_cast<uint64_t>(11);
	return (static_cast<double>(x) * 1.11022302462515654042363166809e-16);
}


double matsu::ExponentialEasing(double x, double e)
{
	if (e == 0.0)
		return x;

	return ((exp(e * fabs(x)) - 1.0) / (exp(e) - 1.0)) * Sign(x);
}

double matsu::Distortion(double x, double e, double asymmetry)
{
	if (e == 0.0)
		return x;

	if (x > 0.0)
		return (exp(x * e) - 1.0) / (exp(e) - 1.0);

	return -((exp(-x * e * (1.0 / asymmetry)) - 1.0) / (exp(e * (1.0 / asymmetry)) - 1.0)) * asymmetry;
}


matsu::AdEnvelope::AdEnvelope(double sampling_frequency, double attack_duration, double decay_duration,
                              double attack_shape, double decay_shape)
{
	m_attack = static_cast<double>(MillisecondsToSamples(sampling_frequency, attack_duration));
	m_decay = static_cast<double>(MillisecondsToSamples(sampling_frequency, decay_duration));
	m_attack = Max(m_attack, 1.0);
	m_decay = Max(m_decay, 1.0);

	m_attack_shape = attack_shape;
	m_decay_shape = decay_shape;

	m_x = 0;
}

size_t matsu::AdEnvelope::GetTotalSamples() const
{
	return static_cast<size_t>(ceil(m_attack + m_decay));
}

double matsu::AdEnvelope::Step()
{
	const double dx = static_cast<double>(m_x);
	m_x = m_x + 1;

	if (dx < m_attack)
		return ExponentialEasing(dx / m_attack, m_attack_shape);
	else if (dx < m_attack + m_decay)
		return ExponentialEasing(1.0 - (dx - m_attack) / m_decay, m_decay_shape);

	return 0.0;
}


matsu::NoiseGenerator::NoiseGenerator(uint64_t seed)
{
	m_state = Max(seed, static_cast<uint64_t>(1));
}

double matsu::NoiseGenerator::Step()
{
	return RandomFloat(&m_state) * 2.0 - 1.0;
}


matsu::Oscillator::Oscillator(double sampling_frequency, double frequency, double feedback_level, double sweep_duration,
                              double sweep_multiply, double sweep_shape)
{
	sampling_frequency = Max(sampling_frequency, 1.0);
	frequency = Max(frequency, 1.0);
	feedback_level = Clamp(feedback_level, 0.0, 1.0);
	sweep_duration = Max(sweep_duration, 0.0);
	sweep_multiply = Max(sweep_multiply, 0.0);

	m_phase = 0.0;
	m_phase_delta_a = (frequency / sampling_frequency) * (M_PI * 2.0);
	m_phase_delta_b = m_phase_delta_a * sweep_multiply;

	m_sweep = 0.0;
	if (sweep_duration != 0.0)
		m_sweep_delta = 1.0 / static_cast<double>(MillisecondsToSamples(sweep_duration, sampling_frequency));
	else
		m_sweep_delta = 0.0;
	m_sweep_shape = sweep_shape;

	m_feedback = 0.0;
	m_feedback_level = feedback_level / (M_PI / 2.0); // For a maximum 'feedback_level' of 1
}

double matsu::Oscillator::Step()
{
	const double sweep = ExponentialEasing(m_sweep, m_sweep_shape);
	const double phase_delta = Mix(m_phase_delta_a, m_phase_delta_b, sweep);
	const double feedback_level = Mix(m_feedback_level, 0.0, sweep);

	m_phase = fmod(m_phase + phase_delta, (M_PI * 2.0));
	m_sweep = Min(m_sweep + m_sweep_delta, 1.0);

	const double signal = sin(m_phase + m_feedback);
	m_feedback = (m_feedback + signal) * feedback_level;

	return signal;
}


matsu::SquareOscillator::SquareOscillator(double sampling_frequency, double frequency, double phase)
{
	sampling_frequency = Max(sampling_frequency, 1.0);
	frequency = Max(frequency, 1.0);

	m_phase = phase;
	m_phase_delta = (frequency / sampling_frequency) * (M_PI * 2.0);
}

double matsu::SquareOscillator::Step()
{
	m_phase = fmod(m_phase + m_phase_delta, (M_PI * 2.0));
	return (m_phase > M_PI) ? -1.0 : 1.0;
}


matsu::OnePoleFilter::OnePoleFilter(double sampling_frequency, FilterType type, double cutoff)
{
	sampling_frequency = Max(sampling_frequency, 1.0);
	cutoff = Max(cutoff, 1.0);

	m_s = 0.0;
	m_c = 1.0 - exp((-M_PI * 2.0) * (cutoff / sampling_frequency));
	m_type = (type == FilterType::Lowpass) ? 0 : 1;
}

double matsu::OnePoleFilter::Step(double x)
{
	m_s = m_s + (x - m_s) * m_c;
	return (m_type == 0) ? (m_s) : (x - m_s);
}

static size_t X1 = 0; // To use as indices
static size_t Y1 = 1;
static size_t X2 = 2;
static size_t Y2 = 3;

static size_t B1 = X1; // Arranged to make a future
static size_t A1 = Y1; // Simd rewrite easier
static size_t B2 = X2;
static size_t A2 = Y2;

matsu::TwoPolesFilter::TwoPolesFilter(double sampling_frequency, FilterType type, double cutoff, double q)
{
	sampling_frequency = Max(sampling_frequency, 1.0);
	cutoff = Max(cutoff, 1.0);
	q = Max(q, 0.01);

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

	switch (type)
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

double matsu::TwoPolesFilter::Step(double x)
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


// ----


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

void matsu::ExportAudioS24(const double* input, double sampling_frequency, size_t length, const char* filename)
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
			const auto v = static_cast<int32_t>((Clamp(*in, -1.0, 1.0)) * 127.0 * 8388607.0);
			memcpy(&conversion, &v, sizeof(int32_t));
			conversion >>= 7;

			// TODO, endianness
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

void matsu::ExportAudioF64(const double* input, double sampling_frequency, size_t length, const char* filename)
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


// ----


#include "thirdparty/pffft/pffft.h"


matsu::Analyser::Analyser(size_t window_length, size_t overlaps_no)
{
	m_window_length = window_length;
	m_to_read_length = m_window_length / overlaps_no;

	// Initialize Pffft
	if ((m_pffft = pffft_new_setup(static_cast<int>(m_window_length), PFFFT_REAL)) == nullptr)
		goto return_failure;

	// Allocate memory
	if ((m_buffer_a = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * m_window_length))) == nullptr ||
	    (m_buffer_b = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * m_window_length))) == nullptr ||
	    (m_window_a = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * m_window_length))) == nullptr ||
	    (m_window_b = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * m_window_length))) == nullptr ||
	    (m_work_area = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * m_window_length))) == nullptr)
	{
		goto return_failure;
	}

	// Bye
	return;

return_failure:
	if (m_pffft != nullptr)
		pffft_destroy_setup(reinterpret_cast<PFFFT_Setup*>(m_pffft));
	if (m_buffer_a != nullptr)
		pffft_aligned_free(m_buffer_a);
	if (m_buffer_b != nullptr)
		pffft_aligned_free(m_buffer_b);
	if (m_window_a != nullptr)
		pffft_aligned_free(m_window_a);
	if (m_window_b != nullptr)
		pffft_aligned_free(m_window_b);
	if (m_work_area != nullptr)
		pffft_aligned_free(m_work_area);

	// TODO, return something, an exception maybe
}

matsu::Analyser::~Analyser()
{
	if (m_pffft != nullptr)
		pffft_destroy_setup(reinterpret_cast<PFFFT_Setup*>(m_pffft));
	if (m_buffer_a != nullptr)
		pffft_aligned_free(m_buffer_a);
	if (m_buffer_b != nullptr)
		pffft_aligned_free(m_buffer_b);
	if (m_window_a != nullptr)
		pffft_aligned_free(m_window_a);
	if (m_window_b != nullptr)
		pffft_aligned_free(m_window_b);
	if (m_work_area != nullptr)
		pffft_aligned_free(m_work_area);
}

matsu::Analyser::Output matsu::Analyser::Analyse(std::function<size_t(size_t, float*)> input_callback,
                                                 std::function<size_t(size_t, float*)> input_callback2,
                                                 std::function<void(size_t, size_t, const float*)> output_callback)
{
	matsu::Analyser::Output ret = {};
	long diff_sum = 0;
	long diff_div = 0;

	memset(m_buffer_a, 0, sizeof(float) * m_window_length);
	memset(m_buffer_b, 0, sizeof(float) * m_window_length);

	while (1)
	{
		// Read buffers
		const size_t read1 = input_callback(m_to_read_length, m_buffer_a + m_window_length - m_to_read_length);
		if (read1 != m_to_read_length)
			memset(m_buffer_a + m_window_length - m_to_read_length + read1, 0,
			       sizeof(float) * (m_to_read_length - read1));

		const size_t read2 = input_callback2(m_to_read_length, m_buffer_b + m_window_length - m_to_read_length);
		if (read2 != m_to_read_length)
			memset(m_buffer_b + m_window_length - m_to_read_length + read2, 0,
			       sizeof(float) * (m_to_read_length - read2));

		// Apply window function
		for (size_t i = 0; i < m_window_length; i += 1)
		{
			// Hann window
			const float window = 0.5f * (1.0f - cosf((static_cast<float>(M_PI * 2.0) * static_cast<float>(i)) /
			                                         static_cast<float>(m_window_length)));

			m_window_a[i] = m_buffer_a[i] * window;
		}

		if (read2 != 0)
		{
			for (size_t i = 0; i < m_window_length; i += 1)
			{
				const float window = 0.5f * (1.0f - cosf((static_cast<float>(M_PI * 2.0) * static_cast<float>(i)) /
				                                         static_cast<float>(m_window_length)));

				m_window_b[i] = m_buffer_b[i] * window;
			}
		}

		// Fourier
		pffft_transform_ordered(reinterpret_cast<PFFFT_Setup*>(m_pffft), m_window_a, m_window_a, m_work_area,
		                        PFFFT_FORWARD);
		ret.windows += 1;

		if (read2 != 0)
			pffft_transform_ordered(reinterpret_cast<PFFFT_Setup*>(m_pffft), m_window_b, m_window_b, m_work_area,
			                        PFFFT_FORWARD);

		// Prepare spectrum
		if (read2 == 0)
		{
			// Convert to magnitude and de-interleave
			for (size_t i = 0; i < m_window_length / 2; i += 1)
			{
				const float magnitude = sqrtf(powf(m_window_a[i * 2 + 0], 2.0f) + powf(m_window_a[i * 2 + 1], 2.0f)) /
				                        static_cast<float>(m_window_length / 2);
				m_window_a[i] = magnitude;
			}
		}
		else
		{
			// Same as above, tho comparing spectrums this time
			for (size_t i = 0; i < m_window_length / 2; i += 1)
			{
				const float magnitude1 = sqrtf(powf(m_window_a[i * 2 + 0], 2.0f) + powf(m_window_a[i * 2 + 1], 2.0f)) /
				                         static_cast<float>(m_window_length / 2);
				const float magnitude2 = sqrtf(powf(m_window_b[i * 2 + 0], 2.0f) + powf(m_window_b[i * 2 + 1], 2.0f)) /
				                         static_cast<float>(m_window_length / 2);

				m_window_a[i] = sqrtf(powf(magnitude1 - magnitude2, 2.0f)); // Distance
				diff_sum += static_cast<long>(m_window_a[i] * 10000.0f);
				diff_div += 1;
			}
		}

		// Output
		output_callback(ret.windows, m_window_length, m_window_a);

		// Scroll buffers
		bool we_still_have_content = false;

		for (size_t i = 0; i < (m_window_length - m_to_read_length); i += 1)
		{
			m_buffer_a[i] = m_buffer_a[i + m_to_read_length];

			if (m_buffer_a[i] != 0.0)
				we_still_have_content = true;
		}

		if (read2 != 0)
		{
			for (size_t i = 0; i < (m_window_length - m_to_read_length); i += 1)
				m_buffer_b[i] = m_buffer_b[i + m_to_read_length];
		}

		// Next step?
		if (we_still_have_content == false)
		{
			if (read1 != static_cast<drwav_uint64>(m_to_read_length))
				break;

			if (read2 != static_cast<drwav_uint64>(m_to_read_length))
				break;
		}
	}

	// Bye!
	if (diff_div != 0)
		ret.difference = static_cast<double>(diff_sum) / static_cast<double>(diff_div);

	return ret;
}
