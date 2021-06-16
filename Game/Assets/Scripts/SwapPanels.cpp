#include "SwapPanels.h"

#include "GameplaySystems.h"

EXPOSE_MEMBERS(SwapPanels) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    // MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
    MEMBER(MemberType::GAME_OBJECT_UID, targetUID),
        MEMBER(MemberType::GAME_OBJECT_UID, currentUID)
};

GENERATE_BODY_IMPL(SwapPanels);

void SwapPanels::Start() {
    
    target = GameplaySystems::GetGameObject(targetUID);
    current = GameplaySystems::GetGameObject(currentUID);
}

void SwapPanels::Update() {

}

void SwapPanels::OnButtonClick()
{
    if (target != nullptr && current != nullptr) {
        target->Enable();
        current->Disable();
    }
}
