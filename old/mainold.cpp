#include "main.hpp"
#include "ModConfig.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/SharedCoroutineStarter.hpp"
#include "GlobalNamespace/FileHelpers.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/CustomPreviewBeatmapLevel.hpp"

#include "config-utils/shared/config-utils.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/CanvasRenderer.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/RenderMode.hpp"
#include "UnityEngine/UI/CanvasScaler.hpp"
#include "UnityEngine/CanvasRenderer.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/Cloth.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/WaitForSeconds.hpp"

#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/Touchable.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "HMUI/ImageView.hpp"

#include "custom-types/shared/coroutine.hpp"

#include "System/Action.hpp"
#include "System/Action_1.hpp"
#include "System/Action_2.hpp"
#include "System/Action_3.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Threading/Tasks/TaskStatus.hpp"
#include "System/Collections/IEnumerator.hpp"

#include "Utils/WebUtils.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm> 
#include <string>

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;
using namespace HMUI;
using namespace System::Threading;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup
DEFINE_CONFIG(ModConfig);

UnityEngine::Canvas* canvas;
UnityEngine::UI::VerticalLayoutGroup* layout;

bool CustomLevel = false;

Button* button = nullptr;
Button* button1 = nullptr;

Button* button11 = nullptr;
Button* button12 = nullptr;

float bannerSize = 16.0f;

UnityEngine::GameObject* screen;
UnityEngine::GameObject* screen1;

UnityEngine::Sprite* cover;

UnityEngine::Sprite* webBanner;

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

std::optional<std::vector<uint8_t>> GetImage(std::string url) {
    std::string data;
    WebUtils::Get(url, 64, data);

    if (data.empty()) return std::nullopt;
    std::vector<uint8_t> bytes(data.begin(), data.end());

    return bytes;
}

std::vector<std::string> getFiles(std::string_view path) {
    std::vector<std::string> directories;
    if(!std::filesystem::is_directory(path))
        return directories;
    std::error_code ec;
    auto directory_iterator = std::filesystem::directory_iterator(path, std::filesystem::directory_options::none, ec);
    if (ec) {
        getLogger().info("Error reading directory at %s: %s", path.data(), ec.message().c_str());
    }
    for (const auto& entry : directory_iterator) {
        if(entry.is_regular_file())
            directories.push_back(entry.path().string());
    }
    return directories;
}

std::string RemoveExtension(std::string path)
{
    if(path.find_last_of(".") != std::string::npos)
        return path.substr(0, path.find_last_of("."));
    return path;
}

std::string GetFileName(std::string path, bool removeExtension)
{
    std::string result = "";
    if(path.find_last_of("/") != std::string::npos)
        result = path.substr(path.find_last_of("/")+1);
    else result = path;
    if (removeExtension) result = RemoveExtension(result);
    return result;;
}

void loadSongBanner(GameObject* screen){
    auto button = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
    BeatSaberUI::SetButtonSprites(button, cover, cover);

    button->get_transform()->set_localScale({3.0f * getModConfig().Scale.GetValue(), 3.0f * getModConfig().Scale.GetValue(), 3.0f * getModConfig().Scale.GetValue()});
    button->set_interactable(false);
}

