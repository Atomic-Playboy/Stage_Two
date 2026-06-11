#pragma once
#include <string>

namespace AppConstants {
    inline const std::string DashboardJson = "dashboard.json";
    inline const std::string TransformerJson = "transformer.json";
    inline const std::string LogFile = "stage_one_runtime.log";

    inline const std::string HtmlDashboard = "HTML_Config/dashboard.html";
    inline const std::string HtmlTransformer = "HTML_Config/transformer.html";
    inline const std::string HtmlDashboardBlender = "HTML_Config/Dashboard_Blender.html";
    inline const std::string HtmlTransformerBlender = "HTML_Config/Transformer_Blender.html";
    inline const std::string HtmlConfiguratorIndex = "HTML_Config/Configurator/index.html";
    inline const std::string HtmlHelpManual = "HTML_Help/help.html";

    inline const std::string NotepadExe = "notepad.exe";
    inline const std::string ParentDir = "../";
    inline const std::string GrandparentDir = "../../";

    inline const std::string InternalDashboardID = "Stage One Dashboard";

    constexpr int MAX_HARDWARE_SLOTS = 4;
}
