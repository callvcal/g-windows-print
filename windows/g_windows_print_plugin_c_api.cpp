#include "g_windows_print_plugin.h"

#include <flutter/plugin_registrar_windows.h>

void GWindowsPrintPluginCApiRegisterWithRegistrar(
        FlutterDesktopPluginRegistrarRef registrar) {
    g_windows_print::GWindowsPrintPlugin::RegisterWithRegistrar(
            flutter::PluginRegistrarManager::GetInstance()
                    ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
