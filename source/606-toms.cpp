/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsuri.hpp"


using namespace matsuri;


class Tom606
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

		double click_gain;

		static Settings DefaultTomHigh()
		{
			return {
			    280.0, // Oscillator length
			    1.0,   // Oscillator gain
			    200.0, // Oscillator frequency
			    0.2,   // Oscillator feedback
			    0.95,  // Oscillator sweep (200 * 0.95 = 190)

			    60.0, // Noise length
			    0.16, // Noise gain

			    0.65 // Click gain
			};
		}

		static Settings DefaultTomLow()
		{
			return {
			    430.0, // Oscillator length
			    1.0,   // Oscillator gain
			    180.0, // Oscillator frequency
			    0.15,  // Oscillator feedback
			    0.65,  // Oscillator sweep (180 * 0.65 = 117)

			    60.0, // Noise length
			    0.16, // Noise gain

			    1.0 // Click gain
			};
		}
	};

	static size_t Render(Settings settings, double sampling_frequency, double* output)
	{
		auto envelope_oscillator = AdEnvelope(sampling_frequency, 0.0, settings.osc_length, 0.0, 8.0);
		auto envelope_noise = AdEnvelope(sampling_frequency, 0.0, settings.noise_length, 0.0, 4.0);

		auto oscillator = Oscillator(sampling_frequency, settings.osc_frequency, settings.osc_feedback,
		                             settings.osc_length, settings.osc_sweep);
		auto noise = NoiseGenerator();

		auto lp = OnePoleFilter(sampling_frequency, FilterType::Lowpass, 700.0);

		const size_t click_attack = static_cast<size_t>(10.0 * (sampling_frequency / 44100.0)); // TODO
		const size_t click_decay = static_cast<size_t>(40.0 * (sampling_frequency / 44100.0));  // Ditto
		const size_t click_length = click_attack + click_decay;

		const size_t samples =
		    click_length + Max(envelope_oscillator.GetTotalSamples(), envelope_noise.GetTotalSamples());

		// Render click
		double max_level = 0.0;
		for (size_t i = 0; i < click_length; i += 1)
		{
			const double e1 = 0.5;
			const double e2 = -3.0;

			double signal;

			if (i < click_attack)
			{
				signal = static_cast<double>(i) / static_cast<double>(click_attack);
				signal = pow(sin(signal * 0.5 * M_PI), e1);
			}
			else
			{
				signal = 1.0 - (static_cast<double>(i - click_attack) / static_cast<double>(click_decay));
				signal = ((pow(2.0, e2 * signal) - 1.0) / (pow(2.0, e2) - 1.0)) * (1.0 - signal) +
				         sin(signal * 0.5 * M_PI) * signal;
			}

			signal = -signal * settings.click_gain;

			output[i] = signal;
			max_level = Max(abs(signal), max_level);
		}

		// Render body
		for (size_t i = click_length; i < samples; i += 1)
		{
			const double e_o = envelope_oscillator.Step();
			const double e_n = envelope_noise.Step();

			// Oscillator plus lowpass'ed noise
			const double o = oscillator.Step();
			const double n = lp.Step(noise.Step());

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

	size_t render_length = Tom606::Render(Tom606::Settings::DefaultTomHigh(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-tom-high.wav");

	render_length = Tom606::Render(Tom606::Settings::DefaultTomLow(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-tom-low.wav");

	return 0;
}
