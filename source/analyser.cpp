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

#include "thirdparty/cargs/include/cargs.h"
#include "thirdparty/lodepng/lodepng.h"

static const char* NAME = "Matsu analyser";
static const int VERSION_MAX = 0;
static const int VERSION_MIN = 1;


static int ExportIndexedImage(const Palette* palette, const uint8_t* data, size_t width, size_t height,
                              const char* filename)
{
	LodePNGState png;
	unsigned char* encoded_blob = nullptr;
	FILE* fp = nullptr;

	if (palette->length > (UINT8_MAX + 1) || width > UINT_MAX || height > UINT_MAX)
		return 1;

	// Initialize Png encoder
	lodepng_state_init(&png);               // Doesn't fail
	lodepng_color_mode_init(&png.info_raw); // Ditto

	png.info_raw.colortype = LCT_PALETTE;
	png.info_raw.bitdepth = 8;

	for (size_t i = 0; i < palette->length; i += 1)
	{
		if (lodepng_palette_add(&png.info_raw,         //
		                        palette->colours[i].r, //
		                        palette->colours[i].g, //
		                        palette->colours[i].b, //
		                        palette->colours[i].a) != 0)
		{
			fprintf(stderr, "LodePng error, palette_add().\n");
			goto return_failure;
		}
	}

	// Encode
	size_t encoded_blob_size;
	{
		const unsigned ret = lodepng_encode(&encoded_blob, &encoded_blob_size, data, static_cast<unsigned>(width),
		                                    static_cast<unsigned>(height), &png);

		if (ret != 0)
		{
			fprintf(stderr, "LodePng error, encode().\n");
			fprintf(stderr, "\"%s\".\n", lodepng_error_text(ret));
			goto return_failure;
		}
	}

	// Write to file
	if ((fp = fopen(filename, "wb")) == nullptr)
	{
		fprintf(stderr, "File output error (at opening a file).\n");
		goto return_failure;
	}

	if (fwrite(encoded_blob, sizeof(uint8_t), encoded_blob_size, fp) != encoded_blob_size)
	{
		fprintf(stderr, "File output error (at writing).\n");
		goto return_failure;
	}

	// Bye!
	fclose(fp);
	free(encoded_blob);
	lodepng_state_cleanup(&png);
	return 0;

return_failure:
	if (fp != nullptr)
		fclose(fp);
	if (encoded_blob != nullptr)
		free(encoded_blob);
	lodepng_state_cleanup(&png);
	return 1;
}


struct Framebuffer
{
	size_t width;
	size_t height;
	size_t stride;
	uint8_t buffer[];
};


static Framebuffer* FramebufferCreate(size_t width, size_t height)
{
	auto* framebuffer = reinterpret_cast<Framebuffer*>(malloc(sizeof(Framebuffer) + sizeof(uint8_t) * width * height));
	if (framebuffer == nullptr)
		return nullptr;

	framebuffer->width = width;
	framebuffer->height = height;
	framebuffer->stride = width;
	memset(framebuffer->buffer, 0, sizeof(uint8_t) * width * height);

	return framebuffer;
}

static void FramebufferDelete(Framebuffer* framebuffer)
{
	free(framebuffer);
}


enum class TextStyle
{
	Normal,
	Bold
};

template <TextStyle STYLE>
static void DrawCharacterInternal(size_t ch_width, size_t ch_height, size_t ch_data_index, uint8_t colour_index,
                                  const uint16_t* font_data, size_t stride, uint8_t* out)
{
	uint16_t acc;

	for (size_t row = 0; row < ch_height; row += 1)
	{
		uint16_t acc = font_data[ch_data_index + row];

		for (size_t col = 0; col < ch_width; col += 1)
		{
			if (STYLE == TextStyle::Normal)
				out[col + 0] = ((acc & 0x01) != 0) ? colour_index : out[col + 0];
			else
			{
				out[col + 0] = ((acc & 0x01) != 0) ? colour_index : out[col + 0];
				out[col + 1] = ((acc & 0x01) != 0) ? colour_index : out[col + 1];
			}

			acc >>= 1;
		}

		out += stride;
	}
}

static size_t DrawCharacter(TextStyle style, const Character* ch, const uint16_t* font_data, uint8_t colour_index,
                            size_t x, size_t y, Framebuffer* framebuffer)
{
	uint8_t* out = framebuffer->buffer + (y + ch->y) * framebuffer->stride + x;
	size_t ch_height = static_cast<size_t>(ch->height);
	size_t ch_width = static_cast<size_t>(ch->width);

	const size_t bold = (style == TextStyle::Bold) ? 1 : 0; // To account for at width calculations

	if (x + (ch_width + bold) > framebuffer->width)
		ch_width = (x < framebuffer->width) ? ((framebuffer->width - x) - bold) : 0;
	if (y + ch_height > framebuffer->height)
		ch_height = (y < framebuffer->height) ? (framebuffer->height - y) : 0;

	switch (style)
	{
	case TextStyle::Bold:
		DrawCharacterInternal<TextStyle::Bold>(ch_width, ch_height, ch->data_index, colour_index, font_data,
		                                       framebuffer->stride, out);
		break;
	default:
		DrawCharacterInternal<TextStyle::Normal>(ch_width, ch_height, ch->data_index, colour_index, font_data,
		                                         framebuffer->stride, out);
	}

	return ch_width;
}

