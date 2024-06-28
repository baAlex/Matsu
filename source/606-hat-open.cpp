/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


static int RenderHatOpen(double sampling_frequency, double** out)
{
	auto envelope_long = AdEnvelope(0.0, 1500.0, sampling_frequency);
	auto envelope_short = AdEnvelope(0.0, 900.0, sampling_frequency);

	auto oscillator_1 = SquareOscillator(245.0, sampling_frequency);
	auto oscillator_2 = SquareOscillator(415.0, sampling_frequency);
	auto oscillator_3 = SquareOscillator(306.0, sampling_frequency);
	auto oscillator_4 = SquareOscillator(437.0, sampling_frequency);
	auto oscillator_5 = SquareOscillator(365.0, sampling_frequency);
	auto oscillator_6 = SquareOscillator(619.0, sampling_frequency);

	auto o1 = Oscillator(2000.0, 2000.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o2 = Oscillator(7500.0, 7500.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o3 = Oscillator(8400.0, 8400.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o4 = Oscillator(6150.0, 6150.0, 0.0, 0.0, 1500.0, sampling_frequency);

	auto o5 = Oscillator(3300.0, 3300.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o6 = Oscillator(4800.0, 4800.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o7 = Oscillator(5500.0, 5500.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o8 = Oscillator(1200.0, 1200.0, 0.0, 0.0, 1500.0, sampling_frequency);

	auto noise = NoiseGenerator();

	// Peculiar bandpass (12 lp and 24 hp, components)
	auto bp_a = TwoPolesFilter<FilterType::Lowpass>(6300.0, 0.6, sampling_frequency); // 6000, 6700
	auto bp_b = TwoPolesFilter<FilterType::Highpass>(6300.0, 0.6, sampling_frequency);
	auto bp_c = TwoPolesFilter<FilterType::Highpass>(6300.0, 0.6, sampling_frequency);
	auto hp = TwoPolesFilter<FilterType::Highpass>(6300.0, 0.6, sampling_frequency);

	const double short_gain = 1.0;
	const double long_gain = 0.8;
	const double clink_gain = 0.75;

	// Render
	for (int x = 0; x < Max(envelope_long.GetTotalSamples(), envelope_short.GetTotalSamples()); x += 1)
	{
		const double e_l = envelope_long.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 2.5); });

		const double e_s = envelope_short.Get(
		    x,                                                    //
		    [](double x) { return x; },                           //
		    [](double x) { return ExponentialEasing(x, 15.0); }); // 10, 20

		// Metallic signal
		double metallic;
		{
			// Square oscillators
			metallic = oscillator_1.Step() + oscillator_2.Step() + oscillator_3.Step() //
			           + oscillator_4.Step() + oscillator_5.Step() + oscillator_6.Step();
			metallic /= 6.0;

			// Clink
			const auto easing = [](double x) { return x; };
			metallic += (o1.Step(easing) + o2.Step(easing) + o3.Step(easing) + o4.Step(easing) + //
			             o5.Step(easing) + o6.Step(easing) + o7.Step(easing) + o8.Step(easing)) *
			            0.05 * clink_gain;

			// Bandpass
			metallic = bp_b.Step(bp_a.Step(metallic));
			metallic = bp_c.Step(metallic);
			metallic = Clamp(metallic * 8.0, -1.0, 1.0); // Normalize and clip it
		}

		// Long tsss
		double l;
		{
			// Distortion
			l = Distortion(metallic, -8.0, 0.4);
		}

		// Short tsss
		double s;
		{
			// Distortion
			s = Distortion(metallic, -4.0, 0.8);
		}

		// Mix
		**out = hp.Step((l * e_l * long_gain) + (s * e_s * short_gain)) + (noise.Step() * 0.04 * e_s) +
		        (noise.Step() * 0.00125 * e_l);
		**out = Clamp(**out, -1.0, 1.0);
		*out += 1;
	}

	// Bye!
	return 0;
}


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main()
{
	double* cursor = buffer;
	RenderHatOpen(SAMPLING_FREQUENCY, &cursor);

	{
		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
		format.channels = 1;
		format.sampleRate = static_cast<drwav_uint32>(SAMPLING_FREQUENCY);
		format.bitsPerSample = 64;

		drwav wav;
		drwav_init_file_write(&wav, "606-hat-open.wav", &format, nullptr);
		drwav_write_pcm_frames(&wav, static_cast<drwav_uint64>(cursor - buffer), buffer);
		drwav_uninit(&wav);
	}

	return 0;
}
