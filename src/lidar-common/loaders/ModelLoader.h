#pragma once

#include <string>

struct Model;
class WorldView;

namespace ModelLoader
{
	bool Load(WorldView* worldView, Model* model, const std::string& filename);
};