static size_t DrawText(const Font* font, TextStyle style, const char* text, uint8_t colour_index, size_t x, size_t y,
                       Framebuffer* framebuffer)
{
	for (const char* c = text; *c != '\0'; c += 1)
	{
		const auto character_index = static_cast<size_t>(*c);
		if (character_index >= font->characters_length)
			continue;

		if (*c == ' ')
		{
			x += font->space_width;
			continue;
		}
		else if (*c == '\t')
		{
			x += font->tab_width;
			continue;
		}

		x += DrawCharacter(style, font->characters + character_index, font->data, colour_index, x, y, framebuffer);
		if (style == TextStyle::Bold)
			x += 1;
	}

	return x;
}


static void DrawSpectrumLine(const float* data, size_t data_length, uint8_t colour_index_min, uint8_t colour_index_max,
                             float exposure, float linearity, size_t x, size_t y, size_t width,
                             Framebuffer* framebuffer)
{
	uint8_t* out = framebuffer->buffer + y * framebuffer->stride + x;

	size_t draw_width = width;
	if (x + draw_width > framebuffer->width)
		draw_width = (x < framebuffer->width) ? (framebuffer->width - x) : 0;

	exposure = -powf(2.0f, exposure);

	for (size_t col = 0; col < draw_width; col += 1)
	{
		// Nearest pick a data sample (divided by two because nyquist)
		size_t data_x;
		{
			// Plain linear with integers
			// data_x = (col * data_length) / (width * 2);

			// Fancy non linear axis
			const float data_xf = powf(static_cast<float>(col) / static_cast<float>(width), linearity);
			data_x = static_cast<size_t>((data_xf * static_cast<float>(data_length)) / 2.0f);
		}

		// Map sample to colours index
		const float index_mul = static_cast<float>(colour_index_max) - static_cast<float>(colour_index_min) + 1.0f;
		const float colour = ExponentialEasing(data[data_x], exposure) * index_mul;

		// Draw!
		out[col] = colour_index_min + Min(static_cast<uint8_t>(floorf(colour)), colour_index_max);
	}
}


struct Settings
{
	const char* input;
	const char* input2;
	const char* output;

	size_t window_length;
	float linearity;
	float scale;
	float exposure;

	int frequency;
	size_t analysed_windows;
	float difference;
};


static void DrawChrome(const Settings* s, const Font* font, const Palette* palette, Framebuffer* framebuffer)
{
	constexpr size_t BUFFER_LENGTH = 256; // May overflow on Windows

	const size_t padding_x = 10;
	const size_t padding_y = 10;

	const size_t text_colour = palette->length - 1;
	const size_t text_colour2 = palette->length / 2 + 1;

	char buffer[BUFFER_LENGTH];

	// Title
	size_t title_len;
	snprintf(buffer, BUFFER_LENGTH, "%s v%i.%i", NAME, VERSION_MAX, VERSION_MIN);
	title_len = DrawText(font, TextStyle::Bold, buffer, text_colour, padding_x, padding_y, framebuffer);

	const char* tool_name = (s->input2 == nullptr) ? "Spectrum plot tool" : "Difference tool";
	title_len = Max(title_len, DrawText(font, TextStyle::Normal, tool_name, text_colour, padding_x,
	                                    padding_y + font->line_height, framebuffer));

	// Information
	if (s->input2 == nullptr)
	{
		snprintf(buffer, BUFFER_LENGTH,
		         "\t|\tInput: \"%s\", %i Hz\t|\tWindow length: %zu, Linearity: %.2f, Scale: %.2fx, Exposure: % "
		         ".2fx\t|\tAnalysed %zu windows",
		         s->input, s->frequency, s->window_length, s->linearity, s->scale, s->exposure, s->analysed_windows);
		DrawText(font, TextStyle::Normal, buffer, text_colour, title_len, padding_y + font->line_height / 2,
		         framebuffer);
	}
	else
	{
		snprintf(buffer, BUFFER_LENGTH,
		         "\t|\tInputs: \"%s\", \"%s\", %i Hz\t|\tWindow length: %zu, Linearity: %.2f, Scale: %.2fx, Exposure: "
		         "% .2fx\t|\tAnalysed %zu windows, Difference: %.2f",
		         s->input, s->input2, s->frequency, s->window_length, s->linearity, s->scale, s->exposure,
		         s->analysed_windows, s->difference);
		DrawText(font, TextStyle::Normal, buffer, text_colour, title_len, padding_y + font->line_height / 2,
		         framebuffer);
	}

	// Ruler
	for (size_t i = 0; i < 4; i += 1)
	{
		const float x = static_cast<float>(i) / static_cast<float>(4);
		const float xp = powf(x, 1.0f / s->linearity);

		const float label_frequency = (x * static_cast<float>(s->frequency)) / (1000.0f * 2.0f);
		const size_t label_x = static_cast<size_t>(xp * static_cast<float>(framebuffer->width));
		const size_t label_y = padding_y + font->line_height * 3 - font->line_height / 2;

		snprintf(buffer, BUFFER_LENGTH, "| %0.1f MHz", label_frequency);
		DrawText(font, TextStyle::Normal, buffer, text_colour2, label_x, label_y, framebuffer);
	}

	{
		snprintf(buffer, BUFFER_LENGTH, "%0.1f MHz |", static_cast<float>(s->frequency) / (1000.0f * 2.0f));

		// const size_t text_length = TextLength(font, TextStyle::Normal, buffer);
		const size_t text_length = 51; // TODO

		DrawText(font, TextStyle::Normal, buffer, text_colour2, framebuffer->width - text_length,
		         padding_y + font->line_height * 3 - font->line_height / 2, framebuffer);
	}
}