void loadBannersIG(){
    if(getModConfig().WebInUse.GetValue()){
        UnityEngine::GameObject* screen = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {getModConfig().X.GetValue() + 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        UnityEngine::GameObject* screen1 = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {(getModConfig().X.GetValue() * -1) - 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        // UnityEngine::GameObject* plane = UnityEngine::GameObject::CreatePrimitive(UnityEngine::PrimitiveType::Plane);
        // plane->get_transform()->set_localPosition({ 1, 1, 1 });

        // auto cloth2 = plane->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth2) cloth2 = plane->get_gameObject()->AddComponent<Cloth*>();

        // cloth2->set_externalAcceleration({0, 0, -1});

        getLogger().info("Created Screens");

        // auto clothE = (function_ptr_t<void, UnityEngine::Cloth*, bool>)CRASH_UNLESS(il2cpp_functions::resolve_icall("UnityEngine.Cloth::set_useGravity"));

        // auto cloth = screen->get_gameObject()->GetComponent<UnityEngine::Cloth*>();
        // if (!cloth) cloth = screen->get_gameObject()->AddComponent<UnityEngine::Cloth*>();

        // auto cloth1 = screen1->get_gameObject()->GetComponent<UnityEngine::Cloth*>();
        // if (!cloth1) cloth1 = screen1->get_gameObject()->AddComponent<UnityEngine::Cloth*>();

        // clothE(cloth, true);

        // cloth->set_externalAcceleration({0, 0, -100});  
        // cloth1->set_externalAcceleration({0, 0, -100});   
        
        // cloth->selfCollision = true;
        // cloth->useContinuousCollision = 0.0f;

        // cloth1->selfCollision = true;
        // cloth1->useContinuousCollision = 0.0f;

        if(getModConfig().SongBanner.GetValue() && CustomLevel){
            std::string lPath = "/sdcard/Pictures/banners/template (DON'T DELETE).png";
            auto lsprite = QuestUI::BeatSaberUI::FileToSprite(lPath);

            std::string rPath = "/sdcard/Pictures/banners/template (DON'T DELETE).png";
            auto rsprite = QuestUI::BeatSaberUI::FileToSprite(rPath);

            getLogger().info("Loaded Imaged");

            button12 = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button12, lsprite, lsprite);

            button11 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button11, rsprite, rsprite);

            button12->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button12->set_interactable(false);

            button11->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button11->set_interactable(false);

            loadSongBanner(screen);
            loadSongBanner(screen1);
        } else{
            // auto lsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().LeftBanner.GetValue());
            // auto rsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().RightBanner.GetValue());

            getLogger().info("Loaded Imaged");

            button12 = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button12, webBanner, webBanner);

            button11 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button11, webBanner, webBanner);

            button12->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button12->set_interactable(false);

            button11->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button11->set_interactable(false);
        }
    } else{
        UnityEngine::GameObject* screen = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {getModConfig().X.GetValue() + 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        UnityEngine::GameObject* screen1 = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {(getModConfig().X.GetValue() * -1) - 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        // UnityEngine::GameObject* plane = UnityEngine::GameObject::CreatePrimitive(UnityEngine::PrimitiveType::Plane);
        // plane->get_transform()->set_localPosition({ 1, 1, 1 });

        // auto cloth2 = plane->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth2) cloth2 = plane->get_gameObject()->AddComponent<Cloth*>();

        // cloth2->set_externalAcceleration({0, 0, -1});

        getLogger().info("Created Screens");

        // auto clothE = (function_ptr_t<void, UnityEngine::Cloth*, bool>)CRASH_UNLESS(il2cpp_functions::resolve_icall("UnityEngine.Cloth::set_useGravity"));

        // auto cloth = screen->get_gameObject()->GetComponent<UnityEngine::Cloth*>();
        // if (!cloth) cloth = screen->get_gameObject()->AddComponent<UnityEngine::Cloth*>();

        // auto cloth1 = screen1->get_gameObject()->GetComponent<UnityEngine::Cloth*>();
        // if (!cloth1) cloth1 = screen1->get_gameObject()->AddComponent<UnityEngine::Cloth*>();

        // clothE(cloth, true);

        // cloth->set_externalAcceleration({0, 0, -100});  
        // cloth1->set_externalAcceleration({0, 0, -100});   
        
        // cloth->selfCollision = true;
        // cloth->useContinuousCollision = 0.0f;

        // cloth1->selfCollision = true;
        // cloth1->useContinuousCollision = 0.0f;

        if(getModConfig().SongBanner.GetValue() && CustomLevel){
            std::string lPath = "/sdcard/Pictures/banners/template (DON'T DELETE).png";
            auto lsprite = QuestUI::BeatSaberUI::FileToSprite(lPath);

            std::string rPath = "/sdcard/Pictures/banners/template (DON'T DELETE).png";
            auto rsprite = QuestUI::BeatSaberUI::FileToSprite(rPath);

            getLogger().info("Loaded Imaged");

            button12 = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button12, lsprite, lsprite);

            button11 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button11, rsprite, rsprite);

            button12->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button12->set_interactable(false);

            button11->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button11->set_interactable(false);

            loadSongBanner(screen);
            loadSongBanner(screen1);
        } else{
            auto lsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().LeftBanner.GetValue());
            auto rsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().RightBanner.GetValue());

            getLogger().info("Loaded Imaged");

            button12 = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button12, lsprite, lsprite);

            button11 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button11, rsprite, rsprite);

            button12->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button12->set_interactable(false);

            button11->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button11->set_interactable(false);
        }
    }
}

