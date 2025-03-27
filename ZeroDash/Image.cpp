#include "Image.h"

Image::Image(uint16_t width, uint16_t height) {
	imgWidth = width;
	imgHeight = height;

	image = gdImageCreateTrueColor(width, height);
}

int Image::GetWidth() const {
	return imgWidth;
}

int Image::GetHeight() const {
	return imgHeight;
}

int Image::GetColor(int r, int g, int b) {
	return gdImageColorAllocate(image, r, g, b);
}

void Image::FillRectangle(int x, int y, int width, int height, int color) {
	int x2 = x + width - 1;
	x2 = std::min(imgWidth - 1, std::max(0, x2));
	int y2 = y + height - 1;
	y2 = std::min(imgHeight - 1, std::max(0, y2));

	gdImageFilledRectangle(image, x, y, x2, y2, color);
}

std::vector<uint8_t> Image::GetImageBytes() {
	std::vector<uint8_t> pixels(imgWidth * imgHeight * 3);

	int pixelIndex = 0;
	for (int y = 0; y < imgHeight; y++) {
		for (int x = 0; x < imgWidth; x++) {
			int color = gdImageGetPixel(image, x, y);

			pixels[pixelIndex + 0] = static_cast<uint8_t>(gdImageRed(image, color));
			pixels[pixelIndex + 1] = static_cast<uint8_t>(gdImageGreen(image, color));
			pixels[pixelIndex + 2] = static_cast<uint8_t>(gdImageBlue(image, color));
			pixelIndex += 3;
		}
	}

	return pixels;
}

void Image::WriteText(const char* text, int x, int y, int size,
	int color, int* brect) {
	gdImageStringFT(image, brect, color, fontPath, size, 0.0, x, y, text);
}

void Image::Destroy() {
	gdImageDestroy(image);
}