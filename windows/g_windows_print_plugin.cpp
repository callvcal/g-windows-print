#include "include/g_windows_print/g_windows_print_plugin.h"

#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <winspool.h>
#include <commdlg.h>
#include <string>
#include <vector>

#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "comdlg32.lib")

namespace g_windows_print {

    namespace {
        std::wstring ToW(const std::string& s) {
            int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
            std::wstring ws(len, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, ws.data(), len);
            if (!ws.empty() && ws.back() == L'\0') ws.pop_back();
            return ws;
        }

        std::string ToU8(const std::wstring& ws) {
            int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string s(len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, s.data(), len, nullptr, nullptr);
            if (!s.empty() && s.back() == '\0') s.pop_back();
            return s;
        }
    }  // namespace

// ================= Constructor =================
    GWindowsPrintPlugin::GWindowsPrintPlugin() {}
    GWindowsPrintPlugin::~GWindowsPrintPlugin() {}

// ================= Registrar =================
    void GWindowsPrintPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar) {
        auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
                registrar->messenger(), "g_windows_print", &flutter::StandardMethodCodec::GetInstance());

        auto plugin = std::make_unique<GWindowsPrintPlugin>();

        channel->SetMethodCallHandler(
                [plugin_ptr = plugin.get()](const auto& call, auto result) {
                    plugin_ptr->HandleMethodCall(call, std::move(result));
                });

        registrar->AddPlugin(std::move(plugin));
    }

// ================= Method Dispatcher =================
    void GWindowsPrintPlugin::HandleMethodCall(
            const flutter::MethodCall<flutter::EncodableValue>& call,
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        if (call.method_name() == "getPrinters") {
            HandleGetPrinters(std::move(result));
        } else if (call.method_name() == "pickPrinter") {
            HandlePickPrinter(std::move(result));
        } else if (call.method_name() == "printPdf") {
            HandlePrintPdf(call, std::move(result));
        } else if (call.method_name() == "showPrintDialog") {
            HandleShowPrintDialog(call, std::move(result));
        } else {
            result->NotImplemented();
        }
    }

// ================= getPrinters =================
    void GWindowsPrintPlugin::HandleGetPrinters(
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        DWORD needed = 0, returned = 0;
        EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 2, nullptr, 0, &needed, &returned);

        std::vector<BYTE> buffer(needed);
        flutter::EncodableList list;

        if (EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 2,
                          buffer.data(), needed, &needed, &returned)) {
            auto* info = reinterpret_cast<PRINTER_INFO_2W*>(buffer.data());
            for (DWORD i = 0; i < returned; i++) {
                std::wstring name = info[i].pPrinterName ? info[i].pPrinterName : L"";
                flutter::EncodableMap map;
                map[flutter::EncodableValue("id")] = flutter::EncodableValue(ToU8(name));
                map[flutter::EncodableValue("name")] = flutter::EncodableValue(ToU8(name));
                list.emplace_back(map);
            }
        }
        result->Success(list);
    }

// ================= pickPrinter (default) =================
    void GWindowsPrintPlugin::HandlePickPrinter(
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        DWORD needed = 0;
        GetDefaultPrinterW(nullptr, &needed);
        std::wstring def;
        if (needed > 0) {
            def.resize(needed);
            if (GetDefaultPrinterW(def.data(), &needed)) {
                if (!def.empty() && def.back() == L'\0') def.pop_back();
            }
        }
        if (!def.empty()) {
            flutter::EncodableMap m;
            m[flutter::EncodableValue("id")] = flutter::EncodableValue(ToU8(def));
            m[flutter::EncodableValue("name")] = flutter::EncodableValue(ToU8(def));
            result->Success(m);
        } else {
            result->Success(flutter::EncodableValue());
        }
    }

