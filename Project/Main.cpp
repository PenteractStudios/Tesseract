#include "Globals.h"
#include "Application.h"
#include "Logging.h"

#include "SDL.h"
#include <stdlib.h>

#include "Leaks.h"

enum class MainState
{
	CREATION,
	INIT,
	START,
	UPDATE,
	FINISH,
	EXIT
};

Application* App = nullptr;

int main(int argc, char ** argv)
{
#ifdef _DEBUG
	_CrtMemState mem_state;
	_CrtMemCheckpoint(&mem_state);
#endif

	// Initialize logging
	log_string = new std::string();

	// Game loop
	int main_return = EXIT_FAILURE;
	MainState state = MainState::CREATION;
	while (state != MainState::EXIT)
	{
		switch (state)
		{
		case MainState::CREATION:
			LOG("Application Creation --------------");
			App = new Application();
			state = MainState::INIT;
			break;

		case MainState::INIT:
			LOG("Application Init --------------");
			if (App->Init() == false)
			{
				LOG("Application Init exits with error -----");
				state = MainState::EXIT;
			}
			else
			{
				state = MainState::START;
			}
			break;

		case MainState::START:
			LOG("Application Start --------------");
			if (App->Start() == false)
			{
				LOG("Application Start exits with error -----");
				state = MainState::EXIT;
			}
			else
			{
				state = MainState::UPDATE;
				LOG("Application Update --------------");
			}
			break;

		case MainState::UPDATE:
		{
			UpdateStatus update_return = App->Update();

			if (update_return == UpdateStatus::ERROR)
			{
				LOG("Application Update exits with error -----");
				state = MainState::EXIT;
			}

			if (update_return == UpdateStatus::STOP)
			{
				state = MainState::FINISH;
			}
			break;
		}

		case MainState::FINISH:
			LOG("Application CleanUp --------------");
			if (App->CleanUp() == false)
			{
				LOG("Application CleanUp exits with error -----");
			}
			else
			{
				LOG("Application CleanUp completed successfuly -----");
				main_return = EXIT_SUCCESS;
			}
			state = MainState::EXIT;
			break;
		}

	}

	LOG("Bye :)\n");

	RELEASE(App);
	RELEASE(log_string);

#ifdef _DEBUG
	_CrtMemDumpAllObjectsSince(&mem_state);
#endif

	return main_return;
}
