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
	double distortion;
	double distortion_symmetry;
	double noise_gain;

	static Settings Default()
	{
		return {
		    6.0,   // Distortion
		    0.125, // Distortion symmetry
		    0.02   // Noise gain
		};
	}
};

static size_t RenderHatClosed(Settings settings, double sampling_frequency, double* output)
{
	auto envelope = AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), 150.0, sampling_frequency);
	auto noise = NoiseGenerator();

	auto oscillator_1 = SquareOscillator(619.0 * 1.38, sampling_frequency);
	auto oscillator_2 = SquareOscillator(437.0 * 1.12, sampling_frequency);
	auto oscillator_3 = SquareOscillator(415.0 * 1.67, sampling_frequency);
	auto oscillator_4 = SquareOscillator(365.0 * 1.16, sampling_frequency);
	auto oscillator_5 = SquareOscillator(306.0 * 1.28, sampling_frequency);
	auto oscillator_6 = SquareOscillator(245.0 * 1.43, sampling_frequency);

	auto bp_a = TwoPolesFilter<FilterType::Highpass>(6900.0, 3.3, sampling_frequency);
	auto bp_b = OnePoleFilter<FilterType::Lowpass>(7800.0, sampling_frequency);
	auto bp_c = OnePoleFilter<FilterType::Lowpass>(7950.0, sampling_frequency);
	auto bp_d = OnePoleFilter<FilterType::Lowpass>(10000.0, sampling_frequency);

	auto hp = TwoPolesFilter<FilterType::Highpass>(8400.0, 0.75, sampling_frequency);
	auto lp = TwoPolesFilter<FilterType::Lowpass>(14000.0, 0.25, sampling_frequency);
	const double lp_wet = 0.75; // Original one seems to die lovely at 22 MHz, it can
	                            // be better filters, a nyquist sampling thing, or both.
	                            // This wet/dry mix can get us somewhat there. A proper
	                            // solution should be a weird '1.5 Poles Filter'.

	// Render metallic noise
	double max_level = 0.0;
	for (int x = 0; x < envelope.GetTotalSamples(); x += 1)
	{
		// Sum six square oscillators
		double signal = oscillator_1.Step() + oscillator_2.Step() + oscillator_3.Step() //
		                + oscillator_4.Step() + oscillator_5.Step() + oscillator_6.Step();
		signal = signal / 6.0;

		// Band pass them
		signal = bp_a.Step(signal);
		signal = bp_b.Step(signal);
		signal = bp_c.Step(signal);
		signal = bp_d.Step(signal);

		// Done!
		output[x] = signal;
		max_level = Max(signal, max_level);
	}

	// Normalize, as distortion depends on volume
	for (int x = 0; x < envelope.GetTotalSamples(); x += 1)
		output[x] = output[x] * (1.0 / max_level);

	// Distort and envelope it
	max_level = 0.0;
	for (int x = 0; x < envelope.GetTotalSamples(); x += 1)
	{
		const double e = envelope.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 9.0); });

		double signal = output[x];

		// Distort metallic noise, a highpass fix asymmetry
		signal = Distortion(signal, -settings.distortion, settings.distortion_symmetry);
		signal = hp.Step(signal);

		// Apply envelope
		signal = signal * e;

		// Add transient noise, also enveloped
		signal = signal + noise.Step() * (settings.noise_gain * e);

		// Lowpass filter, otherwise we will sound too digital
		signal = Mix(signal, lp.Step(signal), lp_wet);

		// Done!
		output[x] = signal;
		max_level = Max(signal, max_level);
	}

	// Normalize one last time
	for (int x = 0; x < envelope.GetTotalSamples(); x += 1)
	{
		output[x] = output[x] * (1.0 / max_level);
		output[x] = Clamp(output[x], -1.0, 1.0);
	}

	// Bye!
	return static_cast<size_t>(envelope.GetTotalSamples());
}


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double render_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main(int argc, const char* argv[])
{
	(void)argc;
	(void)argv;

	const size_t render_length = RenderHatClosed(Settings::Default(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-closed.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-closed-64.wav");

	return 0;
}
