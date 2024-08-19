/*

Copyright (c) 2024 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include <stddef.h>
#include <stdint.h>

struct Character
{
	uint16_t data_index;
	uint8_t width : 4;
	uint8_t height : 4;
	uint8_t y : 4;
};

struct Font
{
	size_t characters_length;
	size_t data_length;
	const Character* characters;
	const uint16_t* data;

	size_t line_height;
	size_t space_width;
	size_t tab_width;
};

struct Colour
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct Palette
{
	size_t length;
	const Colour* colours;
};


// ========


class Font95
{
  public:
	static constexpr size_t CHARACTERS_LENGTH = 128;
	static constexpr size_t DATA_LENGTH = 732;

	static constexpr size_t LINE_HEIGHT = 14;
	static constexpr size_t SPACE_WIDTH = 4;
	static constexpr size_t TAB_WIDTH = 16;

	static constexpr Character CHARACTERS[CHARACTERS_LENGTH] = {
	    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},     {0, 0, 0, 0},
	    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},     {0, 0, 0, 0},
	    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},     {0, 0, 0, 0},
	    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},     {0, 0, 0, 0},
	    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},     {0, 0, 0, 0},
	    {0, 0, 0, 0},   {0, 0, 0, 0},    {0, 0, 0, 0},   {0, 2, 9, 1},    {9, 4, 3, 1},     {12, 7, 9, 1},
	    {21, 6, 10, 0}, {31, 8, 9, 1},   {40, 6, 9, 1},  {49, 2, 3, 1},   {52, 3, 11, 0},   {63, 3, 11, 0},
	    {74, 4, 3, 1},  {77, 6, 5, 4},   {82, 3, 2, 9},  {84, 3, 1, 6},   {85, 2, 1, 9},    {86, 5, 8, 2},
	    {94, 6, 9, 1},  {103, 4, 9, 1},  {112, 6, 9, 1}, {121, 6, 9, 1},  {130, 6, 9, 1},   {139, 6, 9, 1},
	    {148, 6, 9, 1}, {157, 6, 9, 1},  {166, 6, 9, 1}, {175, 6, 9, 1},  {184, 2, 6, 4},   {190, 2, 7, 4},
	    {197, 5, 7, 3}, {204, 6, 3, 5},  {207, 5, 7, 3}, {214, 6, 9, 1},  {223, 11, 10, 0}, {233, 8, 9, 1},
	    {242, 6, 9, 1}, {251, 7, 9, 1},  {260, 7, 9, 1}, {269, 6, 9, 1},  {278, 6, 9, 1},   {287, 7, 9, 1},
	    {296, 7, 9, 1}, {305, 2, 9, 1},  {314, 5, 9, 1}, {323, 7, 9, 1},  {332, 6, 9, 1},   {341, 8, 9, 1},
	    {350, 7, 9, 1}, {359, 7, 9, 1},  {368, 7, 9, 1}, {377, 7, 9, 1},  {386, 7, 9, 1},   {395, 6, 9, 1},
	    {404, 6, 9, 1}, {413, 7, 9, 1},  {422, 8, 9, 1}, {431, 12, 9, 1}, {440, 8, 9, 1},   {449, 8, 9, 1},
	    {458, 8, 9, 1}, {467, 3, 12, 0}, {479, 5, 8, 2}, {487, 3, 12, 0}, {499, 6, 3, 0},   {502, 7, 1, 9},
	    {503, 3, 2, 1}, {505, 6, 6, 4},  {511, 6, 9, 1}, {520, 6, 6, 4},  {526, 6, 9, 1},   {535, 6, 6, 4},
	    {541, 3, 9, 1}, {550, 6, 8, 4},  {558, 6, 9, 1}, {567, 2, 9, 1},  {576, 2, 11, 1},  {587, 6, 9, 1},
	    {596, 2, 9, 1}, {605, 8, 6, 4},  {611, 6, 6, 4}, {617, 6, 6, 4},  {623, 6, 8, 4},   {631, 6, 8, 4},
	    {639, 3, 6, 4}, {645, 5, 6, 4},  {651, 3, 8, 2}, {659, 6, 6, 4},  {665, 6, 6, 4},   {671, 8, 6, 4},
	    {677, 5, 6, 4}, {683, 5, 8, 4},  {691, 5, 6, 4}, {697, 4, 12, 0}, {709, 2, 9, 1},   {718, 4, 12, 0},
	    {730, 7, 2, 6}, {0, 0, 0, 0},
	};

	static constexpr uint16_t DATA[DATA_LENGTH] = {
	    /* ! */ 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0000, 0x0002,
	    /* " */ 0x000A, 0x000A, 0x000A,
	    /* # */ 0x0024, 0x0024, 0x007E, 0x0024, 0x0024, 0x0024, 0x007E, 0x0024, 0x0024,
	    /* $ */ 0x0008, 0x001C, 0x002A, 0x000A, 0x000C, 0x0018, 0x0028, 0x002A, 0x001C, 0x0008,
	    /* % */ 0x000C, 0x0092, 0x004C, 0x0020, 0x0010, 0x0008, 0x0064, 0x0092, 0x0060,
	    /* & */ 0x0004, 0x000A, 0x000A, 0x0004, 0x0004, 0x002A, 0x0012, 0x0012, 0x002C,
	    /* ' */ 0x0002, 0x0002, 0x0002,
	    /* ( */ 0x0004, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0004,
	    /* ) */ 0x0002, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0002,
	    /* * */ 0x000A, 0x0004, 0x000A,
	    /* + */ 0x0008, 0x0008, 0x003E, 0x0008, 0x0008,
	    /* , */ 0x0004, 0x0002,
	    /* - */ 0x0006,
	    /* . */ 0x0002,
	    /* / */ 0x0010, 0x0010, 0x0008, 0x0008, 0x0004, 0x0004, 0x0002, 0x0002,
	    /* 0 */ 0x001C, 0x0022, 0x0022, 0x0022, 0x0022, 0x0022, 0x0022, 0x0022, 0x001C,
	    /* 1 */ 0x0008, 0x000E, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
	    /* 2 */ 0x001C, 0x0022, 0x0020, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x003E,
	    /* 3 */ 0x001C, 0x0022, 0x0020, 0x0020, 0x0018, 0x0020, 0x0020, 0x0022, 0x001C,
	    /* 4 */ 0x0010, 0x0018, 0x0018, 0x0014, 0x0014, 0x0012, 0x003E, 0x0010, 0x0010,
	    /* 5 */ 0x003E, 0x0002, 0x0002, 0x001E, 0x0022, 0x0020, 0x0020, 0x0022, 0x001C,
	    /* 6 */ 0x001C, 0x0022, 0x0002, 0x0002, 0x001E, 0x0022, 0x0022, 0x0022, 0x001C,
	    /* 7 */ 0x003E, 0x0020, 0x0010, 0x0010, 0x0008, 0x0008, 0x0004, 0x0004, 0x0004,
	    /* 8 */ 0x001C, 0x0022, 0x0022, 0x0022, 0x001C, 0x0022, 0x0022, 0x0022, 0x001C,
	    /* 9 */ 0x001C, 0x0022, 0x0022, 0x0022, 0x003C, 0x0020, 0x0020, 0x0022, 0x001C,
	    /* : */ 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0002,
	    /* ; */ 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0002, 0x0002,
	    /* < */ 0x0010, 0x0008, 0x0004, 0x0002, 0x0004, 0x0008, 0x0010,
	    /* = */ 0x003E, 0x0000, 0x003E,
	    /* > */ 0x0002, 0x0004, 0x0008, 0x0010, 0x0008, 0x0004, 0x0002,
	    /* ? */ 0x001C, 0x0022, 0x0020, 0x0020, 0x0010, 0x0008, 0x0008, 0x0000, 0x0008,
	    /* @ */ 0x00F0, 0x0108, 0x0204, 0x04E2, 0x0492, 0x0492, 0x0762, 0x0004, 0x0008, 0x01F0,
	    /* A */ 0x0010, 0x0010, 0x0028, 0x0028, 0x0044, 0x0044, 0x007C, 0x0082, 0x0082,
	    /* B */ 0x001E, 0x0022, 0x0022, 0x0022, 0x001E, 0x0022, 0x0022, 0x0022, 0x001E,
	    /* C */ 0x003C, 0x0042, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0042, 0x003C,
	    /* D */ 0x001E, 0x0022, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0022, 0x001E,
	    /* E */ 0x003E, 0x0002, 0x0002, 0x0002, 0x001E, 0x0002, 0x0002, 0x0002, 0x003E,
	    /* F */ 0x003E, 0x0002, 0x0002, 0x0002, 0x001E, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* G */ 0x003C, 0x0042, 0x0002, 0x0002, 0x0072, 0x0042, 0x0042, 0x0062, 0x005C,
	    /* H */ 0x0042, 0x0042, 0x0042, 0x0042, 0x007E, 0x0042, 0x0042, 0x0042, 0x0042,
	    /* I */ 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* J */ 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0012, 0x0012, 0x000C,
	    /* K */ 0x0022, 0x0012, 0x000A, 0x0006, 0x0006, 0x000A, 0x0012, 0x0022, 0x0042,
	    /* L */ 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x003E,
	    /* M */ 0x0082, 0x0082, 0x00C6, 0x00C6, 0x00AA, 0x00AA, 0x0092, 0x0092, 0x0082,
	    /* N */ 0x0042, 0x0046, 0x0046, 0x004A, 0x004A, 0x0052, 0x0062, 0x0062, 0x0042,
	    /* O */ 0x003C, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x003C,
	    /* P */ 0x003E, 0x0042, 0x0042, 0x0042, 0x003E, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* Q */ 0x003C, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0052, 0x0062, 0x003C,
	    /* R */ 0x003E, 0x0042, 0x0042, 0x0042, 0x003E, 0x0042, 0x0042, 0x0042, 0x0042,
	    /* S */ 0x001C, 0x0022, 0x0002, 0x0002, 0x001C, 0x0020, 0x0020, 0x0022, 0x001C,
	    /* T */ 0x003E, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
	    /* U */ 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x003C,
	    /* V */ 0x0082, 0x0082, 0x0044, 0x0044, 0x0044, 0x0028, 0x0028, 0x0010, 0x0010,
	    /* W */ 0x0802, 0x0802, 0x0444, 0x0444, 0x0444, 0x02A8, 0x02A8, 0x0110, 0x0110,
	    /* X */ 0x0082, 0x0082, 0x0044, 0x0028, 0x0010, 0x0028, 0x0044, 0x0082, 0x0082,
	    /* Y */ 0x0082, 0x0082, 0x0044, 0x0028, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	    /* Z */ 0x00FE, 0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x00FE,
	    /* [ */ 0x0006, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0006,
	    /* \ */ 0x0002, 0x0002, 0x0004, 0x0004, 0x0008, 0x0008, 0x0010, 0x0010,
	    /* ] */ 0x0006, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0006,
	    /* ^ */ 0x0008, 0x0014, 0x0022,
	    /* _ */ 0x007E,
	    /* ` */ 0x0002, 0x0004,
	    /* a */ 0x001C, 0x0020, 0x003C, 0x0022, 0x0022, 0x003C,
	    /* b */ 0x0002, 0x0002, 0x0002, 0x001E, 0x0022, 0x0022, 0x0022, 0x0022, 0x001E,
	    /* c */ 0x001C, 0x0022, 0x0002, 0x0002, 0x0022, 0x001C,
	    /* d */ 0x0020, 0x0020, 0x0020, 0x003C, 0x0022, 0x0022, 0x0022, 0x0022, 0x003C,
	    /* e */ 0x001C, 0x0022, 0x003E, 0x0002, 0x0022, 0x001C,
	    /* f */ 0x0004, 0x0002, 0x0002, 0x0006, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* g */ 0x003C, 0x0022, 0x0022, 0x0022, 0x0022, 0x003C, 0x0020, 0x001E,
	    /* h */ 0x0002, 0x0002, 0x0002, 0x001A, 0x0026, 0x0022, 0x0022, 0x0022, 0x0022,
	    /* i */ 0x0002, 0x0000, 0x0000, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* j */ 0x0002, 0x0000, 0x0000, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* k */ 0x0002, 0x0002, 0x0002, 0x0012, 0x000A, 0x0006, 0x000A, 0x0012, 0x0022,
	    /* l */ 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* m */ 0x006E, 0x0092, 0x0092, 0x0092, 0x0092, 0x0092,
	    /* n */ 0x001A, 0x0026, 0x0022, 0x0022, 0x0022, 0x0022,
	    /* o */ 0x001C, 0x0022, 0x0022, 0x0022, 0x0022, 0x001C,
	    /* p */ 0x001E, 0x0022, 0x0022, 0x0022, 0x0022, 0x001E, 0x0002, 0x0002,
	    /* q */ 0x003C, 0x0022, 0x0022, 0x0022, 0x0022, 0x003C, 0x0020, 0x0020,
	    /* r */ 0x0006, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* s */ 0x000C, 0x0012, 0x0004, 0x0008, 0x0012, 0x000C,
	    /* t */ 0x0002, 0x0002, 0x0006, 0x0002, 0x0002, 0x0002, 0x0002, 0x0004,
	    /* u */ 0x0022, 0x0022, 0x0022, 0x0022, 0x0032, 0x002C,
	    /* v */ 0x0022, 0x0022, 0x0014, 0x0014, 0x0008, 0x0008,
	    /* w */ 0x0092, 0x0092, 0x00AA, 0x00AA, 0x0044, 0x0044,
	    /* x */ 0x0012, 0x0012, 0x000C, 0x000C, 0x0012, 0x0012,
	    /* y */ 0x0012, 0x0012, 0x0012, 0x0012, 0x000C, 0x0004, 0x0004, 0x0003,
	    /* z */ 0x001E, 0x0010, 0x0008, 0x0004, 0x0002, 0x001E,
	    /* { */ 0x0008, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0002, 0x0004, 0x0004, 0x0004, 0x0004, 0x0008,
	    /* | */ 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
	    /* } */ 0x0002, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0008, 0x0004, 0x0004, 0x0004, 0x0004, 0x0002,
	    /* ~ */ 0x004C, 0x0032,
	};

	static Font ToGenericFont()
	{
		return {CHARACTERS_LENGTH, DATA_LENGTH, CHARACTERS, DATA, LINE_HEIGHT, SPACE_WIDTH, TAB_WIDTH};
	}
};

