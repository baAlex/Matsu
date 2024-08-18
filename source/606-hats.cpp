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
	double long_length;
	double short_length;
	double long_gain;
	double short_gain;

	double distortion;
	double distortion_symmetry;
	double noise_gain;

	static Settings DefaultHatOpen()
	{
		return {
		    1500.0, // Long length
		    500.0,  // Short length
		    1.0,    // Long gain
		    0.8,    // Short gain

		    6.0,   // Distortion
		    0.125, // Distortion symmetry
		    0.02   // Noise gain
		};
	}

	static Settings DefaultHatClosed()
	{
		return {
		    0.0,   // Long length
		    150.0, // Short length
		    0.0,   // Long gain
		    1.0,   // Short gain

		    6.0,   // Distortion
		    0.125, // Distortion symmetry
		    0.02   // Noise gain
		};
	}
};


class Squares
{
  public:
	Squares(double sampling_frequency)
	    : m_osc1(619.0 * 1.38, sampling_frequency), //
	      m_osc2(437.0 * 1.12, sampling_frequency), //
	      m_osc3(415.0 * 1.67, sampling_frequency), //
	      m_osc4(365.0 * 1.16, sampling_frequency), //
	      m_osc5(306.0 * 1.28, sampling_frequency), //
	      m_osc6(245.0 * 1.43, sampling_frequency)
	{
	}

	double Step()
	{
		const double signal = m_osc1.Step() + m_osc2.Step() + m_osc3.Step() //
		                      + m_osc4.Step() + m_osc5.Step() + m_osc6.Step();
		return signal / 6.0;
	}

  private:
	SquareOscillator m_osc1;
	SquareOscillator m_osc2;
	SquareOscillator m_osc3;
	SquareOscillator m_osc4;
	SquareOscillator m_osc5;
	SquareOscillator m_osc6;
};


class SharedBandpass
{
  public:
	SharedBandpass(double sampling_frequency)
	    : m_filter1(6900.0, 3.3, sampling_frequency), //
	      m_filter2(7800.0, sampling_frequency),      //
	      m_filter3(7950.0, sampling_frequency),      //
	      m_filter4(10000.0, sampling_frequency)
	{
	}

	double Step(double signal)
	{
		signal = m_filter1.Step(signal);
		signal = m_filter2.Step(signal);
		signal = m_filter3.Step(signal);
		signal = m_filter4.Step(signal);
		return signal;
	}

  private:
	TwoPolesFilter<FilterType::Highpass> m_filter1;
	OnePoleFilter<FilterType::Lowpass> m_filter2;
	OnePoleFilter<FilterType::Lowpass> m_filter3;
	OnePoleFilter<FilterType::Lowpass> m_filter4;
};


static size_t RenderHat(Settings settings, double sampling_frequency, double* output)
{
	auto envelope_long = //
	    AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), settings.long_length, 0.01, 2.5, sampling_frequency);
	auto envelope_short = //
	    AdEnvelope(SamplesToMilliseconds(10, sampling_frequency), settings.short_length, 0.01, 9.0, sampling_frequency);

	auto noise = NoiseGenerator();
	auto squares = Squares(sampling_frequency);
	auto bandpass = SharedBandpass(sampling_frequency);

	auto hp = TwoPolesFilter<FilterType::Highpass>(8400.0, 0.75, sampling_frequency);
	auto lp = TwoPolesFilter<FilterType::Lowpass>(14000.0, 0.25, sampling_frequency);
	const double lp_wet = 0.75; // Original one seems to die lovely at 22 MHz, it can
	                            // be better filters, a nyquist sampling thing, or both.
	                            // This wet/dry mix can get us somewhat there. A proper
	                            // solution should be a weird '1.5 Poles Filter'.

	const int samples = Max(envelope_long.GetTotalSamples(), envelope_short.GetTotalSamples());

	// Render metallic noise
	double max_level = 0.0;
	for (int i = 0; i < samples; i += 1)
	{
		// Six square oscillators
		double signal = squares.Step();

		// Band pass them
		signal = bandpass.Step(signal);

		// Done!
		output[i] = signal;
		max_level = Max(abs(signal), max_level);
	}

	// Normalize, as distortion depends on volume
	for (int i = 0; i < samples; i += 1)
		output[i] = output[i] * (1.0 / max_level);

	// Distort and envelope it
	max_level = 0.0;
	for (int i = 0; i < samples; i += 1)
	{
		const double e_long = envelope_long.Step();
		const double e_short = envelope_short.Step();
		double signal = output[i];

		// Distort metallic noise, highpass to fix asymmetry
		signal = Distortion(signal, -settings.distortion, settings.distortion_symmetry);
		signal = hp.Step(signal);

		// Apply envelopes
		signal = signal * (e_long * settings.long_gain + e_short * settings.short_gain);

		// Add transient noise, also enveloped
		signal = signal + noise.Step() * (settings.noise_gain * e_short * settings.short_gain);

		// Lowpass filter, otherwise it will sound too digital
		signal = Mix(signal, lp.Step(signal), lp_wet);

		// Done!
		output[i] = signal;
		max_level = Max(abs(signal), max_level);
	}

	// Normalize one last time
	for (int i = 0; i < samples; i += 1)
	{
		output[i] = output[i] * (1.0 / max_level);
		output[i] = Clamp(output[i], -1.0, 1.0);
	}

	// Bye!
	return static_cast<size_t>(samples);
}


struct SettingsCymbal
{
	double long_length;
	double short_length;
	double companion_length;
	double long_gain;
	double short_gain;
	double companion_gain;

	double distortion;
	double distortion_symmetry;
	double noise_gain;

