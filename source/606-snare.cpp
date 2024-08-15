/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


struct Settings
{
	double oscillator_length;
	double noise_length;
	double oscillator_gain;
	double noise_gain;

	double frequency_a;
	double frequency_b;
	double feedback;

	static Settings Default()
	{
		return {
		    150.0, // Oscillator length
		    150.0, // Noise length
		    0.7,   // Oscillator gain
		    0.9,   // Noise gain

		    320.0, // Frequency a
		    190.0, // Frequency b
		    0.0    // Feedback
		};
	}
};

static size_t RenderSnare(Settings settings, double sampling_frequency, double* output)
{
	auto envelope_oscillator = AdEnvelope(2.0, settings.oscillator_length - 2.0, 0.01, 8.0, sampling_frequency);
	auto envelope_noise = AdEnvelope(2.0, settings.noise_length - 2.0, 0.01, 9.0, sampling_frequency);

	auto oscillator = Oscillator(settings.frequency_a, settings.frequency_b, settings.feedback, 0.0,
	                             settings.oscillator_length, 8.0, sampling_frequency);
	auto noise = NoiseGenerator();

	auto bp_a = TwoPolesFilter<FilterType::Highpass>(2700.0, 0.75, sampling_frequency);
	auto bp_b = TwoPolesFilter<FilterType::Lowpass>(16000.0, 0.5, sampling_frequency);
	auto bp_c = OnePoleFilter<FilterType::Lowpass>(2700.0, sampling_frequency);

	const int samples = Max(envelope_oscillator.GetTotalSamples(), envelope_noise.GetTotalSamples());

	// Render
	double max_level = 0.0;
	for (int i = 0; i < samples; i += 1)
	{
		const double e_o = envelope_oscillator.Step();
		const double e_n = envelope_noise.Step();

		// Oscillator plus bandpass'ed noise
		const double o = oscillator.Step();
		const double n = bp_c.Step(bp_b.Step(bp_a.Step(noise.Step())));

		const double signal = (o * e_o * settings.oscillator_gain) + (n * e_n * settings.noise_gain);

		// Done!
		output[i] = signal;
		max_level = Max(abs(signal), max_level);
	}

	// Normalize
	for (int i = 0; i < samples; i += 1)
	{
		output[i] = output[i] * (1.0 / max_level);
		output[i] = Clamp(output[i], -1.0, 1.0);
	}

	// Bye!
	return static_cast<size_t>(samples);
}


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double render_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main(int argc, const char* argv[])
{
	(void)argc;
	(void)argv;

	size_t render_length = RenderSnare(Settings::Default(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-snare.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-snare-64.wav");

	return 0;
}