constexpr Character Font95::CHARACTERS[Font95::CHARACTERS_LENGTH]; // C++ weird corners (tldr: link problems)
constexpr uint16_t Font95::DATA[Font95::DATA_LENGTH];              // Ditto


class Citrink
{
	// Citrink Palette
	// https://lospec.com/palette-list/citrink
	// by Inkpendude (@inkpendude)

  public:
	static constexpr size_t LENGTH = 8;

	static constexpr Colour COLOURS[LENGTH] = {
	    {0x20, 0x15, 0x33, 0xFF}, {0x25, 0x24, 0x46, 0xFF}, {0x25, 0x4D, 0x70, 0xFF}, {0x16, 0x6E, 0x7A, 0xFF},
	    {0x52, 0xC3, 0x3F, 0xFF}, {0xB2, 0xD9, 0x42, 0xFF}, {0xFC, 0xF6, 0x60, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}};

	static Palette ToGenericPalette()
	{
		return {LENGTH, COLOURS};
	}
};

constexpr Colour Citrink::COLOURS[Citrink::LENGTH];


class Slso8
{
	// SLSO8 Palette
	// https://lospec.com/palette-list/slso8
	// by Luis Miguel Maldonado

  public:
	static constexpr size_t LENGTH = 8;

	static constexpr Colour COLOURS[LENGTH] = {
	    {0x0D, 0x2B, 0x45, 0xFF}, {0x20, 0x3C, 0x56, 0xFF}, {0x54, 0x4E, 0x68, 0xFF}, {0x8D, 0x69, 0x7A, 0xFF},
	    {0xD0, 0x81, 0x59, 0xFF}, {0xFF, 0xAA, 0x5E, 0xFF}, {0xFF, 0xD4, 0xA3, 0xFF}, {0xFF, 0xEC, 0xD6, 0xFF}};

