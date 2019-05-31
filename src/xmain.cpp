#include "CrossWindow/CrossWindow.h"

#include <iostream>
#include "Renderer.h"

void xmain(int argc, const char** argv)
{
	// todo: consider global error handling but....the problem is that then visual studio doesn't report where the exception came from
	// consider Debug > Windows > Exceptions Settings > Break When Thrown > All C++ Exceptions but it might also report those we are not interested in

	xwin::WindowDesc windowDesc;
	windowDesc.name = "Test";
	windowDesc.title = "My Title";
	windowDesc.visible = true;
	windowDesc.width = 1280;
	windowDesc.height = 720;

	xwin::Window window;
	xwin::EventQueue eventQueue;

	if (!window.create(windowDesc, eventQueue))
	{
		return;
	}

	xwin::WindowDesc desc = window.getDesc();
	Renderer renderer(window.getDelegate().hwnd, desc.width, desc.height);

	auto resourceManager = renderer.getResourceManager();
	auto teapotMesh = resourceManager->loadModel(L"teapot.obj");

	ScenePtr scene(new Scene());
	auto teapot = scene->createObject(teapotMesh);
	teapot->setWorldPosition(0.0f, -0.3f, 2.5f);
	teapot->setScale(0.3f);

	renderer.setScene(scene);

	// Handle window events
	bool isRunning = true;
	while (true)
	{
		bool shouldRender = true;
		eventQueue.update();

		while (!eventQueue.empty())
		{
			const xwin::Event& event = eventQueue.front();

			if (event.type == xwin::EventType::Close)
			{
				window.close();
				isRunning = false;
			}
			else if (event.type == xwin::EventType::Resize)
			{
				// resize renderer
				auto data = event.data.resize;
				renderer.resize(data.width, data.height);
				shouldRender = false;
			}

			eventQueue.pop();

			// Render view
			if (shouldRender)
			{
				renderer.render();
			}
		}
	}
}
