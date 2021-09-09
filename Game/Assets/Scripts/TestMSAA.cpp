#include "TestMSAA.h"

#include "Components/UI/ComponentVideo.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(TestMSAA) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    // MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
    MEMBER(MemberType::GAME_OBJECT_UID, videoID)
};

GENERATE_BODY_IMPL(TestMSAA);

void TestMSAA::Start() {
    video = GameplaySystems::GetGameObject(videoID);
    Debug::Log("video read");
    if (video){
        videoComp = video->GetComponent<ComponentVideo>();
        Debug::Log("componet video read");
        if (videoComp) {
            Debug::Log("componet video play");
            videoComp->Play();
        }
    }
}

void TestMSAA::Update() {
    if (videoComp->HasVideoFinished()) {
        Debug::Log("WOLOLOOOO");
    }
    /*std::string logMSG = "IS ACTIVE " + std::to_string(Screen::IsMSAAActive());
    Debug::Log(logMSG.c_str());

    std::string logMSG2 = "SAMPLING " + std::to_string(static_cast<int>(Screen::GetMSAAType()));
    Debug::Log(logMSG2.c_str());

    std::string logMSG3 = "BLOOM THRESHOLD " + std::to_string(static_cast<int>(Screen::GetBloomThreshold()));
    Debug::Log(logMSG3.c_str());

    if (Input::GetKeyCodeUp(Input::KEYCODE::KEY_1)) {
        Screen::SetMSAAType(MSAA_SAMPLES_TYPE::MSAA_X2);
    }
    if (Input::GetKeyCodeUp(Input::KEYCODE::KEY_2)) {
        Screen::SetMSAAType(MSAA_SAMPLES_TYPE::MSAA_X4);
    }
    if (Input::GetKeyCodeUp(Input::KEYCODE::KEY_3)) {
        Screen::SetMSAAType(MSAA_SAMPLES_TYPE::MSAA_X8);
    }
    if (Input::GetKeyCodeUp(Input::KEYCODE::KEY_Y)) {
        Screen::SetMSAAActive(!Screen::IsMSAAActive());
    }

    if (Input::GetKeyCodeUp(Input::KEYCODE::KEY_O)) {
        Screen::SetBloomThreshold(.5f);
    }

    if (Input::GetKeyCodeUp(Input::KEYCODE::KEY_P)) {
        Screen::SetBloomThreshold(1.0f);
    }*/
}