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
	auto envelope_o = AdEnvelope(0.0, 280.0, sampling_frequency);
	auto envelope_n = AdEnvelope(0.0, 60.0, sampling_frequency);

	auto oscillator = Oscillator(200.0, 195.0, 0.2, 0.0, 280.0, sampling_frequency); // 190, 200

	auto noise = NoiseGenerator();
	auto lp = OnePoleFilter<FilterType::Lowpass>(1000.0, sampling_frequency);

	const double noise_gain = 0.13;
	const double oscillator_gain = 1.0;

	// Render
	for (int x = 0; x < envelope_o.GetTotalSamples(); x += 1)
	{
		const double e_o = envelope_o.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 8.0); });

		const double e_n = envelope_n.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 4.0); });

		const double o = oscillator.Step( //
		    [](double x) { return 1.0 - ExponentialEasing(1.0 - x, 8.0); });

		double n = noise.Step();
		n = lp.Step(n);

		const double mix = (o * e_o * oscillator_gain) + (n * e_n * noise_gain);

		**out = Clamp(mix, -1.0, 1.0);
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
