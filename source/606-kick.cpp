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
	double oscillator1_length;
	double oscillator1_gain;
	double oscillator1_frequency;
	double oscillator1_feedback;

	double oscillator2_length;
	double oscillator2_gain;
	double oscillator2_frequency;
	double oscillator2_feedback;

	double click_gain;

	static Settings Default()
	{
		return {
		    300.0, // Oscillator 1 length
		    0.8,   // Oscillator 1 gain
		    60.0,  // Oscillator 1 frequency
		    0.0,   // Oscillator 1 feedback

		    70.0,  // Oscillator 2 length
		    0.4,   // Oscillator 2 gain
		    120.0, // Oscillator 2 frequency
		    0.1,   // Oscillator 2 feedback

		    1.0 // Click gain
		};
	}
};

static size_t RenderKick(Settings settings, double sampling_frequency, double* output)
{
	auto envelope1 = AdEnvelope(0.0, settings.oscillator1_length, 0.01, 8.0, sampling_frequency);
	auto envelope2 = AdEnvelope(0.0, settings.oscillator2_length, 0.01, 8.0, sampling_frequency);

	auto oscillator1 =
	    Oscillator(settings.oscillator1_frequency, settings.oscillator1_frequency, settings.oscillator1_feedback, 0.0,
	               settings.oscillator1_length, 8.0, sampling_frequency);
	auto oscillator2 =
	    Oscillator(settings.oscillator2_frequency, settings.oscillator2_frequency, settings.oscillator2_feedback, 0.0,
	               settings.oscillator2_length, 8.0, sampling_frequency);

	const int click_attack = static_cast<int>(50.0 * (sampling_frequency / 44100.0)); // TODO
	const int click_decay = static_cast<int>(64.0 * (sampling_frequency / 44100.0));  // Ditto
	const int click_length = click_attack + click_decay;

	const int samples = click_length + Max(envelope1.GetTotalSamples(), envelope2.GetTotalSamples());

	// Render click
	double max_level = 0.0;
	for (int i = 0; i < click_length; i += 1)
	{
		const double e1 = 0.7;
		const double e2 = 2.35;
		const double e3 = 1.14;
		const double e4 = 3.0;

		double signal;

		if (i < click_attack)
		{
			signal = static_cast<double>(i) / static_cast<double>(click_attack);
			signal = pow(signal, e1);
		}
		else
		{
			signal = 1.0 - (static_cast<double>(i - click_attack) / static_cast<double>(click_decay));
			signal = pow(signal, e3 + (e2 - e3) * pow(signal, e4));
		}

		signal = -signal * settings.click_gain;

		output[i] = signal;
		max_level = Max(abs(signal), max_level);
	}

	// Render body
	for (int i = click_length; i < samples; i += 1)
	{
		const double e1 = envelope1.Step();
		const double e2 = envelope2.Step();

		// Two oscillators
		const double o1 = oscillator1.Step();
		const double o2 = oscillator2.Step();

		const double signal = (o1 * e1 * settings.oscillator1_gain) + (o2 * e2 * settings.oscillator2_gain);

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

	size_t render_length = RenderKick(Settings::Default(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-kick.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-kick-64.wav");

	return 0;
}
