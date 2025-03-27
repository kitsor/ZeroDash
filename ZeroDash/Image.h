#pragma once
#include <cstdint> // uint16_t
#include <vector>
#include <gd.h>

class Image {
public:
	const char* fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

	Image(uint16_t width, uint16_t height);
	int GetWidth() const;
	int GetHeight() const;
	int GetColor(int r, int g, int b);

	void FillRectangle(int x, int y, int width, int height, int color);
	void WriteText(const char* text, int x, int y, int size,
		int color, int* brect = nullptr);

	std::vector<uint8_t> GetImageBytes();

	void Destroy();

private:
	uint16_t imgWidth, imgHeight;
	gdImagePtr image;
};