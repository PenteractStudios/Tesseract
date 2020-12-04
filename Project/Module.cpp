#include "Module.h"

#include "Globals.h"

#include "Leaks.h"

Module::~Module() {}

bool Module::Init()
{
	return true;
}

bool Module::Start()
{
	return true;
}

UpdateStatus Module::PreUpdate()
{
	return UpdateStatus::CONTINUE;
}

UpdateStatus Module::Update()
{
	return UpdateStatus::CONTINUE;
}

UpdateStatus Module::PostUpdate()
{
	return UpdateStatus::CONTINUE;
}

bool Module::CleanUp()
{
	return true;
}
