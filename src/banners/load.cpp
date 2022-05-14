#include "main.hpp"
#include "ModConfig.hpp"

#include "banners/load.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/LayerMask.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "config-utils/shared/config-utils.hpp"

#include "HMUI/ImageView.hpp"

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;
using namespace HMUI;

namespace Banners{
    UnityEngine::GameObject* LeftScreen;
    UnityEngine::GameObject* RightScreen;
    UnityEngine::GameObject* LeftScreenIG;
    UnityEngine::GameObject* RightScreenIG;

    void LoadMenu(){
        LeftScreen = BeatSaberUI::CreateFloatingScreen(
            { 120, 240 },
            {
                ( getModConfig().X.GetValue() * -1 ) - 15.0f,
                getModConfig().Y.GetValue() + 17.0f,
                25.0
            },
            { 0, 0, 0 },
            0.0f,
            false,
            false,
            1
        );

        RightScreen = BeatSaberUI::CreateFloatingScreen(
            { 120, 240 },
            {
                getModConfig().X.GetValue() + 15.0f,
                getModConfig().Y.GetValue() + 17.0f,
                25.0
            },
            { 0, 0, 0 },
            0.0f,
            false,
            false,
            1
        );

        getLogger().info("Created Screens");

        HMUI::ImageView* leftImage = BeatSaberUI::CreateImage(LeftScreen->get_transform(), Banners::GetLeft(), { 0, 0 }, {
            200.0f * getModConfig().Scale.GetValue(), 
            400.0f * getModConfig().Scale.GetValue()
        });

        HMUI::ImageView* rightImage = BeatSaberUI::CreateImage(RightScreen->get_transform(), Banners::GetRight(), { 0, 0 }, {
            200.0f * getModConfig().Scale.GetValue(), 
            400.0f * getModConfig().Scale.GetValue()
        });
    }

    void LoadInGame(){
        LeftScreenIG = BeatSaberUI::CreateFloatingScreen(
            { 120, 240 },
            {
                ( getModConfig().X.GetValue() * -1 ) - 15.0f,
                getModConfig().Y.GetValue() + 17.0f,
                25.0
            },
            { 0, 0, 0 },
            0.0f,
            false,
            false,
            1
        );

        RightScreenIG = BeatSaberUI::CreateFloatingScreen(
            { 120, 240 },
            {
                getModConfig().X.GetValue() + 15.0f,
                getModConfig().Y.GetValue() + 17.0f,
                25.0
            },
            { 0, 0, 0 },
            0.0f,
            false,
            false,
            1
        );

        getLogger().info("Created Screens");

        HMUI::ImageView* leftImageIG = BeatSaberUI::CreateImage(LeftScreenIG->get_transform(), Banners::GetLeft(), { 0, 0 }, {
            200.0f * getModConfig().Scale.GetValue(), 
            400.0f * getModConfig().Scale.GetValue()
        });

        HMUI::ImageView* rightImageIG = BeatSaberUI::CreateImage(RightScreenIG->get_transform(), Banners::GetRight(), { 0, 0 }, {
            200.0f * getModConfig().Scale.GetValue(), 
            400.0f * getModConfig().Scale.GetValue()
        });
    }

    UnityEngine::Sprite* GetLeft(){
        getLogger().info("Loading left Banner, %s", getModConfig().LeftBanner.GetValue().c_str());
        return QuestUI::BeatSaberUI::FileToSprite(getModConfig().LeftBanner.GetValue());
    }

    UnityEngine::Sprite* GetRight(){
        getLogger().info("Loading right Banner, %s", getModConfig().RightBanner.GetValue().c_str());
        return QuestUI::BeatSaberUI::FileToSprite(getModConfig().RightBanner.GetValue());
    }
}