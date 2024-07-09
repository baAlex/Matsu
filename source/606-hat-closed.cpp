/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


static int RenderHatClosed(double sampling_frequency, double** out)
{
	auto envelope = AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), 140.0, sampling_frequency);

	auto oscillator_1 = SquareOscillator(619.0, sampling_frequency);
	auto oscillator_2 = SquareOscillator(437.0, sampling_frequency);
	auto oscillator_3 = SquareOscillator(415.0, sampling_frequency);
	auto oscillator_4 = SquareOscillator(365.0, sampling_frequency);
	auto oscillator_5 = SquareOscillator(306.0, sampling_frequency);
	auto oscillator_6 = SquareOscillator(245.0, sampling_frequency);

	auto o1 = Oscillator(7502.0, 7502.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o2 = Oscillator(6149.0, 6149.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o3 = Oscillator(5552.0, 5552.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o4 = Oscillator(4746.0, 4746.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o5 = Oscillator(3363.0, 3363.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o6 = Oscillator(1094.0, 1094.0, 0.0, 0.0, 1500.0, sampling_frequency);

	auto noise = NoiseGenerator();

	// Peculiar bandpass (12db lp and 24db hp, components)
	auto bp_a = TwoPolesFilter<FilterType::Lowpass>(6600.0, 0.6, sampling_frequency); // 6000, 6700
	auto bp_b = TwoPolesFilter<FilterType::Highpass>(6600.0, 0.5, sampling_frequency);
	auto bp_c = TwoPolesFilter<FilterType::Highpass>(6600.0, 0.5, sampling_frequency);

	// These two after envelope
	auto hp = TwoPolesFilter<FilterType::Highpass>(6000.0, 0.5, sampling_frequency);
	auto lp = OnePoleFilter<FilterType::Lowpass>(7800.0, sampling_frequency); // Too digital otherwise

	const double tss_gain = 3.0;
	const double clink_gain = 0.8;
	const double noise_gain = 0.8;

	// Render
	for (int x = 0; x < envelope.GetTotalSamples(); x += 1)
	{
		const double e = envelope.Get(
		    x,                                                   //
		    [](double x) { return x; },                          //
		    [](double x) { return ExponentialEasing(x, 9.0); }); // 10, 20

		// Metallic signal
		double metallic;
		{
			// Square oscillators
			metallic = oscillator_1.Step() + oscillator_2.Step() + oscillator_3.Step() //
			           + oscillator_4.Step() + oscillator_5.Step() + oscillator_6.Step();
			metallic /= 6.0;

			// Clink
			const auto easing = [](double x) { return x; };
			metallic += (o1.Step(easing) + o2.Step(easing) + o3.Step(easing) + //
			             o4.Step(easing) + o5.Step(easing) + o6.Step(easing)) *
			            0.05 * clink_gain;

			// Bandpass
			metallic = bp_b.Step(bp_a.Step(metallic));
			metallic = bp_c.Step(metallic);
			metallic = Clamp(metallic * 8.0, -1.0, 1.0); // Normalize and clip it
		}

		// Tsss
		double tss;
		{
			// Distortion
			tss = Distortion(metallic, -7.0, 0.5);
		}

		// Mix
		**out = lp.Step(hp.Step((tss * e * tss_gain)) + (noise.Step() * 0.06 * e * noise_gain));
		**out = Clamp(**out, -1.0, 1.0);
		*out += 1;
	}

	// Bye!
	return 0;
}


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double render_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main()
{
	double* cursor = render_buffer;
	RenderHatClosed(SAMPLING_FREQUENCY, &cursor);

	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer),
	               "606-hat-closed.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer),
	               "606-hat-closed-64.wav");

	return 0;
}
