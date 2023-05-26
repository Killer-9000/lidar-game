#pragma once

#include <string>

class WorldView;

namespace SceneLoader
{
	bool Load(WorldView* worldView, const std::string& filename);
	bool Save(WorldView* worldView, const std::string& filename);
};