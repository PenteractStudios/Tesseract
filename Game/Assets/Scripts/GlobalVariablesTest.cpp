#include "GlobalVariablesTest.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(GlobalVariablesTest) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    // MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
    // MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(GlobalVariablesTest);

void GlobalVariablesTest::Start() {
}

void GlobalVariablesTest::Update() {
    int globalVarTest = GameplaySystems::GetGlobalVariable<int>("GlobalVarTest", 0);
    globalVarTest += 1;
    std::string logMSG = "Test: " + std::to_string(globalVarTest);
    Debug::Log(logMSG.c_str());
    GameplaySystems::SetGlobalVariable<int>("GlobalVarTest", globalVarTest);
}