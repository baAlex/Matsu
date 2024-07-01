/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


static int RenderSnare(double sampling_frequency, double** out)
{
	auto envelope_o = AdEnvelope(2.0, 150.0 - 2.0, sampling_frequency);
	auto envelope_n = AdEnvelope(2.0, 150.0 - 2.0, sampling_frequency);

	auto oscillator = Oscillator(320.0 /* 340 */, 190.0 /* 170 */, 0.0, 0.0, 150.0, sampling_frequency);

	auto noise = NoiseGenerator();
	auto hp = TwoPolesFilter<FilterType::Highpass>(2200.0 * SemitoneDetune(3.5), 0.75, sampling_frequency);
	auto lp1 = OnePoleFilter<FilterType::Lowpass>(2200.0 * SemitoneDetune(3.5), sampling_frequency);
	auto lp2 = TwoPolesFilter<FilterType::Lowpass>(16000.0, 0.5, sampling_frequency);

	const double noise_gain = 0.9;      // 0.9, 1.0
	const double oscillator_gain = 0.7; // 0.7

	// Render
	for (int x = 0; x < Max(envelope_o.GetTotalSamples(), envelope_n.GetTotalSamples()); x += 1)
	{
		const double e_o = envelope_o.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 8.0); });

		const double e_n = envelope_n.Get(
		    x,                                                   //
		    [](double x) { return x; },                          //
		    [](double x) { return ExponentialEasing(x, 9.0); }); // 8, 11

		const double o = oscillator.Step( //
		    [](double x) { return 1.0 - ExponentialEasing(1.0 - x, 8.0); });

		double n = noise.Step();
		n = hp.Step(n);
		n = lp2.Step(n);
		n = lp1.Step(n);

		const double mix = (o * e_o * oscillator_gain) + (n * e_n * noise_gain);

		**out = mix;
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
	RenderSnare(SAMPLING_FREQUENCY, &cursor);

	ExportS24(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer), "606-snare.wav");
	ExportF64(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer), "606-snare-64.wav");

	return 0;
}