	static Palette ToGenericPalette()
	{
		return {LENGTH, COLOURS};
	}
};

constexpr Colour Slso8::COLOURS[Slso8::LENGTH];


class Sunraze
{
	// Sunraze Palette
	// https://lospec.com/palette-list/sunraze
	// by Dain Kaplan

  public:
	static constexpr size_t LENGTH = 14;

	static constexpr Colour COLOURS[LENGTH] = {
	    {0x27, 0x03, 0x2A, 0xFF}, {0x4B, 0x08, 0x3D, 0xFF}, {0x73, 0x11, 0x44, 0xFF}, {0x89, 0x0C, 0x38, 0xFF},
	    {0xAB, 0x0A, 0x2A, 0xFF}, {0xBE, 0x20, 0x28, 0xFF}, {0xCF, 0x49, 0x2C, 0xFF}, {0xE3, 0x64, 0x33, 0xFF},
	    {0xE3, 0x88, 0x4E, 0xFF}, {0xEC, 0xB5, 0x5F, 0xFF}, {0xEE, 0xD6, 0x7B, 0xFF}, {0xF4, 0xEF, 0xAE, 0xFF},
	    {0xFF, 0xDD, 0xD9, 0xFF}, {0xFB, 0xFB, 0xF2, 0xFF}};

	static Palette ToGenericPalette()
	{
		return {LENGTH, COLOURS};
	}
};

constexpr Colour Sunraze::COLOURS[Sunraze::LENGTH];


#include <limits.h>

#include "matsu.hpp"

#include "thirdparty/cargs/include/cargs.h"
#include "thirdparty/lodepng/lodepng.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"

#include "thirdparty/dr_libs/dr_wav.h"
#pragma clang diagnostic pop

#else
#include "thirdparty/dr_libs/dr_wav.h"
#endif


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
		const float colour = matsu::ExponentialEasing(data[data_x], exposure) * index_mul;

		// Draw!
		out[col] = colour_index_min + matsu::Min(static_cast<uint8_t>(floorf(colour)), colour_index_max);
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
	title_len = matsu::Max(title_len, DrawText(font, TextStyle::Normal, tool_name, text_colour, padding_x,
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

		s.linearity = matsu::Max(s.linearity, 1.0f);
		s.scale = matsu::Clamp(s.scale, 1.0f / 4.0f, 8.0f);
		s.exposure = matsu::Max(s.exposure, 1.0f);
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
		matsu::Analyser analyser(s.window_length, overlaps_no);

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
