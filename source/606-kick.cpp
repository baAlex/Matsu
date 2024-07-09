/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


static int RenderKick(double sampling_frequency, double** out)
{
	auto click = AdEnvelope(SamplesToMilliseconds(49, sampling_frequency),
	                        SamplesToMilliseconds(64, sampling_frequency), sampling_frequency);

	auto envelope1 = AdEnvelope(0.0, 300.0, sampling_frequency);
	auto envelope2 = AdEnvelope(0.0, 70.0, sampling_frequency);

	auto oscillator1 = Oscillator(60.0, 60.0, 0.0, 0.0, 300.0, sampling_frequency);
	auto oscillator2 = Oscillator(120.0, 120.0, 0.1, 0.1, 70.0, sampling_frequency);

	const double oscillator1_gain = 0.8;
	const double oscillator2_gain = 0.4;

	// Render
	for (int x = 0; x < click.GetTotalSamples(); x += 1)
	{
		const double e1 = 0.7;
		const double e2 = 2.35;
		const double e3 = 1.14;
		const double e4 = 3.0;

		const double signal = click.Get(
		    x,                                    //
		    [&](double x) { return pow(x, e1); }, //
		    [&](double x) { return pow(x, e3 + (e2 - e3) * pow(x, e4)); });

		**out = -signal;
		*out += 1;
	}

	for (int x = 0; x < Max(envelope1.GetTotalSamples(), envelope2.GetTotalSamples()); x += 1)
	{
		const double e1 = envelope1.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 8.0); });

		const double e2 = envelope2.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 8.0); });

		const double o1 = oscillator1.Step( //
		    [](double x) { return 1.0 - ExponentialEasing(1.0 - x, 8.0); });

		const double o2 = oscillator2.Step( //
		    [](double x) { return 1.0 - ExponentialEasing(1.0 - x, 8.0); });

		const double mix = (o1 * e1 * oscillator1_gain) + (o2 * e2 * oscillator2_gain);

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
	RenderKick(SAMPLING_FREQUENCY, &cursor);

	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer), "606-kick.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, static_cast<size_t>(cursor - render_buffer), "606-kick-64.wav");

	return 0;
}
