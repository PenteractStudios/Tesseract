#include "Factory.h"

#include "Creator.h"

#include <unordered_map>

struct FactoryContext {
	std::unordered_map<std::string, Creator*> table;
};

static FactoryContext* factoryContext = nullptr;

void Factory::CreateContext() {
	factoryContext = new FactoryContext();
}

void Factory::DestroyContext() {
	RELEASE(factoryContext);
}

Script* Factory::Create(const std::string& className, GameObject* owner) {
	auto it = factoryContext->table.find(className);
	if (it != factoryContext->table.end()) {
		return it->second->Create(owner);
	} else {
		return (Script*) nullptr;
	}
}

void Factory::RegisterScript(const std::string& className, Creator* creator) {
	factoryContext->table[className] = creator;
}