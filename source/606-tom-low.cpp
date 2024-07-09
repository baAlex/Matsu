/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


static int RenderTomLow(double sampling_frequency, double** out)
{
	auto click = AdEnvelope(SamplesToMilliseconds(10, sampling_frequency),
	                        SamplesToMilliseconds(40, sampling_frequency), sampling_frequency);

	auto envelope_o = AdEnvelope(0.0, 430.0, sampling_frequency);
	auto envelope_n = AdEnvelope(0.0, 60.0, sampling_frequency);

	auto oscillator = Oscillator(180.0, 118.0, 0.15, 0.00, 430.0, sampling_frequency); // 150, 180 / 115, 120

	auto noise = NoiseGenerator();
	auto lp = OnePoleFilter<FilterType::Lowpass>(700.0, sampling_frequency);

	const double noise_gain = 0.16;
	const double oscillator_gain = 1.0;

	// Render
	for (int x = 0; x < click.GetTotalSamples(); x += 1)
	{
		const double e1 = 0.5;
		const double e2 = -2.0;

		const double signal = click.Get(
		    x,                                                      //
		    [&](double x) { return pow(sin(x * 0.5 * M_PI), e1); }, //
		    [&](double x)
		    { return ((pow(2.0, e2 * x) - 1.0) / (pow(2.0, e2) - 1.0)) * (1.0 - x) + sin(x * 0.5 * M_PI) * x; });

		**out = -signal;
		*out += 1;
	}

	for (int x = 0; x < Max(envelope_o.GetTotalSamples(), envelope_n.GetTotalSamples()); x += 1)
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
	RenderTomLow(SAMPLING_FREQUENCY, &cursor);

	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer), "606-tom-low.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer),
	               "606-tom-low-64.wav");

	return 0;
}
