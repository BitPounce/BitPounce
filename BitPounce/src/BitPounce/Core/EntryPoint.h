#pragma once
#include "Logger.h"

#include "Application.h"

int main()
{
	BitPounce::Logger::Initialize();

	BitPounce::Application* app = CreateApp();
	int erorrCode = app->Run();
	delete app;

	return erorrCode;
}