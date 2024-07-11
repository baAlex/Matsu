/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "matsu.hpp"
#include "resources.hpp"


static size_t DrawCharacter(const Character* ch, const uint16_t* data, size_t colour, size_t x, size_t y, size_t stride,
                            uint8_t* framebuffer)
{
	framebuffer = framebuffer + (y + ch->y) * stride + x;

	for (size_t row = 0; row < static_cast<size_t>(ch->height); row += 1)
	{
		uint16_t acc = data[ch->data_index + row];

		for (size_t col = 0; col < static_cast<size_t>(ch->width); col += 1)
		{
			framebuffer[col] = ((acc & 0x01) != 0) ? colour : framebuffer[col];
			acc >>= 1;
		}

		framebuffer += stride;
	}

	return ch->width;
}

static void DrawText(const Font* font, const char* text, size_t colour, size_t x, size_t y, size_t stride,
                     uint8_t* framebuffer)
{
	for (const char* c = text; *c != '\0'; c += 1)
	{
		const auto index = static_cast<size_t>(*c);
		if (index >= font->characters_length)
			continue;

		x += DrawCharacter(font->characters + index, font->data, colour, x, y, stride, framebuffer);
	}
}


static void DrawSpectrumWindow(const float* data, size_t data_length, size_t x, size_t y, size_t stride,
                               uint8_t* framebuffer)
{
	framebuffer = framebuffer + y * stride + x;

	for (size_t i = 0; i < data_length / 2; i += 1)
	{
		assert(data[i * 2] <= 1.0f);
		framebuffer[i] = static_cast<uint8_t>(ExponentialEasing(Clamp(data[i * 2], 0.0f, 1.0f), -32.0f) * 255.0f);
	}
}