	static SettingsCymbal Default()
	{
		return {
		    1000.0, // Long length
		    200.0,  // Short length
		    1600.0, // Companion length
		    0.17,   // Long gain
		    0.8,    // Short gain
		    0.17,   // Companion gain

		    6.0,   // Distortion
		    0.125, // Distortion symmetry
		    0.02   // Noise gain
		};
	}
};


static size_t RenderCymbal(SettingsCymbal settings, double sampling_frequency, double* auxiliary, double* output)
{
	auto envelope_long = //
	    AdEnvelope(SamplesToMilliseconds(80, sampling_frequency), settings.long_length, 0.01, 2.5, sampling_frequency);
	auto envelope_short = //
	    AdEnvelope(SamplesToMilliseconds(80, sampling_frequency), settings.short_length, 0.01, 9.0, sampling_frequency);
	auto envelope_companion = //
	    AdEnvelope(SamplesToMilliseconds(80, sampling_frequency), settings.companion_length, 0.01, 2.5,
	               sampling_frequency);

	auto noise = NoiseGenerator();
	auto squares = Squares(sampling_frequency);
	auto bandpass = SharedBandpass(sampling_frequency);

	auto bp1 = TwoPolesFilter<FilterType::Lowpass>(3500.0, 4.0, sampling_frequency);
	auto bp2 = TwoPolesFilter<FilterType::Highpass>(800.0, 0.75, sampling_frequency);
	auto bp3 = TwoPolesFilter<FilterType::Lowpass>(3500.0, 4.0, sampling_frequency);

	auto hp = TwoPolesFilter<FilterType::Highpass>(8400.0, 0.75, sampling_frequency);
	auto lp = TwoPolesFilter<FilterType::Lowpass>(14000.0, 0.25, sampling_frequency);
	const double lp_wet = 0.75; // Original one seems to die lovely at 22 MHz, it can
	                            // be better filters, a nyquist sampling thing, or both.
	                            // This wet/dry mix can get us somewhat there. A proper
	                            // solution should be a weird '1.5 Poles Filter'.

	const int samples = Max(Max(envelope_long.GetTotalSamples(), envelope_short.GetTotalSamples()),
	                        envelope_companion.GetTotalSamples());

	// Render metallic noise
	double out_max_level = 0.0;
	double aux_max_level = 0.0;
	for (int i = 0; i < samples; i += 1)
	{
		// Six square oscillators
		double signal = squares.Step();

		// Divide signal, band pass them
		double out_signal = bandpass.Step(signal);
		double aux_signal = bp2.Step(bp1.Step(signal));

		// Done!
		output[i] = out_signal;
		out_max_level = Max(abs(out_signal), out_max_level);

		auxiliary[i] = aux_signal;
		aux_max_level = Max(abs(aux_signal), aux_max_level);
	}

	// Normalize, as distortion depends on volume
	for (int i = 0; i < samples; i += 1)
	{
		output[i] = output[i] * (1.0 / out_max_level);
		auxiliary[i] = auxiliary[i] * (1.0 / aux_max_level);
	}

	// Distort and envelope it
	out_max_level = 0.0;
	for (int i = 0; i < samples; i += 1)
	{
		const double e_long = envelope_long.Step();
		const double e_long2 = envelope_companion.Step();
		const double e_short = envelope_short.Step();

		double main_signal = output[i];
		double companion_signal = auxiliary[i];

		// Distort metallic noise, highpass to fix asymmetry
		main_signal = Distortion(main_signal, -settings.distortion, settings.distortion_symmetry);
		main_signal = hp.Step(main_signal);

		companion_signal = Distortion(companion_signal, -settings.distortion, settings.distortion_symmetry);
		companion_signal = bp3.Step(companion_signal); // Not quite a highpass here

		// Apply envelopes
		main_signal = main_signal * (e_long * settings.long_gain + e_short * settings.short_gain);
		companion_signal = companion_signal * (e_long2 * settings.companion_gain);

		double signal = Mix(main_signal, companion_signal, 0.04);

		// Add noise, also enveloped
		signal = signal + noise.Step() * (settings.noise_gain * e_long * settings.long_gain);

		// Lowpass filter, otherwise it will sound too digital
		signal = Mix(signal, lp.Step(signal), lp_wet);

		// Done!
		output[i] = signal;
		out_max_level = Max(abs(signal), out_max_level);
	}

	// Normalize one last time
	for (int i = 0; i < samples; i += 1)
	{
		output[i] = output[i] * (1.0 / out_max_level);
		output[i] = Clamp(output[i], -1.0, 1.0);
	}

	// Bye!
	return static_cast<size_t>(samples);
}


static constexpr double SAMPLING_FREQUENCY = 44100.0;
static double render_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];
static double auxiliary_buffer[static_cast<size_t>(SAMPLING_FREQUENCY) * 2];

int main(int argc, const char* argv[])
{
	(void)argc;
	(void)argv;

	size_t render_length = RenderHat(Settings::DefaultHatOpen(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-open.wav");
	// ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-open-64.wav");

	render_length = RenderHat(Settings::DefaultHatClosed(), SAMPLING_FREQUENCY, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-closed.wav");
	// ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-hat-closed-64.wav");

	render_length = RenderCymbal(SettingsCymbal::Default(), SAMPLING_FREQUENCY, auxiliary_buffer, render_buffer);
	ExportAudioS24(render_buffer, SAMPLING_FREQUENCY, render_length, "606-cymbal.wav");
	// ExportAudioF64(render_buffer, SAMPLING_FREQUENCY, render_length, "606-cymbal-64.wav");

	return 0;
}
