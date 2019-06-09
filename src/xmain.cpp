#include "CrossWindow/CrossWindow.h"

#include <iostream>
#include <chrono>
#include "Logger.h"
#include "Renderer.h"

void performUpdate(Renderer& renderer, float fDelta);

ScenePtr createScene(ResourceManager* resourceManager)
{
	auto teapotMesh = resourceManager->loadModel(L"teapot.obj");

	ScenePtr scene(new Scene());
	auto teapot = scene->createObject(teapotMesh);
	teapot->setPosition(0.0f, -0.3f, 2.5f);
	teapot->setScale(0.3f);

	return scene;
}

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
		std::cout << "Failed to create window. Exiting..." << std::endl;
		return;
	}

	// Create renderer and scene based on window
	xwin::WindowDesc desc = window.getDesc();
	Renderer renderer(window.getDelegate().hwnd, desc.width, desc.height);
	renderer.setScene(createScene(renderer.getResourceManager()));

	// store for frame counting and limiting fps
	auto previousTime = std::chrono::high_resolution_clock::now();
	auto maxFps = 60.0f;
	
	bool isRunning = true;
	while (isRunning)
	{
		InputManager* input = renderer.getInputManager();

		bool shouldRender = true;
		eventQueue.update();

		// Handle window events
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
				xwin::ResizeData data = event.data.resize;
				renderer.resize(data.width, data.height);
				shouldRender = false;
			}
			else if (event.type == xwin::EventType::Focus)
			{
				xwin::FocusData data = event.data.focus;
				if (!data.focused)
				{
					std::cout << "Window unfocused" << std::endl;
					renderer.getInputManager()->notifyLostFocus();
				}
			}
			else if (event.type == xwin::EventType::Keyboard)
			{
				xwin::KeyboardData data = event.data.keyboard;
				input->notifyKeyStateChange(data.key, data.state);
			}
			else if (event.type == xwin::EventType::MouseInput)
			{
				xwin::MouseInputData data = event.data.mouseInput;
				input->notifyMouseButtonChange(data.button, data.state);
			}
			else if (event.type == xwin::EventType::MouseRaw)
			{
				xwin::MouseRawData data = event.data.mouseRaw;
				input->notifyMouseRawInput(data.deltax, data.deltay);
			}

			eventQueue.pop();
		}

		// find out how much time has passed since the last render
		auto newTime = std::chrono::high_resolution_clock::now();
		float elapsed = std::chrono::duration<float>(newTime - previousTime).count();
		float minElapsed = 1.0f / maxFps;
		if (elapsed < minElapsed)
		{
			continue; // wait some more
		}

		previousTime = newTime;

		// perform update step
		performUpdate(renderer, elapsed);

		// Render view
		if (shouldRender)
		{
			renderer.render();
		}

		input->notifyRenderFinished();
	}
}

// camera pitch and yaw as euler angles stored here
// because quaternions to euler conversions have errors.
// using a stored euler value and figuring it out from there is easier.
float cameraPitch;
float cameraYaw;

void performUpdate(Renderer& renderer, float fDelta)
{
	auto camera = renderer.getCamera();
	auto input = renderer.getInputManager();

	float cameraVelocity = 5.0f;
	float cameraSensitivity = 8.0f;

	// determine the direction we want to move the camera
	glm::vec3 cameraMove(0.0f, 0.0f, 0.0f);
	if (input->isDown(xwin::Key::Q))
		cameraMove += camera->getUp();
	if (input->isDown(xwin::Key::E))
		cameraMove += camera->getUp() * -1.0f;
	if (input->isDown(xwin::Key::W))
		cameraMove += camera->getForward();
	if (input->isDown(xwin::Key::A))
		cameraMove += camera->getRight() * -1.0f;
	if (input->isDown(xwin::Key::S))
		cameraMove += camera->getForward() * -1.0f;
	if (input->isDown(xwin::Key::D))
		cameraMove += camera->getRight();

	// If we're moving the camera, scale it to the velocity
	if (cameraMove != glm::vec3(0, 0, 0))
	{
		float moveDistance = cameraVelocity * fDelta;

		glm::vec3 moveBy = glm::normalize(cameraMove) * moveDistance;
		camera->move(moveBy);
	}

	// Rotate the camera if we need to
	if (input->isDown(xwin::MouseInput::Right))
	{
		auto camera = renderer.getCamera();

		float sensitivity = cameraSensitivity * fDelta;
		float deltaX = input->getAxis(InputAxis::MOUSE_X) * sensitivity;
		float deltaY = input->getAxis(InputAxis::MOUSE_Y) * sensitivity;

		cameraYaw += deltaX;
		cameraPitch = glm::clamp(cameraPitch + deltaY, -90.0f, 90.0f);

		// wrap left/right to 0-360
		if (cameraYaw > 360)
		{
			cameraYaw = glm::mod(cameraYaw, 360.0f);
		}
		else if (cameraYaw < 0)
		{
			cameraYaw = cameraYaw + 360.0f * (1 + cameraYaw / -360.0f);
		}

		camera->setRotation(cameraPitch, cameraYaw, 0);
	}

	// Rotate the object if the left mouse button is being held down
	if (input->isDown(xwin::MouseInput::Left))
	{
		auto scene = renderer.getScene();
		
		auto object = *scene->begin();

		float sensitivity = 15.0f * fDelta;
		float deltaX = input->getAxis(InputAxis::MOUSE_X) * sensitivity;
		float deltaY = input->getAxis(InputAxis::MOUSE_Y) * sensitivity;

		if (deltaX)
		{
			object->addRotation(0, -deltaX, 0);
		}
		if (deltaY)
		{
			object->rotateAround(camera->getRight(), -deltaY);
		}
	}
}