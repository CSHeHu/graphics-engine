#include <iostream>
#include "Application.h"

int main()
{
	Application app;

	if (!app.init())
	{
		std::cout << "Failed to initialize application" << std::endl;
		return -1;
	}

	app.run();

	return 0;
}
