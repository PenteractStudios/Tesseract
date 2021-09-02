#include "ModifyTiling.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(ModifyTiling) {
    // Add members here to expose them to the engine. Example:
    MEMBER(MemberType::BOOL, isTiling),
    MEMBER(MemberType::BOOL, isOffset),
    MEMBER(MemberType::FLOAT, multiplierX),
    MEMBER(MemberType::FLOAT, multiplierY),
    // MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(ModifyTiling);

void ModifyTiling::Start() {

}

void ModifyTiling::Update() {
    if (isTiling) {
        tiling += float2(0.5f, 0.5f)* Time::GetDeltaTime();
        GetOwner().GetComponent<ComponentMeshRenderer>()->SetTextureTiling(tiling);
    }

    if (isOffset) {
        offset += float2(multiplierX, multiplierY) * Time::GetDeltaTime();
        GetOwner().GetComponent<ComponentMeshRenderer>()->SetTextureOffset(offset);
    }
}
