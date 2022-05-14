#include "main.hpp"
#include "ModConfig.hpp"

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"

#include "banners/load.hpp"
#include "ui/settings.hpp"
#include "web/server.hpp"
#include "utils/WebUtils.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "HMUI/Touchable.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "config-utils/shared/config-utils.hpp"

#include "socket_lib/shared/ServerSocket.hpp"
#include "socket_lib/shared/SocketHandler.hpp"
#include "socket_lib/shared/SocketLogger.hpp"

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup
DEFINE_CONFIG(ModConfig);

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
	
    getLogger().info("Completed setup!");
}

MAKE_HOOK_MATCH(MainMenuViewController_DidActivate, &MainMenuViewController::DidActivate, void,
    MainMenuViewController* self,
    bool firstActivation,
    bool addedToHierarchy,
    bool screenSystemEnabling
) {
    MainMenuViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    getLogger().info("Menu Opened");

    if(firstActivation && getModConfig().Active.GetValue()){
        Banners::LoadMenu();

        if(getModConfig().Token.GetValue() == "None"){
            auto json = WebUtils::GetJSON("https://192.168.11.13/api/v1/devices/register");

            if(json->IsObject()){
                auto jsonData = json->GetObject();
                getModConfig().Token.SetValue(jsonData["token"].GetString());
                getLogger().info("Registered Device Token");
            }
        }
    }
}

MAKE_HOOK_MATCH(AudioTimeSyncController_StartSong, &AudioTimeSyncController::StartSong, void,
    AudioTimeSyncController* self,
    float startTimeOffset
) {
    AudioTimeSyncController_StartSong(self, startTimeOffset);

    if(getModConfig().Active.GetValue() && getModConfig().ShowInGame.GetValue()){
        Banners::LoadInGame();
    }
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    getModConfig().Init(modInfo);

    LoggerContextObject logger = getLogger().WithContext("load");

    SocketLib::SocketHandler::getCommonSocketHandler().getLogger().loggerCallback += []( SocketLib::LoggerLevel level, std::string_view tag, std::string_view const log ){
        getLogger().info("Socket Log: %s", std::string(log).c_str());
    };

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, BannerUI::DidActivate);
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, BannerUI::DidActivate);
    getLogger().info("Successfully installed Settings UI!");

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, MainMenuViewController_DidActivate);
    INSTALL_HOOK(logger, AudioTimeSyncController_StartSong);
    getLogger().info("Installed all hooks!");
}