int main(int argc, const char* argv[])
{
	const size_t width = 640; // 90s style
	const size_t height = 480;
	const size_t colours = 256;
	const auto font = Font95::ToGenericFont();

	const size_t window_length = 1024; // Power of two
	const size_t overlaps_no = 8;      // Same

	bool wav_initialized = false;
	drwav wav;
	PFFFT_Setup* pffft = nullptr;

	uint8_t* framebuffer = nullptr;
	float* window_a = nullptr; // For input
	float* window_b = nullptr; // For output
	float* window_c = nullptr; // As work area
	Colour* palette = nullptr;

	// Hello world
	{
		printf("Silly Analyser v0.1\n");

		if (argc == 1)
		{
			printf(" - Usage: bla... bla... bla...\n");
			return 1;
		}
	}

	// Load audio
	{
		printf(" - Opening \"%s\"...\n", argv[1]);

		if (drwav_init_file(&wav, argv[1], nullptr) == DRWAV_FALSE)
		{
			fprintf(stderr, "DrWav error, init_file().\n");
			goto return_failure;
		}

		wav_initialized = true;

		printf("    - Frequency: %u Hz\n", wav.sampleRate);

		// clang-format off
		switch (wav.fmt.formatTag)
		{
		case DR_WAVE_FORMAT_PCM:        printf("    - Format: PCM, %u bits\n", wav.bitsPerSample); break;
		case DR_WAVE_FORMAT_ADPCM:      printf("    - Format: ADPCM, %u bits\n", wav.bitsPerSample); break;
		case DR_WAVE_FORMAT_IEEE_FLOAT: printf("    - Format: FLOAT, %u bits\n", wav.bitsPerSample); break;
		default:                        printf("    - Format: %u bits\n", wav.bitsPerSample);
		}
		// clang-format on

		printf("    - Channels: %u\n", wav.channels);
	}

	// Initialize Pffft
	// - Aside from simpler, real transform uses less memory
	if ((pffft = pffft_new_setup(static_cast<int>(window_length), PFFFT_REAL)) == nullptr)
	{
		fprintf(stderr, "Pffft error, new_setup().\n");
		goto return_failure;
	}

	// Allocate things
	{
		if ((framebuffer = reinterpret_cast<uint8_t*>(malloc(sizeof(uint8_t) * width * height))) == nullptr)
		{
			fprintf(stderr, "No enough memory (at framebuffer allocation).\n");
			goto return_failure;
		}

		if ((window_a = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * window_length))) == nullptr ||
		    (window_b = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * window_length))) == nullptr ||
		    (window_c = reinterpret_cast<float*>(pffft_aligned_malloc(sizeof(float) * window_length))) == nullptr)
		{
			fprintf(stderr, "No enough memory (at audio block allocation).\n");
			goto return_failure;
		}

		if ((palette = reinterpret_cast<Colour*>(malloc(sizeof(Colour) * colours))) == nullptr)
		{
			fprintf(stderr, "No enough memory (at palette allocation).\n");
			goto return_failure;
		}

		memset(framebuffer, 255, sizeof(uint8_t) * width * height);

		for (size_t i = 0; i < colours; i += 1)
		{
			palette[i].r = static_cast<uint8_t>(i);
			palette[i].g = static_cast<uint8_t>(i);
			palette[i].b = static_cast<uint8_t>(i);
			palette[i].a = 255;
		}
	}

	// Analyse
	{
		size_t read_frames = 0;
		size_t analysed_windows = 0;

		printf(" - Analysing...\n");
		printf("    - Window length: %zu samples\n", window_length);
		printf("    - Overlaps: %zu\n", overlaps_no);

		DrawText(&font, "Testing, testing, one, two, three; Yay!", 0, 10, 10, width, framebuffer);

		memset(window_a, 0, sizeof(float) * window_length);
		const size_t to_read_length = window_length / overlaps_no;

		double phase = 0.0; // Developers, developers, developers

		while (1)
		{
			// Read audio
			const drwav_uint64 read = drwav_read_pcm_frames_f32(&wav, static_cast<drwav_uint64>(to_read_length),
			                                                    window_a + window_length - to_read_length);

			read_frames += static_cast<drwav_uint64>(read);

			if (read != to_read_length)
				memset(window_a + window_length - to_read_length + read, 0, sizeof(float) * (to_read_length - read));

			// Developers, developers, developers
			// - Feed a synthetic signal
			if (0)
			{
				const double bin = 44100.0 / static_cast<double>(to_read_length);
				const double delta = ((bin * 1.0) / 44100.0) * M_PI_TWO; // Second bin
				// const double delta = ((bin * 2.0) / 44100.0) * M_PI_TWO; // Third bin

				for (size_t i = 0; i < to_read_length; i += 1)
				{
					window_a[window_length - to_read_length + i] = static_cast<float>(sin(phase));
					phase += delta;
				}
			}

			// Apply window function
			for (size_t i = 0; i < window_length; i += 1)
			{
				// Hann window (aka: a cosine/sine easing)
				const float w =
				    0.5f * (1.0f - cosf((M_PI_TWO * static_cast<float>(i)) / static_cast<float>(window_length)));

				window_b[i] = window_a[i] * w;
			}

			// Fourier it
			// - 'ordered' = interleaved output
			pffft_transform_ordered(pffft, window_b, window_b, window_c, PFFFT_FORWARD);
			analysed_windows += 1;

			// Developers, developers, developers
			if (0)
			{
				printf("[");
				for (size_t i = 0; i < window_length; i += 2)
				{
					if ((i % 40) == 0)
						printf("\n\t");

					// const float magnitude = sqrtf(powf(window_b[i + 0], 2.0f) + powf(window_b[i +
					// 1], 2.0f));

					// printf("%.2f,\t", window_a[i + 0]);

					// printf("%.2f,\t", window_b[i + 0]);
					printf("%.2f,\t", fabsf(window_b[i + 1]));
					// printf("%.02f,\t", magnitude);
					// printf("%.02f,\t", magnitude / static_cast<float>(window_length / 2)); // Normalized
				}
				printf("\n],\n");
			}

			// Draw
			for (size_t i = 0; i < window_length / 2; i += 1)
			{
				const float magnitude = sqrtf(powf(window_b[i * 2 + 0], 2.0f) + powf(window_b[i * 2 + 1], 2.0f));
				window_b[i * 2] = magnitude / static_cast<float>(window_length / 2);
				// window_b[i * 2] = fabsf(window_b[i*2]) / static_cast<float>(window_length / 2);
			}

			if (analysed_windows < 400)
				DrawSpectrumWindow(window_b, window_length, 10, 50 + analysed_windows, width, framebuffer);

			// Next step?
			if (read != static_cast<drwav_uint64>(to_read_length))
				break;

			// '''Scroll''' window
			for (size_t i = 0; i < (window_length - to_read_length); i += 1)
				window_a[i] = window_a[i + to_read_length];
		}

		printf("    - Read %zu frames\n", read_frames);
		printf("    - Analysed %zu windows\n", analysed_windows);
	}

	// Save to file
	if (argc > 2)
	{
		printf(" - Saving \"%s\"...\n", argv[2]);
		if (ExportImagePalette(framebuffer, palette, width, height, colours, argv[2]) != 0)
			goto return_failure;
	}
	else
	{
		printf(" - No file saved\n");
	}

	// Bye!
	free(palette);
	pffft_aligned_free(window_c);
	pffft_aligned_free(window_b);
	pffft_aligned_free(window_a);
	free(framebuffer);
	pffft_destroy_setup(pffft);
	drwav_uninit(&wav);

	printf(" - Bye!\n");
	return 0;

return_failure:
	if (palette != nullptr)
		free(palette);
	if (window_c != nullptr)
		pffft_aligned_free(window_c);
	if (window_b != nullptr)
		pffft_aligned_free(window_b);
	if (window_a != nullptr)
		pffft_aligned_free(window_a);
	if (framebuffer != nullptr)
		free(framebuffer);
	if (pffft != nullptr)
		pffft_destroy_setup(pffft);
	if (wav_initialized == true)
		drwav_uninit(&wav);
	return 1;
}
