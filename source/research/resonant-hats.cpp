/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "../matsu.hpp"


struct Settings
{
	double short_long_gain_ratio;
	double distortion;
	double distortion_symmetry;
	double noise_gain;

	static Settings Default()
	{
		return {
		    0.6,   // Short/long gain ratio
		    6.0,   // Distortion
		    0.125, // Distortion symmetry
		    0.06   // Noise gain
		};
	}
};

static size_t RenderHatOpen(Settings settings, double sampling_frequency, double* output)
{
	auto envelope_long = AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), 1500.0, sampling_frequency);
	auto envelope_short = AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), 500.0, sampling_frequency);
	auto noise = NoiseGenerator();

	auto oscillator_1 = SquareOscillator(619.0 * 1.38, sampling_frequency);
	auto oscillator_2 = SquareOscillator(437.0 * 1.12, sampling_frequency);
	auto oscillator_3 = SquareOscillator(415.0 * 1.67, sampling_frequency);
	auto oscillator_4 = SquareOscillator(365.0 * 1.16, sampling_frequency);
	auto oscillator_5 = SquareOscillator(306.0 * 1.28, sampling_frequency);
	auto oscillator_6 = SquareOscillator(245.0 * 1.43, sampling_frequency);

	auto bp_a = TwoPolesFilter<FilterType::Highpass>(6822.0, 3.5, sampling_frequency);
	auto bp_b = OnePoleFilter<FilterType::Lowpass>(7802.0, sampling_frequency);
	auto bp_c = OnePoleFilter<FilterType::Lowpass>(7951.0, sampling_frequency);
	auto bp_d = OnePoleFilter<FilterType::Lowpass>(12000.0, sampling_frequency);

	auto lp = TwoPolesFilter<FilterType::Lowpass>(14000.0, 0.5, sampling_frequency);
	auto hp = TwoPolesFilter<FilterType::Highpass>(6363.0, 0.5, sampling_frequency);

	// Render noise
	double max_level = 0.0;
	for (int x = 0; x < envelope_long.GetTotalSamples(); x += 1)
	{
		double signal = oscillator_1.Step() + oscillator_2.Step() + oscillator_3.Step() //
		                + oscillator_4.Step() + oscillator_5.Step() + oscillator_6.Step();
		signal = signal / 6.0;

		signal = bp_a.Step(signal);
		signal = bp_b.Step(signal);
		signal = bp_c.Step(signal);
		signal = bp_d.Step(signal);

		signal = lp.Step(signal);

		output[x] = signal;
		max_level = Max(signal, max_level);
	}

	// Normalize, as distortion depends on volume
	for (int x = 0; x < envelope_long.GetTotalSamples(); x += 1)
		output[x] = output[x] * (1.0 / max_level);

	// Distort and envelope it
	max_level = 0.0;
	for (int x = 0; x < envelope_long.GetTotalSamples(); x += 1)
	{
		const double e_l = envelope_long.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 2.5); });

		const double e_s = envelope_short.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 9.0); });

		double signal = output[x];

		signal = Distortion(signal, -settings.distortion, settings.distortion_symmetry);
		signal = hp.Step(signal);
		signal = signal * (e_l + e_s * settings.short_long_gain_ratio);

		signal = signal + noise.Step() * (settings.noise_gain * e_s * settings.short_long_gain_ratio);

		output[x] = signal;
		max_level = Max(signal, max_level);
	}

	// Normalize one last time
	for (int x = 0; x < envelope_long.GetTotalSamples(); x += 1)
	{
		output[x] = output[x] * (1.0 / max_level);
		output[x] = Clamp(output[x], -1.0, 1.0);
	}

	// Bye!
	return static_cast<size_t>(envelope_long.GetTotalSamples());
}


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double render_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main(int argc, const char* argv[])
{
	(void)argc;
	(void)argv;

	const size_t render_length = RenderHatOpen(Settings::Default(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "resonant-hats.wav");

	return 0;
}