static struct cag_option s_cvars[] = {
    {.identifier = 'i',
     .access_letters = "i",
     .access_name = "input",
     .value_name = "FILENAME",
     .description = "File to read"},

    {.identifier = 'd',
     .access_letters = "d",
     .access_name = "difference",
     .value_name = "FILENAME",
     .description = "File to read, and calculate difference with"},

    {.identifier = 'o',
     .access_letters = "o",
     .access_name = "output",
     .value_name = "FILENAME",
     .description = "File to write, optional"},

    {.identifier = 'w',
     .access_letters = "w",
     .access_name = "window",
     .value_name = "NUMBER",
     .description = "Window length (512, 1024, 2048 or 4096, default: 2048)"},

    {.identifier = 'l',
     .access_letters = "l",
     .access_name = "linearity",
     .value_name = "NUMBER",
     .description = "X axis linearity (1: linear, >1: exponential, default: 2)"},

    {.identifier = 's',
     .access_letters = "s",
     .access_name = "scale",
     .value_name = "NUMBER",
     .description = "Y axis scale (default: 1)"},

    {.identifier = 'e',
     .access_letters = "e",
     .access_name = "exposure",
     .value_name = "NUMBER",
     .description = "Exposure (default: 8)"},

    {.identifier = 'h', .access_letters = "h", .access_name = "help", .description = "Shows the command help"}};


static int LoadAudio(const char* filename, bool* out_initialzed, drwav* out_wav)
{
	printf(" - Opening \"%s\"...\n", filename);

	if (drwav_init_file(out_wav, filename, nullptr) == DRWAV_FALSE)
	{
		fprintf(stderr, "DrWav error, init_file().\n");
		return 1;
	}

	*out_initialzed = true;

	printf("    - Frequency: %u Hz\n", out_wav->sampleRate);

	// clang-format off
		switch (out_wav->fmt.formatTag)
		{
		case DR_WAVE_FORMAT_PCM:        printf("    - Format: PCM, %u bits\n", out_wav->bitsPerSample); break;
		case DR_WAVE_FORMAT_ADPCM:      printf("    - Format: ADPCM, %u bits\n", out_wav->bitsPerSample); break;
		case DR_WAVE_FORMAT_IEEE_FLOAT: printf("    - Format: FLOAT, %u bits\n", out_wav->bitsPerSample); break;
		default:                        printf("    - Format: %u bits\n", out_wav->bitsPerSample);
		}
	// clang-format on

	printf("    - Channels: %u\n", out_wav->channels);

	return 0;
}