void loadBannersMenu(){
    if(getModConfig().WebInUse.GetValue()){
        screen = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {getModConfig().X.GetValue() + 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        screen1 = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {(getModConfig().X.GetValue() * -1) - 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        // UnityEngine::GameObject* plane = UnityEngine::GameObject::CreatePrimitive(UnityEngine::PrimitiveType::Plane);
        // plane->get_transform()->set_localPosition({ 1, 1, 1 });

        // auto cloth2 = plane->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth2) cloth2 = plane->get_gameObject()->AddComponent<Cloth*>();

        // cloth2->set_externalAcceleration({0, 0, -1});

        getLogger().info("Created Screens");

        // auto cloth = screen->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth) cloth = screen->get_gameObject()->AddComponent<Cloth*>();

        // auto cloth1 = screen1->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth1) cloth1 = screen1->get_gameObject()->AddComponent<Cloth*>();

        // cloth->set_externalAcceleration({0, 0, -1});  
        // cloth1->set_externalAcceleration({0, 0, -1});   
        
        // cloth->selfCollision = true;
        // cloth->useContinuousCollision = 0.0f;

        // cloth1->selfCollision = true;
        // cloth1->useContinuousCollision = 0.0f;

        // auto lsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().LeftBanner.GetValue());
        // auto rsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().RightBanner.GetValue());

        getLogger().info("Loaded Imaged");

        button = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
        BeatSaberUI::SetButtonSprites(button, webBanner, webBanner);

        button1 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
        BeatSaberUI::SetButtonSprites(button1, webBanner, webBanner);

        button->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
        button->set_interactable(false);

        button1->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
        button1->set_interactable(false);
    } else{
        screen = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {getModConfig().X.GetValue() + 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        screen1 = BeatSaberUI::CreateFloatingScreen(
            {120, 240},
            {(getModConfig().X.GetValue() * -1) - 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
            {0, 0, 0},
            0.0f,
            false,
            false,
            1
        );

        // UnityEngine::GameObject* plane = UnityEngine::GameObject::CreatePrimitive(UnityEngine::PrimitiveType::Plane);
        // plane->get_transform()->set_localPosition({ 1, 1, 1 });

        // auto cloth2 = plane->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth2) cloth2 = plane->get_gameObject()->AddComponent<Cloth*>();

        // cloth2->set_externalAcceleration({0, 0, -1});

        getLogger().info("Created Screens");

        // auto cloth = screen->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth) cloth = screen->get_gameObject()->AddComponent<Cloth*>();

        // auto cloth1 = screen1->get_gameObject()->GetComponent<Cloth*>();
        // if (!cloth1) cloth1 = screen1->get_gameObject()->AddComponent<Cloth*>();

        // cloth->set_externalAcceleration({0, 0, -1});  
        // cloth1->set_externalAcceleration({0, 0, -1});   
        
        // cloth->selfCollision = true;
        // cloth->useContinuousCollision = 0.0f;

        // cloth1->selfCollision = true;
        // cloth1->useContinuousCollision = 0.0f;

        getLogger().info("Loading left Banner, %s", getModConfig().LeftBanner.GetValue().c_str());
        auto lsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().LeftBanner.GetValue());
        getLogger().info("Loading right Banner, %s", getModConfig().RightBanner.GetValue().c_str());
        auto rsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().RightBanner.GetValue());

        getLogger().info("Loaded Imaged");

        button = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
        BeatSaberUI::SetButtonSprites(button, lsprite, lsprite);

        button1 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
        BeatSaberUI::SetButtonSprites(button1, rsprite, rsprite);

        button->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
        button->set_interactable(false);

        button1->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
        button1->set_interactable(false);
    }
}

void reloadBanners(){
    GameObject::Destroy(screen);
    GameObject::Destroy(screen1);

    if(getModConfig().Active.GetValue()){
        if(getModConfig().WebInUse.GetValue()){
            screen = BeatSaberUI::CreateFloatingScreen(
                {120, 240},
                {getModConfig().X.GetValue() + 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
                {0, 0, 0},
                0.0f,
                false,
                false,
                1
            );

            // auto cloth = screen->get_gameObject()->GetComponent<Cloth*>();
            // if (!cloth) cloth = screen->get_gameObject()->AddComponent<Cloth*>();

            // cloth->set_externalAcceleration({0, 0, -1});
            // cloth->selfCollision = true;


            // auto rsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().RightBanner.GetValue());

            button = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button, webBanner, webBanner);

            button->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button->set_interactable(false);

            screen1 = BeatSaberUI::CreateFloatingScreen(
                {120, 240},
                {(getModConfig().X.GetValue() * -1) - 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
                {0, 0, 0},
                0.0f,
                false,
                false,
                1
            );

            // auto cloth1 = screen1->get_gameObject()->GetComponent<Cloth*>();
            // if (!cloth1) cloth1 = screen1->get_gameObject()->AddComponent<Cloth*>();

            // cloth1->set_externalAcceleration({0, 0, -1});
            // cloth1->selfCollision = true;
            // cloth1->useContinuousCollision = 0.0f;

            // auto lsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().LeftBanner.GetValue());

            button1 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button1, webBanner, webBanner);

            button1->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button1->set_interactable(false);
        } else{
            screen = BeatSaberUI::CreateFloatingScreen(
                {120, 240},
                {getModConfig().X.GetValue() + 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
                {0, 0, 0},
                0.0f,
                false,
                false,
                1
            );

            // auto cloth = screen->get_gameObject()->GetComponent<Cloth*>();
            // if (!cloth) cloth = screen->get_gameObject()->AddComponent<Cloth*>();

            // cloth->set_externalAcceleration({0, 0, -1});
            // cloth->selfCollision = true;


            auto rsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().RightBanner.GetValue());

            button = QuestUI::BeatSaberUI::CreateUIButton(screen->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button, rsprite, rsprite);

            button->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button->set_interactable(false);

            screen1 = BeatSaberUI::CreateFloatingScreen(
                {120, 240},
                {(getModConfig().X.GetValue() * -1) - 15.0f, getModConfig().Y.GetValue() + 17.0f, 25.0},
                {0, 0, 0},
                0.0f,
                false,
                false,
                1
            );

            // auto cloth1 = screen1->get_gameObject()->GetComponent<Cloth*>();
            // if (!cloth1) cloth1 = screen1->get_gameObject()->AddComponent<Cloth*>();

            // cloth1->set_externalAcceleration({0, 0, -1});
            // cloth1->selfCollision = true;
            // cloth1->useContinuousCollision = 0.0f;

            auto lsprite = QuestUI::BeatSaberUI::FileToSprite(getModConfig().LeftBanner.GetValue());

            button1 = QuestUI::BeatSaberUI::CreateUIButton(screen1->get_transform(), "", "SettingsButton", {0, 0}, {10, 10}, nullptr);
            BeatSaberUI::SetButtonSprites(button1, lsprite, lsprite);

            button1->get_transform()->set_localScale({16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue(), 16.0f * getModConfig().Scale.GetValue()});
            button1->set_interactable(false);
        }
    }
}

void GetCoverTexture(System::Threading::Tasks::Task_1<UnityEngine::Sprite*>* coverSpriteTask) {
    cover = coverSpriteTask->get_Result();
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

// MAKE_HOOK_MATCH(StandardLevelDetailViewController_DidActivate, &GlobalNamespace::StandardLevelDetailViewController::DidActivate, void,
//     GlobalNamespace::StandardLevelDetailViewController* self,
//     bool firstActivation,
//     bool addedToHeirarchy,
//     bool screenSystemEnabling
// ) {
//     StandardLevelDetailViewController_DidActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);

//     auto levelBarTransform = self->get_transform()->Find(il2cpp_utils::newcsstr("LevelDetail"))->Find(il2cpp_utils::newcsstr("LevelBarBig"));
//     if (!levelBarTransform) return;

//     auto* imageTransform = levelBarTransform->Find(il2cpp_utils::newcsstr("SongArtwork"))->GetComponent<RectTransform*>();
//     ImageView* imageView = imageTransform->GetComponent<ImageView*>();

//     cover = imageView->get_sprite();
// }

custom_types::Helpers::Coroutine shrinkBanners() {
    for (int i = 0; i < 10; i++) {
        if(bannerSize <= 16.0f)co_return;

        bannerSize = bannerSize - 0.1f;
        getLogger().info("Banner Size %f", bannerSize);

        button12->get_transform()->set_localScale({bannerSize * getModConfig().Scale.GetValue(), bannerSize * getModConfig().Scale.GetValue(), bannerSize * getModConfig().Scale.GetValue()});
        button11->get_transform()->set_localScale({bannerSize * getModConfig().Scale.GetValue(), bannerSize * getModConfig().Scale.GetValue(), bannerSize * getModConfig().Scale.GetValue()});

        co_yield reinterpret_cast<System::Collections::IEnumerator*>(CRASH_UNLESS(WaitForSeconds::New_ctor(0.1f)));
    }

    co_return;
}

MAKE_HOOK_MATCH(ScoreController_HandleNoteWasCut, &ScoreController::HandleNoteWasCut, void,
    ScoreController* self,
    NoteController* note,
    ByRef<GlobalNamespace::NoteCutInfo> info
) {
    ScoreController_HandleNoteWasCut(self, note, info);

    if(getModConfig().Active.GetValue() && getModConfig().ShowInGame.GetValue() && getModConfig().Reactive.GetValue()){
        getLogger().info("Making Banners bigger");

        button12->get_transform()->set_localScale({17.0f * getModConfig().Scale.GetValue(), 17.0f * getModConfig().Scale.GetValue(), 17.0f * getModConfig().Scale.GetValue()});
        button11->get_transform()->set_localScale({17.0f * getModConfig().Scale.GetValue(), 17.0f * getModConfig().Scale.GetValue(), 17.0f * getModConfig().Scale.GetValue()});
        bannerSize = 17.0f;

        SharedCoroutineStarter::get_instance()->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(shrinkBanners()));
    }
}

MAKE_HOOK_MATCH(RefreshContent, &StandardLevelDetailView::RefreshContent, void,
    StandardLevelDetailView* self
) {
    RefreshContent(self);

    IPreviewBeatmapLevel* level = reinterpret_cast<IPreviewBeatmapLevel*>(self->dyn__level());

    auto coverSpriteTask = level->GetCoverImageAsync(CancellationToken::get_None());

    CustomLevel = (il2cpp_functions::class_is_assignable_from(classof(CustomPreviewBeatmapLevel*), il2cpp_functions::object_get_class(reinterpret_cast<Il2CppObject*>(level))));
    if (CustomLevel) {
        auto action = il2cpp_utils::MakeDelegate<System::Action_1<System::Threading::Tasks::Task*>*>(classof(System::Action_1<System::Threading::Tasks::Task*>*), coverSpriteTask, GetCoverTexture);
        reinterpret_cast<System::Threading::Tasks::Task*>(coverSpriteTask)->ContinueWith(action);
    }
}

MAKE_HOOK_MATCH(AudioTimeSyncController_StartSong, &AudioTimeSyncController::StartSong, void,
    AudioTimeSyncController* self,
    float startTimeOffset
) {
    AudioTimeSyncController_StartSong(self, startTimeOffset);

    if(getModConfig().Active.GetValue() && getModConfig().ShowInGame.GetValue()){
        loadBannersIG();
    }
}

MAKE_HOOK_MATCH(MainMenuViewController_DidActivate, &MainMenuViewController::DidActivate, void,
    MainMenuViewController* self,
    bool firstActivation,
    bool addedToHierarchy,
    bool screenSystemEnabling
) {
    MainMenuViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    getLogger().info("Menu Opened");

    if(!direxists("/sdcard/Pictures/banners/")){
        getLogger().info("Banner folder does not exist");

        int makePath = mkpath("/sdcard/Pictures/banners/");
        getLogger().info("Made Folder");

        if (makePath == -1){
            getLogger().error("Failed to make Folder!");
        }
    }

    if(firstActivation && getModConfig().Active.GetValue() && getModConfig().WebInUse.GetValue()){
        getLogger().info("Loading WebBanners");

        std::string url = "https://bs.phazed.xyz/api/v1/canvas.png?id="+getModConfig().BannerID.GetValue();
        getLogger().info("Found Image: %s", url.c_str());

        std::optional<std::vector<uint8_t>> imgUrl = GetImage(url);
        getLogger().info("Got image data");

        if(imgUrl.has_value()){
            getLogger().info("creating sprites");
            webBanner = QuestUI::BeatSaberUI::ArrayToSprite(il2cpp_utils::vectorToArray(imgUrl.value()));
        }
    }

    if (firstActivation && getModConfig().Active.GetValue()) {
        getLogger().info("Loading banners");
        loadBannersMenu();
    }
};

std::string linkCode = "";

custom_types::Helpers::Coroutine loadMenu(auto files, UnityEngine::GameObject* bannerm, UnityEngine::GameObject* settings) {
    for (auto &file : files) {
        getLogger().info("Banner %s", file.c_str());

        HorizontalLayoutGroup* levelBarLayout = BeatSaberUI::CreateHorizontalLayoutGroup(bannerm->get_transform());
        GameObject* prefab = levelBarLayout->get_gameObject();

        levelBarLayout->set_childControlWidth(false);
        levelBarLayout->set_childForceExpandWidth(true);

        LayoutElement* levelBarLayoutElement = levelBarLayout->GetComponent<LayoutElement*>();
        levelBarLayoutElement->set_minHeight(9.0f);
        levelBarLayoutElement->set_minWidth(90.0f);

        BeatSaberUI::CreateText(levelBarLayoutElement->get_transform(), GetFileName(file.c_str(), true), UnityEngine::Vector2(0.0f, 0.0f));

        HMUI::ModalView* modal = BeatSaberUI::CreateModal(settings->get_transform(), UnityEngine::Vector2(60.0f, 40.0f), [](HMUI::ModalView* modal){}, true);
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableModalContainer(modal);

        BeatSaberUI::CreateText(container->get_transform(), GetFileName(file.c_str(), false));

        std::string path = file.c_str();
        auto sprite_active = QuestUI::BeatSaberUI::FileToSprite(path);
        std::string otherPath = file.c_str();
        auto sprite_inactive = QuestUI::BeatSaberUI::FileToSprite(otherPath);
        co_yield nullptr;

        Button* previewBtn = BeatSaberUI::CreateUIButton(container->get_transform(), "", "SettingsButton", {0, 0}, {10, 20}, nullptr);
        BeatSaberUI::SetButtonSprites(previewBtn, sprite_inactive, sprite_active);
        co_yield nullptr;

        previewBtn->get_transform()->set_localScale({1.0f, 1.0f, 1.0f});

        BeatSaberUI::CreateUIButton(container->get_transform(), "Set Both Banners", "PlayButton", Vector2(0.0f, 0.0f), Vector2(24.0f, 15.0f), [file]() {
            getLogger().info("Set Both Banners: %s", file.c_str());
            getModConfig().WebInUse.SetValue(false);

            getModConfig().LeftBanner.SetValue(file.c_str());
            getModConfig().RightBanner.SetValue(file.c_str());

            reloadBanners();
        });

        BeatSaberUI::CreateUIButton(container->get_transform(), "Set Left Banner", "PlayButton", Vector2(0.0f, 0.0f), Vector2(24.0f, 15.0f), [file]() {
            getLogger().info("Set Left Banner: %s", file.c_str());
            getModConfig().WebInUse.SetValue(false);

            getModConfig().LeftBanner.SetValue(file.c_str());

            reloadBanners();
        });

        BeatSaberUI::CreateUIButton(container->get_transform(), "Set Right Banner", "PlayButton", Vector2(0.0f, 0.0f), Vector2(24.0f, 15.0f), [file]() {
            getLogger().info("Set Right Banner: %s", file.c_str());
            getModConfig().WebInUse.SetValue(false);

            getModConfig().RightBanner.SetValue(file.c_str());

            reloadBanners();
        });

        Button* usebutton = BeatSaberUI::CreateUIButton(levelBarLayoutElement->get_transform(), "", "PlayButton", Vector2(0.0f, 0.0f), Vector2(12.0f, 0.0f),
            [modal]() {
                getLogger().info("Button Clicked");
                
                modal->Show(true, true, nullptr);
            });

        auto text = QuestUI::BeatSaberUI::CreateText(usebutton->get_transform(), "Use");
        text->set_alignment(TMPro::TextAlignmentOptions::Center);
    }

    co_return;
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
    if(firstActivation){
        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* settings = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        std::vector<std::string> files = getFiles("/sdcard/Pictures/banners/");

        getLogger().info("%lu files found", files.size());

        HMUI::ModalView* settingsModal = BeatSaberUI::CreateModal(settings->get_transform(), UnityEngine::Vector2(120.0f, 80.0f), [](HMUI::ModalView* modal){}, true);
        UnityEngine::GameObject* settingsm = BeatSaberUI::CreateScrollableModalContainer(settingsModal);

        TMPro::TextMeshProUGUI* userText = BeatSaberUI::CreateText(settingsm->get_transform(), "PhazeSaber Link Settings");
        
        BeatSaberUI::CreateText(settingsm->get_transform(), "Qustom Banners Settings");

        BeatSaberUI::CreateToggle(settingsm->get_transform(), "Active", getModConfig().Active.GetValue(),
            [](bool value) { 
                getModConfig().Active.SetValue(value);
            });

        BeatSaberUI::CreateToggle(settingsm->get_transform(), "Show In Song", getModConfig().ShowInGame.GetValue(),
            [](bool value) { 
                getModConfig().ShowInGame.SetValue(value);
            });

        BeatSaberUI::CreateToggle(settingsm->get_transform(), "Reactive Banners", getModConfig().Reactive.GetValue(),
            [](bool value) { 
                getModConfig().Reactive.SetValue(value);
            });

        BeatSaberUI::CreateToggle(settingsm->get_transform(), "Enable Song Banners", getModConfig().SongBanner.GetValue(),
            [](bool value) { 
                getModConfig().SongBanner.SetValue(value);
            });

        BeatSaberUI::CreateIncrementSetting(settingsm->get_transform(), "X Position", 2, 0.5f, getModConfig().X.GetValue(), -15.0f, 15.0f,
            [](float value) { 
                getModConfig().X.SetValue(value);
            });

        BeatSaberUI::CreateIncrementSetting(settingsm->get_transform(), "Y Position", 2, 0.5f, getModConfig().Y.GetValue(), -15.0f, 15.0f,
            [](float value) { 
                getModConfig().Y.SetValue(value);
            });

        BeatSaberUI::CreateIncrementSetting(settingsm->get_transform(), "Scale", 2, 0.1f, getModConfig().Scale.GetValue(), 0.0f, 2.0f,
            [](float value) { 
                getModConfig().Scale.SetValue(value);
            });

        BeatSaberUI::CreateUIButton(settingsm->get_transform(), "Save",
            [&]() {
                reloadBanners();
            });

        BeatSaberUI::CreateUIButton(settings->get_transform(), "Settings",
            [settingsModal]() {
                getLogger().info("Button Clicked");

                settingsModal->Show(true, true, nullptr);
            });

        HMUI::ModalView* settingsModal1 = BeatSaberUI::CreateModal(settings->get_transform(), UnityEngine::Vector2(120.0f, 80.0f), [](HMUI::ModalView* modal){}, true);
        UnityEngine::GameObject* bannerm = BeatSaberUI::CreateScrollableModalContainer(settingsModal1);

        BeatSaberUI::CreateUIButton(settings->get_transform(), "Local Banners",
            [settingsModal1]() {
                getLogger().info("Button Clicked");

                settingsModal1->Show(true, true, nullptr);
            });

        SharedCoroutineStarter::get_instance()->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(loadMenu(files, bannerm, settings)));

        // if(getModConfig().DeviceID.GetValue() != ""){
        //     auto json = WebUtils::GetJSON("https://bs.phazed.xyz/api/v1/banners/byDevice?id="+getModConfig().DeviceID.GetValue());

        //     getLogger().info("Got banners");

        //     if(json->IsObject()){
        //         auto jsonData = json->GetObject();
        //         getLogger().info("Got object");

        //         if(jsonData["banners"].IsArray()){
        //             auto data = jsonData["banners"].GetArray();
        //             int bannerCount = jsonData["banners"].Size();
        //             getLogger().info("Got array");

        //             getLogger().info("Amount: %i", bannerCount);

        //             HorizontalLayoutGroup* levelBarLayout = BeatSaberUI::CreateHorizontalLayoutGroup(settings->get_transform());

        //             LayoutElement* levelBarLayoutElement = levelBarLayout->GetComponent<LayoutElement*>();
        //             levelBarLayoutElement->set_minHeight(55.0f);
        //             levelBarLayoutElement->set_minWidth(90.0f);

        //             if(bannerCount >= 1){
        //                 getLogger().info("Banner 1 ID: %s", data[0]["_id"].GetString());

        //                 std::string url = "https://bs.phazed.xyz/api/v1/canvas.png?id="+std::string(data[0]["_id"].GetString());
        //                 getLogger().info("Found Image: %s", url.c_str());

        //                 std::optional<std::vector<uint8_t>> imgUrl = GetImage(url);
        //                 getLogger().info("Got image data");

        //                 if(imgUrl.has_value()){
        //                     VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(levelBarLayout->get_transform());

        //                     getLogger().info("creating sprites");
        //                     auto sprite = QuestUI::BeatSaberUI::ArrayToSprite(il2cpp_utils::vectorToArray(imgUrl.value()));

        //                     getLogger().info("creating button");
        //                     auto button = BeatSaberUI::CreateUIButton(layout->get_transform(), "", "SettingsButton", Vector2(10, 10), nullptr);
        //                     getLogger().info("created button");

        //                     BeatSaberUI::SetButtonSprites(button, sprite, sprite);
        //                     getLogger().info("set sprites");

        //                     button->get_transform()->set_localScale({1.0f, 1.0f, 1.0f});
        //                     button->set_interactable(false);
        //                     getLogger().info("created image");

        //                     std::string id = data[0]["_id"].GetString();

        //                     HorizontalLayoutGroup* buttonC = BeatSaberUI::CreateHorizontalLayoutGroup(layout->get_transform());

        //                     LayoutElement* buttonCElement = buttonC->GetComponent<LayoutElement*>();
        //                     buttonCElement->set_minHeight(12.0f);

        //                     Button* usebutton = BeatSaberUI::CreateUIButton(buttonC->get_transform(), "", "PlayButton", Vector2(0.0f, 0.0f), Vector2(12.0f, 0.0f),
        //                         [sprite, id]() {
        //                             getModConfig().WebInUse.SetValue(true);
        //                             getModConfig().BannerID.SetValue(id);

        //                             webBanner = sprite;

        //                             reloadBanners();
        //                         });

        //                     auto text = QuestUI::BeatSaberUI::CreateText(usebutton->get_transform(), "Use");
        //                     text->set_alignment(TMPro::TextAlignmentOptions::Center);
        //                 }
        //             }

        //             if(bannerCount >= 2){
        //                 getLogger().info("Banner 2 ID: %s", data[1]["_id"].GetString());

        //                 std::string url = "https://bs.phazed.xyz/api/v1/canvas.png?id="+std::string(data[1]["_id"].GetString());
        //                 getLogger().info("Found Image: %s", url.c_str());

        //                 std::optional<std::vector<uint8_t>> imgUrl = GetImage(url);
        //                 getLogger().info("Got image data");

        //                 if(imgUrl.has_value()){
        //                     VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(levelBarLayout->get_transform());

        //                     getLogger().info("creating sprites");
        //                     auto sprite = QuestUI::BeatSaberUI::ArrayToSprite(il2cpp_utils::vectorToArray(imgUrl.value()));

        //                     getLogger().info("creating button");
        //                     auto button = BeatSaberUI::CreateUIButton(layout->get_transform(), "", "SettingsButton", Vector2(10, 10), nullptr);
        //                     getLogger().info("created button");

        //                     BeatSaberUI::SetButtonSprites(button, sprite, sprite);
        //                     getLogger().info("set sprites");

        //                     button->get_transform()->set_localScale({1.0f, 1.0f, 1.0f});
        //                     button->set_interactable(false);
        //                     getLogger().info("created image");

        //                     std::string id = data[1]["_id"].GetString();

        //                     HorizontalLayoutGroup* buttonC = BeatSaberUI::CreateHorizontalLayoutGroup(layout->get_transform());

        //                     LayoutElement* buttonCElement = buttonC->GetComponent<LayoutElement*>();
        //                     buttonCElement->set_minHeight(12.0f);

        //                     Button* usebutton = BeatSaberUI::CreateUIButton(buttonC->get_transform(), "", "PlayButton", Vector2(0.0f, 0.0f), Vector2(12.0f, 0.0f),
        //                         [sprite, id]() {
        //                             getModConfig().WebInUse.SetValue(true);
        //                             getModConfig().BannerID.SetValue(id);

        //                             webBanner = sprite;

        //                             reloadBanners();
        //                         });

        //                     auto text = QuestUI::BeatSaberUI::CreateText(usebutton->get_transform(), "Use");
        //                     text->set_alignment(TMPro::TextAlignmentOptions::Center);
        //                 }
        //             }

        //             if(bannerCount >= 3){
        //                 getLogger().info("Banner 3 ID: %s", data[2]["_id"].GetString());

        //                 std::string url = "https://bs.phazed.xyz/api/v1/canvas.png?id="+std::string(data[2]["_id"].GetString());
        //                 getLogger().info("Found Image: %s", url.c_str());

        //                 std::optional<std::vector<uint8_t>> imgUrl = GetImage(url);
        //                 getLogger().info("Got image data");

        //                 if(imgUrl.has_value()){
        //                    VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(levelBarLayout->get_transform());

        //                     getLogger().info("creating sprites");
        //                     auto sprite = QuestUI::BeatSaberUI::ArrayToSprite(il2cpp_utils::vectorToArray(imgUrl.value()));

        //                     getLogger().info("creating button");
        //                     auto button = BeatSaberUI::CreateUIButton(layout->get_transform(), "", "SettingsButton", Vector2(10, 10), nullptr);
        //                     getLogger().info("created button");

        //                     BeatSaberUI::SetButtonSprites(button, sprite, sprite);
        //                     getLogger().info("set sprites");

        //                     button->get_transform()->set_localScale({1.0f, 1.0f, 1.0f});
        //                     button->set_interactable(false);
        //                     getLogger().info("created image");

        //                     std::string id = data[2]["_id"].GetString();

        //                     HorizontalLayoutGroup* buttonC = BeatSaberUI::CreateHorizontalLayoutGroup(layout->get_transform());

        //                     LayoutElement* buttonCElement = buttonC->GetComponent<LayoutElement*>();
        //                     buttonCElement->set_minHeight(12.0f);

        //                     Button* usebutton = BeatSaberUI::CreateUIButton(buttonC->get_transform(), "", "PlayButton", Vector2(0.0f, 0.0f), Vector2(12.0f, 0.0f),
        //                         [sprite, id]() {
        //                             getModConfig().WebInUse.SetValue(true);
        //                             getModConfig().BannerID.SetValue(id);

        //                             webBanner = sprite;

        //                             reloadBanners();
        //                         });

        //                     auto text = QuestUI::BeatSaberUI::CreateText(usebutton->get_transform(), "Use");
        //                     text->set_alignment(TMPro::TextAlignmentOptions::Center);
        //                 }
        //             }
        //         }
        //     }
        // }
    } else{
        
    };
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    getModConfig().Init(modInfo);

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, DidActivate);
    getLogger().info("Successfully installed Settings UI!");

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, MainMenuViewController_DidActivate);
    INSTALL_HOOK(logger, AudioTimeSyncController_StartSong);
    INSTALL_HOOK(logger, RefreshContent);
    INSTALL_HOOK(logger, ScoreController_HandleNoteWasCut);
    getLogger().info("Installed all hooks!");
}