#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(Active, bool, "Active", true);
    CONFIG_VALUE(ShowInGame, bool, "Show In Game", true);
    CONFIG_VALUE(Reactive, bool, "Reactive", false);
    CONFIG_VALUE(X, float, "X Position", 0.0);
    CONFIG_VALUE(Y, float, "Y Position", 0.0);
    CONFIG_VALUE(Scale, float, "Scale", 1.0);
    CONFIG_VALUE(LeftBanner, std::string, "LeftBanner", "/sdcard/Pictures/banners/banner.png");
    CONFIG_VALUE(RightBanner, std::string, "RightBanner", "/sdcard/Pictures/banners/banner.png");
    CONFIG_VALUE(Token, std::string, "Token", "None");

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(Active);
        CONFIG_INIT_VALUE(ShowInGame);
        CONFIG_INIT_VALUE(Reactive);
        CONFIG_INIT_VALUE(X);
        CONFIG_INIT_VALUE(Y);
        CONFIG_INIT_VALUE(Scale);
        CONFIG_INIT_VALUE(LeftBanner);
        CONFIG_INIT_VALUE(RightBanner);
        CONFIG_INIT_VALUE(Token);
    )
)