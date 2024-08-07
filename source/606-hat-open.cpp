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
	double short_gain;
	double long_gain;
	double noise_gain;
	double distortion;
	double clink_gain;
};

static size_t RenderHatOpen(Settings settings, double sampling_frequency, double* output)
{
	auto envelope_long = AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), 1500.0, sampling_frequency);
	auto envelope_short = AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), 500.0, sampling_frequency);

	auto oscillator_1 = SquareOscillator(619.0, sampling_frequency);
	auto oscillator_2 = SquareOscillator(437.0, sampling_frequency);
	auto oscillator_3 = SquareOscillator(415.0, sampling_frequency);
	auto oscillator_4 = SquareOscillator(365.0, sampling_frequency);
	auto oscillator_5 = SquareOscillator(306.0, sampling_frequency);
	auto oscillator_6 = SquareOscillator(245.0, sampling_frequency);

	auto o1 = Oscillator(7802.0, 7802.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o6 = Oscillator(6822.0, 6822.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o2 = Oscillator(6149.0, 6149.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o3 = Oscillator(5552.0, 5552.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o4 = Oscillator(4746.0, 4746.0, 0.0, 0.0, 1500.0, sampling_frequency);
	auto o5 = Oscillator(3363.0, 3363.0, 0.0, 0.0, 1500.0, sampling_frequency);

	auto noise = NoiseGenerator();

	// Peculiar bandpass (12db lp and 24db hp, components)
	auto bp_a = TwoPolesFilter<FilterType::Lowpass>(6600.0, 0.25, sampling_frequency); // 6000, 6700
	auto bp_b = TwoPolesFilter<FilterType::Highpass>(6600.0, 1.2, sampling_frequency);
	auto bp_c = TwoPolesFilter<FilterType::Highpass>(6600.0, 1.2, sampling_frequency);

	// These two after envelope
	auto hp = TwoPolesFilter<FilterType::Highpass>(6000.0, 0.5, sampling_frequency);
	auto lp = OnePoleFilter<FilterType::Lowpass>(16000.0, sampling_frequency); // Too digital otherwise

	// Render
	for (int x = 0; x < Max(envelope_long.GetTotalSamples(), envelope_short.GetTotalSamples()); x += 1)
	{
		const double e_l = envelope_long.Get(
		    x,                          //
		    [](double x) { return x; }, //
		    [](double x) { return ExponentialEasing(x, 1.75); });

		const double e_s = envelope_short.Get(
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
			            settings.clink_gain;

			// Bandpass
			metallic = bp_b.Step(bp_a.Step(metallic));
			metallic = bp_c.Step(metallic);
			metallic = Clamp(metallic * 8.0, -1.0, 1.0); // Normalize and clip it

			// Distortion
			metallic = Distortion(metallic, -settings.distortion, 0.0);
		}

		// Mix
		output[x] = lp.Step(hp.Step((metallic * e_l * settings.long_gain) + (metallic * e_s * settings.short_gain)) +
		                    (noise.Step() * e_s * settings.noise_gain));
		output[x] = Clamp(output[x], -1.0, 1.0);
	}

	// Bye!
	return static_cast<size_t>(Max(envelope_long.GetTotalSamples(), envelope_short.GetTotalSamples()));
}


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double render_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main(int argc, const char* argv[])
{
	float* reference_data = nullptr;
	drwav_uint64 reference_data_length;
	size_t render_length = 0;

	Analyser analyser(512, 5);

	// Open file to compare against
	if (argc > 1)
	{
		printf("Opening '%s'...\n", argv[1]);

		unsigned channels;
		unsigned frequency;

		reference_data =
		    drwav_open_file_and_read_pcm_frames_f32(argv[1], &channels, &frequency, &reference_data_length, nullptr);

		assert(reference_data != nullptr);
		assert(frequency == static_cast<unsigned>(SAMPLING_FREQUENCY));
	}

	// Default sensible settings
	Settings s;
	s.short_gain = 0.817;
	s.long_gain = 1.066;
	s.noise_gain = 0.060;
	s.distortion = 5.341;
	s.clink_gain = 0.03;
	uint64_t r = 0x654a6ce66da6697b;

	Settings childs[16];
	float childs_score[16];

	// Iterative tuning
	if (argc > 1)
	{
		for (size_t g = 0; g < 16; g += 1)
		{
			for (size_t ch = 0; ch < 16; ch += 1)
			{
				// Tweak settings
				childs[ch].short_gain = s.short_gain * (RandomFloat(&r) * 0.75 + 1.25);
				childs[ch].long_gain = s.long_gain * (RandomFloat(&r) * 0.75 + 1.25);
				childs[ch].noise_gain = s.noise_gain * (RandomFloat(&r) * 0.75 + 1.25);
				childs[ch].distortion = s.distortion * (RandomFloat(&r) * 0.75 + 1.25);

				childs[ch].noise_gain = Min(childs[ch].noise_gain, 0.06); // Some rules >:(
				childs[ch].distortion = Min(childs[ch].distortion, 8.0);

				// Render
				render_length = RenderHatOpen(childs[ch], SAMPLING_FREQUENCY, render_buffer);

				// Compare
				float* input_a = reference_data;
				double* input_b = render_buffer;

				const auto input_callback1 = [&](size_t to_read_length, float* out) -> size_t
				{
					const auto length =
					    Min(to_read_length, static_cast<size_t>((reference_data + reference_data_length) - input_a));
					memcpy(out, input_a, sizeof(float) * length);
					input_a += length;
					return length;
				};

				const auto input_callback2 = [&](size_t to_read_length, float* out) -> size_t
				{
					const auto length =
					    Min(to_read_length, static_cast<size_t>((render_buffer + render_length) - input_b));
					for (size_t i = 0; i < length; i += 1)
						out[i] = static_cast<float>(input_b[i]);
					input_b += length;
					return length;
				};

				const auto draw_callback = [&](size_t analysed_windows, size_t window_length, const float* data)
				{
					(void)analysed_windows;
					(void)window_length;
					(void)data;
					return;
				};

				auto analysis = analyser.Analyse(input_callback1, input_callback2, draw_callback);
				childs_score[ch] = analysis.difference;

				// Some feedback
				printf("Child %zu, Difference %.4f\t[%.3f, %.3f, %.3f, %.3f]\n", ch, analysis.difference,
				       childs[ch].short_gain, childs[ch].long_gain, childs[ch].noise_gain, childs[ch].distortion);
			}

			// Choose best childs
			size_t first_best = 0;
			for (size_t i = 0; i < 16; i += 1)
			{
				if (childs_score[i] < childs_score[first_best])
					first_best = i;
			}

			size_t second_best = (first_best + 1) % 16;
			for (size_t i = 0; i < 16; i += 1)
			{
				if (i == first_best)
					continue;
				if (childs_score[i] < childs_score[second_best])
					second_best = i;
			}

			printf("Generation %zu best: %zu, %zu\n", g, first_best, second_best);

			// Mix
			s.short_gain = exp((log(childs[first_best].short_gain) + log(childs[second_best].short_gain)) / 2.0);
			s.long_gain = exp((log(childs[first_best].long_gain) + log(childs[second_best].long_gain)) / 2.0);
			s.noise_gain = exp((log(childs[first_best].noise_gain) + log(childs[second_best].noise_gain)) / 2.0);
			s.distortion = exp((log(childs[first_best].distortion) + log(childs[second_best].distortion)) / 2.0);

			printf("Mix [%.3f, %.3f, %.3f, %.3f], seed: 0x%zx\n", s.short_gain, s.long_gain, s.noise_gain, s.distortion,
			       r);
		}

		// Final render
		render_length = RenderHatOpen(s, SAMPLING_FREQUENCY, render_buffer);
	}

	// Just render
	else
	{
		render_length = RenderHatOpen(s, SAMPLING_FREQUENCY, render_buffer);
	}

	// Save audio
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-open.wav");
	ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-open-64.wav");

	// Bye!
	drwav_free(reference_data, NULL);
	return 0;
}