// ================= printPdf (direct) =================
    void GWindowsPrintPlugin::HandlePrintPdf(
            const flutter::MethodCall<flutter::EncodableValue>& call,
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        const auto* args = std::get_if<flutter::EncodableMap>(call.arguments());
        if (!args) {
            result->Error("bad_args", "Expected map arguments");
            return;
        }

        std::string printerId = std::get<std::string>(args->at(flutter::EncodableValue("printerId")));
        auto data = std::get<std::vector<uint8_t>>(args->at(flutter::EncodableValue("pdfData")));
        std::string jobName = "Flutter PDF Job";
        auto itJob = args->find(flutter::EncodableValue("jobName"));
        if (itJob != args->end()) jobName = std::get<std::string>(itJob->second);

        HANDLE hPrinter = nullptr;
        std::wstring printerName = ToW(printerId);
        if (!OpenPrinterW(printerName.data(), &hPrinter, nullptr)) {
            result->Error("open_printer_failed", "Could not open printer");
            return;
        }

        std::wstring jobNameW = ToW(jobName);
        DOC_INFO_1W di{};
        di.pDocName = jobNameW.data();
        di.pOutputFile = nullptr;
        di.pDatatype = const_cast<LPWSTR>(L"RAW");

        DWORD jobId = StartDocPrinterW(hPrinter, 1, (LPBYTE)&di);
        if (jobId == 0) {
            ClosePrinter(hPrinter);
            result->Error("start_doc_failed", "Failed StartDocPrinter");
            return;
        }

        if (!StartPagePrinter(hPrinter)) {
            EndDocPrinter(hPrinter);
            ClosePrinter(hPrinter);
            result->Error("start_page_failed", "Failed StartPagePrinter");
            return;
        }

        DWORD written = 0;
        BOOL ok = WritePrinter(hPrinter, (LPVOID)data.data(), (DWORD)data.size(), &written);
        EndPagePrinter(hPrinter);
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);

        if (!ok || written != data.size()) {
            result->Error("write_failed", "WritePrinter failed");
            return;
        }

        result->Success();
    }

// ================= showPrintDialog (UI + print) =================
    void GWindowsPrintPlugin::HandleShowPrintDialog(
            const flutter::MethodCall<flutter::EncodableValue>& call,
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        const auto* args = std::get_if<flutter::EncodableMap>(call.arguments());
        if (!args) {
            result->Error("bad_args", "Expected map arguments");
            return;
        }

        auto data = std::get<std::vector<uint8_t>>(args->at(flutter::EncodableValue("pdfData")));
        std::string jobName = "Flutter PDF Job";
        auto itJob = args->find(flutter::EncodableValue("jobName"));
        if (itJob != args->end()) jobName = std::get<std::string>(itJob->second);

        PRINTDLGW pd = {0};
        pd.lStructSize = sizeof(pd);
        pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_ALLPAGES;

        if (!PrintDlgW(&pd)) {
            result->Error("cancelled", "User cancelled print dialog");
            return;
        }

        // Extract selected printer name
        DEVNAMES* devNames = (DEVNAMES*)GlobalLock(pd.hDevNames);
        if (!devNames) {
            result->Error("devnames_failed", "Could not get printer name");
            return;
        }

        LPCWSTR printerNameW = (LPCWSTR)((LPBYTE)devNames + devNames->wDeviceOffset);
        std::wstring printerName(printerNameW);
        GlobalUnlock(pd.hDevNames);

        // Open printer spooler
        HANDLE hPrinter = nullptr;
        if (!OpenPrinterW((LPWSTR)printerName.c_str(), &hPrinter, nullptr)) {
            result->Error("open_printer_failed", "Could not open printer");
            return;
        }

        std::wstring jobNameW = ToW(jobName);
        DOC_INFO_1W di{};
        di.pDocName = jobNameW.data();
        di.pOutputFile = nullptr;
        di.pDatatype = const_cast<LPWSTR>(L"RAW");

        DWORD jobId = StartDocPrinterW(hPrinter, 1, (LPBYTE)&di);
        if (jobId == 0) {
            ClosePrinter(hPrinter);
            result->Error("start_doc_failed", "Failed StartDocPrinter");
            return;
        }

        if (!StartPagePrinter(hPrinter)) {
            EndDocPrinter(hPrinter);
            ClosePrinter(hPrinter);
            result->Error("start_page_failed", "Failed StartPagePrinter");
            return;
        }

        DWORD written = 0;
        BOOL ok = WritePrinter(hPrinter, (LPVOID)data.data(), (DWORD)data.size(), &written);
        EndPagePrinter(hPrinter);
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);

        if (!ok || written != data.size()) {
            result->Error("write_failed", "WritePrinter failed in dialog");
            return;
        }

        result->Success();
    }

}  // namespace g_windows_print
