#ifndef FLUTTER_PLUGIN_G_WINDOWS_PRINT_PLUGIN_H_
#define FLUTTER_PLUGIN_G_WINDOWS_PRINT_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace g_windows_print {

    class GWindowsPrintPlugin : public flutter::Plugin {
    public:
        static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

        GWindowsPrintPlugin();
        virtual ~GWindowsPrintPlugin();

        GWindowsPrintPlugin(const GWindowsPrintPlugin&) = delete;
        GWindowsPrintPlugin& operator=(const GWindowsPrintPlugin&) = delete;

    private:
        void HandleMethodCall(
                const flutter::MethodCall<flutter::EncodableValue> &call,
                std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

        void HandleGetPrinters(std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
        void HandlePickPrinter(std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
        void HandlePrintPdf(const flutter::MethodCall<flutter::EncodableValue>& call,
                            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
        void HandleShowPrintDialog(const flutter::MethodCall<flutter::EncodableValue>& call,
                                   std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    };

}  // namespace g_windows_print

#endif  // FLUTTER_PLUGIN_G_WINDOWS_PRINT_PLUGIN_H_
