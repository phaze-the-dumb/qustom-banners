#include "ui/settings.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/Touchable.hpp"
#include "HMUI/ImageView.hpp"

#include "TMPro/TextAlignmentOptions.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "UnityEngine/GameObject.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

namespace BannerUI{
    void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
        if(!firstActivation)return;

        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* settings = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        auto headerText = BeatSaberUI::CreateText(settings->get_transform(), "Custom Banners Settings\n", Vector2::get_zero());
        headerText->set_fontSize(5);
        headerText->set_alignment(TMPro::TextAlignmentOptions::Center);

        auto layout = BeatSaberUI::CreateVerticalLayoutGroup(settings->get_transform());
        layout->set_spacing(6);

        LayoutElement* layoutElement = layout->GetComponent<LayoutElement*>();
        layoutElement->set_preferredWidth(120);
        layoutElement->set_preferredHeight(65);

        // // Account Linking
        // auto linkAccText = BeatSaberUI::CreateText(layout->get_transform(), "To Link Your Account Go To: https://bs.phazed.xyz/acc", Vector2::get_zero());
        // auto codeText = BeatSaberUI::CreateText(layout->get_transform(), "123 456", Vector2::get_zero());

        // linkAccText->set_fontSize(3);
        // linkAccText->set_alignment(TMPro::TextAlignmentOptions::Center);

        // codeText->set_fontSize(10);
        // codeText->set_alignment(TMPro::TextAlignmentOptions::Center);

        // Banners
        
    }
}