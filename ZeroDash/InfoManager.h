#pragma once

#include "Display.h"
#include "Image.h"
#include "StatsProvider.h"

class InfoManager {
public:
	InfoManager(const Config& config);

	void Run();

private:
	Display display;
	Image image;
	StatsProvider statsProvider;

	int dateBgColor;
	int dateFgColor;
	int timeColor;
	int blackColor;

	int color1, color2, color3, color4, color5;

	void DrawDateTime();
	void DrawSystemStats();
	void DrawPiStats();
};