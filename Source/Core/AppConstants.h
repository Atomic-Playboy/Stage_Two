#pragma once
#include <string>

namespace AppConstants {
    inline std::string DashboardJson = "dashboard.json";
    inline std::string TransformerJson = "transformer.json";
    inline std::string GtrConfigs[4] = {"gtr1.json", "gtr2.json", "gtr3.json", "gtr4.json"};
    inline std::string LogFile = "stage_two.log";

    inline std::string HtmlDashboard = "HTML_Config/dashboard.html";
    inline std::string HtmlTransformer = "HTML_Config/transformer.html";
    inline std::string HtmlDashboardBlender = "HTML_Config/Dashboard_Blender.html";
    inline std::string HtmlTransformerBlender = "HTML_Config/Transformer_Blender.html";
    inline std::string HtmlConfiguratorIndex = "HTML_Config/Configurator/index.html";
    inline std::string HtmlGtrConfig = "HTML_Config/GTR_Config.html";
    inline std::string HtmlHelpManual = "HTML_Help/help.html";

    inline std::string TextEditor = "Textpad.exe";

    inline const std::string ParentDir = "../";
    inline const std::string GrandparentDir = "../../";

    inline const std::string InternalDashboardID = "Stage Two Dashboard";
    constexpr int MAX_HARDWARE_SLOTS = 4;
}
