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


class Kick606
{
  public:
	struct Settings
	{
		double osc1_length;
		double osc1_gain;
		double osc1_frequency;
		double osc1_feedback;

		double osc2_length;
		double osc2_gain;
		double osc2_frequency;
		double osc2_feedback;

		double click_gain;

		static Settings Default()
		{
			return {
			    300.0, // Oscillator 1 length
			    0.8,   // Oscillator 1 gain
			    60.0,  // Oscillator 1 frequency
			    0.0,   // Oscillator 1 feedback

			    70.0,  // Oscillator 2 length
			    0.4,   // Oscillator 2 gain
			    120.0, // Oscillator 2 frequency
			    0.1,   // Oscillator 2 feedback

			    1.0 // Click gain
			};
		}
	};

	static size_t Render(Settings settings, double sampling_frequency, double* output)
	{
		auto envelope1 = AdEnvelope(sampling_frequency, 0.0, settings.osc1_length, 0.0, 8.0);
		auto envelope2 = AdEnvelope(sampling_frequency, 0.0, settings.osc2_length, 0.0, 8.0);

		auto oscillator1 = Oscillator(sampling_frequency, settings.osc1_frequency, settings.osc1_feedback);
		auto oscillator2 = Oscillator(sampling_frequency, settings.osc2_frequency, settings.osc2_feedback);

		const size_t click_attack = static_cast<size_t>(50.0 * (sampling_frequency / 44100.0)); // TODO
		const size_t click_decay = static_cast<size_t>(64.0 * (sampling_frequency / 44100.0));  // Ditto
		const size_t click_length = click_attack + click_decay;

		const size_t samples = click_length + Max(envelope1.GetTotalSamples(), envelope2.GetTotalSamples());

		// Render click
		double max_level = 0.0;
		for (size_t i = 0; i < click_length; i += 1)
		{
			const double e1 = 0.7;
			const double e2 = 2.35;
			const double e3 = 1.14;
			const double e4 = 3.0;

			double signal;

			if (i < click_attack)
			{
				signal = static_cast<double>(i) / static_cast<double>(click_attack);
				signal = pow(signal, e1);
			}
			else
			{
				signal = 1.0 - (static_cast<double>(i - click_attack) / static_cast<double>(click_decay));
				signal = pow(signal, e3 + (e2 - e3) * pow(signal, e4));
			}

			signal = -signal * settings.click_gain;

			output[i] = signal;
			max_level = Max(abs(signal), max_level);
		}

		// Render body
		for (size_t i = click_length; i < samples; i += 1)
		{
			const double e1 = envelope1.Step();
			const double e2 = envelope2.Step();

			// Two oscillators
			const double o1 = oscillator1.Step();
			const double o2 = oscillator2.Step();

			const double signal = (o1 * e1 * settings.osc1_gain) + (o2 * e2 * settings.osc2_gain);

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

	size_t render_length = Kick606::Render(Kick606::Settings::Default(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-kick.wav");

	return 0;
}
