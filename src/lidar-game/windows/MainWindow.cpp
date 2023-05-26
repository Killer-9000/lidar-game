#include "MainWindow.h"

bool MainWindow::Render()
{
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport();

	if (ImGui::Begin("main"))
	{
	}
	ImGui::End();

	EndRender();

	return true;
}
