/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"


using namespace matsu;


class Snare606
{
  public:
	struct Settings
	{
		double osc_length;
		double osc_gain;
		double osc_frequency;
		double osc_feedback;
		double osc_sweep;

		double noise_length;
		double noise_gain;

		static Settings Default()
		{
			return {
			    150.0, // Oscillator length
			    0.7,   // Oscillator gain
			    320.0, // Oscillator frequency
			    0.0,   // Oscillator feedback
			    0.60,  // Oscillator sweep (320 * 0.60 = 192)

			    150.0, // Noise length
			    0.9    // Noise gain
			};
		}
	};

	static size_t Render(Settings settings, double sampling_frequency, double* output)
	{
		auto envelope_oscillator = AdEnvelope(sampling_frequency, 2.0, settings.osc_length - 2.0, 0.0, 8.0);
		auto envelope_noise = AdEnvelope(sampling_frequency, 2.0, settings.noise_length - 2.0, 0.0, 9.0);

		auto oscillator = Oscillator(sampling_frequency, settings.osc_frequency, settings.osc_feedback,
		                             settings.osc_length, settings.osc_sweep);
		auto noise = NoiseGenerator();

		auto bp_a = TwoPolesFilter(sampling_frequency, FilterType::Highpass, 2700.0, 0.75);
		auto bp_b = TwoPolesFilter(sampling_frequency, FilterType::Lowpass, 16000.0, 0.5);
		auto bp_c = OnePoleFilter(sampling_frequency, FilterType::Lowpass, 2700.0);

		const size_t samples = Max(envelope_oscillator.GetTotalSamples(), envelope_noise.GetTotalSamples());

		// Render
		double max_level = 0.0;
		for (size_t i = 0; i < samples; i += 1)
		{
			const double e_o = envelope_oscillator.Step();
			const double e_n = envelope_noise.Step();

			// Oscillator plus bandpass'ed noise
			const double o = oscillator.Step();
			const double n = bp_c.Step(bp_b.Step(bp_a.Step(noise.Step())));

			const double signal = (o * e_o * settings.osc_gain) + (n * e_n * settings.noise_gain);

			// Done!
			output[i] = signal;
			max_level = Max(abs(signal), max_level);
		}

		// Normalize
		for (size_t i = 0; i < samples; i += 1)
		{
			output[i] = output[i] * (1.0 / max_level);
			output[i] = Clamp(output[i], -1.0, 1.0);
		}

		// Bye!
		return samples;
	}
};


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double render_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main(int argc, const char* argv[])
{
	(void)argc;
	(void)argv;

	size_t render_length = Snare606::Render(Snare606::Settings::Default(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-snare.wav");

	return 0;
}
