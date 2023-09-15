#include "main.hpp"
#include "ModConfig.hpp"

#include "banners/load.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/LayerMask.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"

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
    UnityEngine::GameObject* leftScreen;
    UnityEngine::GameObject* rightScreen;
    UnityEngine::GameObject* leftScreenIG;
    UnityEngine::GameObject* rightScreenIG;

    void LoadMenu(){
        leftScreen = BeatSaberUI::CreateFloatingScreen(
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

        rightScreen = BeatSaberUI::CreateFloatingScreen(
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

        leftScreen->GetComponent<Canvas*>()->set_sortingOrder(0);
        rightScreen->GetComponent<Canvas*>()->set_sortingOrder(0);

        HMUI::ImageView* leftImage = BeatSaberUI::CreateImage(leftScreen->get_transform(), Banners::GetLeft(), { 0, 0 }, {
            200.0f * getModConfig().Scale.GetValue(), 
            400.0f * getModConfig().Scale.GetValue()
        });

        HMUI::ImageView* rightImage = BeatSaberUI::CreateImage(rightScreen->get_transform(), Banners::GetRight(), { 0, 0 }, {
            200.0f * getModConfig().Scale.GetValue(),
            400.0f * getModConfig().Scale.GetValue()
        });
    }

    void LoadInGame(){
        leftScreenIG = BeatSaberUI::CreateFloatingScreen(
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

        rightScreenIG = BeatSaberUI::CreateFloatingScreen(
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

        leftScreen->GetComponent<Canvas*>()->set_sortingOrder(0);
        rightScreen->GetComponent<Canvas*>()->set_sortingOrder(0);

        HMUI::ImageView* leftImageIG = BeatSaberUI::CreateImage(leftScreenIG->get_transform(), Banners::GetLeft(), { 0, 0 }, {
            200.0f * getModConfig().Scale.GetValue(),
            400.0f * getModConfig().Scale.GetValue()
        });

        HMUI::ImageView* rightImageIG = BeatSaberUI::CreateImage(rightScreenIG->get_transform(), Banners::GetRight(), { 0, 0 }, {
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