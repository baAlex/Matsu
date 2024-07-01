/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


static int RenderTomHigh(double sampling_frequency, double** out)
{
	auto envelope = AdEnvelope(0.0, 280.0, sampling_frequency);

	auto oscillator = Oscillator(240.0, 190.0, 0.15, 0.0, 280.0, sampling_frequency);

	// auto noise = NoiseGenerator();
	// auto hp = TwoPolesFilter<FilterType::Highpass>(2200.0, 0.75, sampling_frequency);
	// auto lp1 = OnePoleFilter<FilterType::Lowpass>(2200.0, sampling_frequency);
	// auto lp2 = TwoPolesFilter<FilterType::Lowpass>(16000.0, 0.5, sampling_frequency);

	// const double noise_gain = 0.0;
	const double oscillator_gain = 1.0;

	// Render
	for (int x = 0; x < envelope.GetTotalSamples(); x += 1)
	{
		const double e = envelope.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 8.0); });

		const double o = oscillator.Step( //
		    [](double x) { return 1.0 - ExponentialEasing(1.0 - x, 8.0); });

		// double n = noise.Step();
		// n = hp.Step(n);
		// n = lp2.Step(n);
		// n = lp1.Step(n);

		const double mix = (o * oscillator_gain /*+ n * noise_gain*/) * e;

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
	RenderTomHigh(SAMPLING_FREQUENCY, &cursor);

	ExportS24(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer), "606-tom-high.wav");
	ExportF64(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer), "606-tom-high-64.wav");

	return 0;
}
