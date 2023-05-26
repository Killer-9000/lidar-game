#pragma once

#include "graphics/IWindow.h"

class MainWindow : public IWindow
{
public:
	MainWindow()
		: IWindow("Lidar Game", 720, 480)
	{ }

	virtual bool Render() override;

private:
};