int main(int argc, char* argv[])
{
	const Font font = Font95::ToGenericFont();
	Palette palette;

	Settings s;

	drwav wav;
	bool wav_initialized = false;
	drwav wav2;
	bool wav2_initialized = false;

	Framebuffer* framebuffer = nullptr;
	constexpr size_t FRAMEBUFFER_WIDTH = 1024; // 90s style
	constexpr size_t FRAMEBUFFER_HEIGHT = 768;

	// Read settings
	{
		printf("%s v%u.%u\n", NAME, VERSION_MAX, VERSION_MIN);

		cag_option_context cag;
		cag_option_init(&cag, s_cvars, CAG_ARRAY_SIZE(s_cvars), argc, argv);

		s.input = nullptr;
		s.input2 = nullptr;
		s.output = nullptr;
		s.window_length = 2048;
		s.linearity = 2.0f;
		s.scale = 1.0f;
		s.exposure = 8.0f;

		while (cag_option_fetch(&cag))
		{
			switch (cag_option_get_identifier(&cag))
			{
			case 'i': s.input = cag_option_get_value(&cag); break;
			case 'd': s.input2 = cag_option_get_value(&cag); break;
			case 'o': s.output = cag_option_get_value(&cag); break;
			case 'w': s.window_length = atol(cag_option_get_value(&cag)); break;
			case 'l': s.linearity = atof(cag_option_get_value(&cag)); break;
			case 's': s.scale = atof(cag_option_get_value(&cag)); break;
			case 'e': s.exposure = atof(cag_option_get_value(&cag)); break;
			case 'h':
				printf("Usage: analyser [OPTION]...\n");
				cag_option_print(s_cvars, CAG_ARRAY_SIZE(s_cvars), stdout);
				return EXIT_SUCCESS;
			case '?':
				cag_option_print_error(&cag, stderr);
				return EXIT_FAILURE;
				break;
			}
		}

		if (s.input == nullptr)
		{
			fprintf(stderr, "No input specified.\n");
			return EXIT_FAILURE;
		}

		if (s.window_length != 512 && s.window_length != 1024 && s.window_length != 2048 && s.window_length != 4096)
		{
			fprintf(stderr, "Invalid window length.\n");
			return EXIT_FAILURE;
		}

		s.linearity = Max(s.linearity, 1.0f);
		s.scale = Clamp(s.scale, 1.0f / 4.0f, 8.0f);
		s.exposure = Max(s.exposure, 1.0f);
	}

	// Load audio
	if (s.input2 == nullptr)
	{
		if (LoadAudio(s.input, &wav_initialized, &wav) != 0)
			goto return_failure;

		palette = Citrink::ToGenericPalette();
	}
	else
	{
		if (LoadAudio(s.input, &wav_initialized, &wav) != 0 || LoadAudio(s.input2, &wav2_initialized, &wav2) != 0)
			goto return_failure;

		palette = Slso8::ToGenericPalette();
	}

	s.frequency = wav.sampleRate;

	// Create framebuffer
	if ((framebuffer = FramebufferCreate(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT)) == nullptr)
	{
		fprintf(stderr, "No enough memory (at framebuffer creation).\n");
		goto return_failure;
	}

	// Analyse
	{
		const auto overlaps_no = static_cast<size_t>(20.0f * s.scale * (static_cast<float>(s.window_length) / 2048.0f));
		Analyser analyser(s.window_length, overlaps_no);

		const auto read_callback = [&](size_t to_read_length, float* out) -> size_t
		{
			return static_cast<size_t>( //
			    drwav_read_pcm_frames_f32(&wav, static_cast<drwav_uint64>(to_read_length), out));
		};

		const auto read_callback2 = [&](size_t to_read_length, float* out) -> size_t
		{
			if (s.input2 == nullptr)
				return 0;

			return static_cast<size_t>( //
			    drwav_read_pcm_frames_f32(&wav2, static_cast<drwav_uint64>(to_read_length), out));
		};

		const auto draw_callback = [&](size_t analysed_windows, size_t window_length, const float* data)
		{
			if (analysed_windows < framebuffer->height - 57) // TODO
				DrawSpectrumLine(data, window_length, 0, static_cast<uint8_t>(palette.length - 1), s.exposure,
				                 s.linearity, 0, 57 + analysed_windows, framebuffer->width, framebuffer);
		};

		printf(" - Analysing...\n");

		const auto analysis = analyser.Analyse(read_callback, read_callback2, draw_callback);
		s.analysed_windows = analysis.windows;
		s.difference = analysis.difference;

		printf("    - Overlaps: %zu\n", overlaps_no);
		printf("    - Analysed %zu windows\n", analysis.windows);
		printf("    - Difference %.4f\n", analysis.difference);
	}

	// Draw chrome
	DrawChrome(&s, &font, &palette, framebuffer);

	// Save to file
	if (s.output != nullptr)
	{
		printf(" - Saving \"%s\"...\n", s.output);
		if (ExportIndexedImage(&palette, framebuffer->buffer, framebuffer->width, framebuffer->height, s.output) != 0)
			goto return_failure;
	}
	else
	{
		printf(" - No file saved\n");
	}

	// Bye!
	drwav_uninit(&wav);
	if (wav2_initialized == true)
		drwav_uninit(&wav2);
	FramebufferDelete(framebuffer);

	printf(" - Bye!\n");
	return EXIT_SUCCESS;

return_failure:
	if (wav_initialized == true)
		drwav_uninit(&wav);
	if (wav2_initialized == true)
		drwav_uninit(&wav2);
	if (framebuffer != nullptr)
		FramebufferDelete(framebuffer);
	return EXIT_FAILURE;
}
