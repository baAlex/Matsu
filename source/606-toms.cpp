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
	double click_gain;

	double frequency_a;
	double frequency_b;
	double feedback;

	static Settings DefaultTomHigh()
	{
		return {
		    280.0, // Oscillator length
		    60.0,  // Noise length
		    1.0,   // Oscillator gain
		    0.16,  // Noise gain
		    0.65,  // Click gain

		    200.0, // Frequency a
		    190.0, // Frequency b
		    0.2    // Feedback
		};
	}

	static Settings DefaultTomLow()
	{
		return {
		    430.0, // Oscillator length
		    60.0,  // Noise length
		    1.0,   // Oscillator gain
		    0.16,  // Noise gain
		    1.0,   // Click gain

		    180.0, // Frequency a
		    118.0, // Frequency b
		    0.15   // Feedback
		};
	}
};

static size_t RenderTom(Settings settings, double sampling_frequency, double* output)
{
	auto envelope_oscillator = AdEnvelope(0.0, settings.oscillator_length, 0.01, 8.0, sampling_frequency);
	auto envelope_noise = AdEnvelope(0.0, settings.noise_length, 0.01, 4.0, sampling_frequency);

	auto oscillator = Oscillator(settings.frequency_a, settings.frequency_b, settings.feedback, 0.00,
	                             settings.oscillator_length, 8.0, sampling_frequency);
	auto noise = NoiseGenerator();

	auto lp = OnePoleFilter<FilterType::Lowpass>(700.0, sampling_frequency);

	const int click_attack = static_cast<int>(10.0 * (sampling_frequency / 44100.0)); // TODO
	const int click_decay = static_cast<int>(40.0 * (sampling_frequency / 44100.0));  // Ditto
	const int click_length = click_attack + click_decay;

	const int samples = click_length + Max(envelope_oscillator.GetTotalSamples(), envelope_noise.GetTotalSamples());

	// Render click
	double max_level = 0.0;
	for (int i = 0; i < click_length; i += 1)
	{
		const double e1 = 0.5;
		const double e2 = -3.0;

		double signal;

		if (i < click_attack)
		{
			signal = static_cast<double>(i) / static_cast<double>(click_attack);
			signal = pow(sin(signal * 0.5 * M_PI), e1);
		}
		else
		{
			signal = 1.0 - (static_cast<double>(i - click_attack) / static_cast<double>(click_decay));
			signal = ((pow(2.0, e2 * signal) - 1.0) / (pow(2.0, e2) - 1.0)) * (1.0 - signal) +
			         sin(signal * 0.5 * M_PI) * signal;
		}

		signal = -signal * settings.click_gain;

		output[i] = signal;
		max_level = Max(abs(signal), max_level);
	}

	// Render body
	for (int i = click_length; i < samples; i += 1)
	{
		const double e_o = envelope_oscillator.Step();
		const double e_n = envelope_noise.Step();

		// Oscillator plus lowpass'ed noise
		const double o = oscillator.Step();
		const double n = lp.Step(noise.Step());

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

	size_t render_length = RenderTom(Settings::DefaultTomHigh(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-tom-high.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-tom-high-64.wav");

	render_length = RenderTom(Settings::DefaultTomLow(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-tom-low.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-tom-low-64.wav");

	return 0;
}
