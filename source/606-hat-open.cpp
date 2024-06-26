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
	auto envelope_short = AdEnvelope(0.0, 500.0, sampling_frequency);

	auto oscillator_1 = SquareOscillator(245.0 * SemitoneDetune(-0.6), sampling_frequency); // -0.6, 0
	auto oscillator_2 = SquareOscillator(415.0 * SemitoneDetune(-0.6), sampling_frequency);
	auto oscillator_3 = SquareOscillator(306.0 * SemitoneDetune(-0.6), sampling_frequency);
	auto oscillator_4 = SquareOscillator(437.0 * SemitoneDetune(-0.6), sampling_frequency);
	auto oscillator_5 = SquareOscillator(365.0 * SemitoneDetune(-0.6), sampling_frequency);
	auto oscillator_6 = SquareOscillator(619.0 * SemitoneDetune(-0.6), sampling_frequency);

	const double d = SemitoneDetune(-0.25); // -0.5, 0
	auto o1 = Oscillator(7740.0 * d, 7740.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o2 = Oscillator(7500.0 * d, 7500.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o3 = Oscillator(7250.0 * d, 7250.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o4 = Oscillator(6150.0 * d, 6150.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);

	auto o5 = Oscillator(3300.0 * d, 3300.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o6 = Oscillator(4800.0 * d, 4800.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o7 = Oscillator(5500.0 * d, 5500.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o8 = Oscillator(8400.0 * d, 8400.0 * d, 0.0, 0.0, 1500.0, sampling_frequency);

	auto delay = Delay(2.0, sampling_frequency);

	// Peculiar bandpass (12 lp and 24 hp, components)
	auto bp_a = TwoPolesFilter<FilterType::Lowpass>(7000.0 * SemitoneDetune(-0.6), 0.5, sampling_frequency);
	auto bp_b = TwoPolesFilter<FilterType::Highpass>(7000.0 * SemitoneDetune(-0.6), 0.7, sampling_frequency);
	auto bp_c = TwoPolesFilter<FilterType::Highpass>(7000.0 * SemitoneDetune(-0.6), 0.7, sampling_frequency);

	auto hp = TwoPolesFilter<FilterType::Highpass>(6000.0, 0.7, sampling_frequency);

	const double short_gain = 0.3 * 0.8;
	const double long_gain = 0.51 * 0.8;
	const double clink_gain = 1.8 * 0.8;

	// Render
	for (int x = 0; x < Max(envelope_long.GetTotalSamples(), envelope_short.GetTotalSamples()); x += 1)
	{
		const double e_l = envelope_long.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 2.5); });

		const double e_s = envelope_short.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 10.0); });

		// Metallic signal
		double signal = oscillator_1.Step() + oscillator_2.Step() + oscillator_3.Step() //
		                + oscillator_4.Step() + oscillator_5.Step() + oscillator_6.Step();
		signal /= 6.0;

		// Bandpass
		signal = bp_b.Step(bp_a.Step(signal));
		signal = bp_c.Step(signal);
		signal = Clamp(signal * 6.0, -1.0, 1.0); // Normalize and clip it

		// Long tsss
		double l;
		{
			// Decorrelate from short tsss
			l = delay.Step(-signal);

			// Distortion
			l = ExponentialEasing(signal, -12.0); // -4, -12
		}

		// Short tsss
		double s;
		{
			// Distortion
			s = ExponentialEasing(signal, -18.0);
		}

		// Clink
		double clink;
		{
			const auto easing = [](double x) { return x; };
			clink = ((o1.Step(easing) + o2.Step(easing) + o3.Step(easing) + o4.Step(easing)) * 0.08 + //
			         (o5.Step(easing) + o6.Step(easing) + o7.Step(easing) + o8.Step(easing)) * 0.02);
		}

		// Mix
		double mix = hp.Step((s * e_s * short_gain) + (l * e_l * long_gain) + (clink * (e_l + e_s) * clink_gain));
		mix = Clamp(mix, -1.0, 1.0);

		**out = mix;
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
