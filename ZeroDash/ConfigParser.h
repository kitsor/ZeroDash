#pragma once

#include "common.h"

using namespace std;

class ConfigParser {
public:
	static Config Parse(const string filename);
};