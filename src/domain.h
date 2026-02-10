#pragma once
#include "geo.h"

#include <string>
#include <vector>

struct Stop {
	std::string name;
	geo::Coordinates coordinates;
};

struct Bus {
	std::string name;
	std::vector<const Stop*> route;
	bool is_roundtrip;
};