#include "ui/settings.hpp"
#include "web/server.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/Touchable.hpp"

#include "UnityEngine/GameObject.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

namespace BannerUI{
    void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* settings = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        BeatSaberUI::CreateText(settings->get_transform(), "Custom Banners Settings");
        
        HMUI::ModalView* linkModal = BeatSaberUI::CreateModal(settings->get_transform(), UnityEngine::Vector2(120.0f, 80.0f), [](HMUI::ModalView* modal){}, false);
        VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(linkModal->get_transform());

        BeatSaberUI::CreateText(layout->get_transform(), "To Link Your Account Go To: https://bs.phazed.xyz/acc");
        auto statusText = BeatSaberUI::CreateText(layout->get_transform(), "Loading...");

        BeatSaberUI::CreateUIButton(layout->get_transform(), "Cancel",
            [linkModal]() {
                linkModal->Hide(true, nullptr);
                WEB::Close();
            });

        BeatSaberUI::CreateUIButton(settings->get_transform(), "Link Account To Web",
            [linkModal, statusText]() {
                linkModal->Show(true, true, nullptr);
                WEB::Init(statusText);
            });
    }
}