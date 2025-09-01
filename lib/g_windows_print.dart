import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'g_windows_print_platform_interface.dart';

class PrinterDropdown extends StatefulWidget {
  final String? label;

  const PrinterDropdown({super.key, this.label});

  @override
  State<PrinterDropdown> createState() => _PrinterDropdownState();
}

class _PrinterDropdownState extends State<PrinterDropdown> {
  List<PrinterInfo> _printers = [];
  PrinterInfo? _selected;

  @override
  void initState() {
    super.initState();
    _init();
  }

  Future<void> _init() async {
    // Load printers from system
    final list = await GWindowsPrint.getPrinters();
    // Try restore saved printer
    final saved = await GWindowsPrint.loadSavedPrinter();

    setState(() {
      _printers = list;
      _selected = saved;
    });
  }

  @override
  Widget build(BuildContext context) {
    return DropdownButtonFormField<PrinterInfo>(
      value: _selected,
      decoration: InputDecoration(labelText: widget.label ?? "Select Printer"),
      items: _printers
          .map((p) => DropdownMenuItem(
                value: p,
                child: Text(p.name),
              ))
          .toList(),
      onChanged: (PrinterInfo? newPrinter) async {
        setState(() => _selected = newPrinter);
        await GWindowsPrint.setSelectedPrinter(newPrinter);
      },
    );
  }
}

class PrinterInfo {
  final String id; // Unique system printer name
  final String name; // Display name
  const PrinterInfo({required this.id, required this.name});

  factory PrinterInfo.fromMap(Map<dynamic, dynamic> map) {
    return PrinterInfo(
      id: map['id'] as String,
      name: map['name'] as String,
    );
  }

  Map<String, dynamic> toJson() => {'id': id, 'name': name};
}

class GWindowsPrint {
  static const MethodChannel _channel = MethodChannel('g_windows_print');
  static const _prefKey = "selected_printer";

  static PrinterInfo? _selectedPrinter;

  Future<String?> getPlatformVersion() {
    return GWindowsPrintPlatform.instance.getPlatformVersion();
  }

  /// Returns all available printers
  static Future<List<PrinterInfo>> getPrinters() async {
    final result = await _channel.invokeMethod<List<dynamic>>('getPrinters');
    return (result ?? [])
        .cast<Map>()
        .map((e) => PrinterInfo.fromMap(e))
        .toList();
  }

  /// Returns the default system printer (from native)
  static Future<PrinterInfo?> pickPrinter() async {
    final result =
        await _channel.invokeMethod<Map<dynamic, dynamic>?>('pickPrinter');
    if (result == null) return null;
    return PrinterInfo.fromMap(result);
  }

  /// Save the chosen printer both in memory and in shared_preferences
  static Future<void> setSelectedPrinter(PrinterInfo? printer) async {
    _selectedPrinter = printer;
    final prefs = await SharedPreferences.getInstance();
    if (printer == null) {
      await prefs.remove(_prefKey);
    } else {
      await prefs.setStringList(_prefKey, [printer.id, printer.name]);
    }
  }

  /// Load saved printer (if exists) from disk
  static Future<PrinterInfo?> loadSavedPrinter() async {
    final prefs = await SharedPreferences.getInstance();
    final list = prefs.getStringList(_prefKey);
    if (list != null && list.length == 2) {
      _selectedPrinter = PrinterInfo(id: list[0], name: list[1]);
      return _selectedPrinter;
    }
    return null;
  }

  static PrinterInfo? get selectedPrinter => _selectedPrinter;

  /// Sends raw PDF bytes to a printer.
  ///
  /// Priority:
  /// 1. If [printerId] is given, use it.
  /// 2. Else, use saved printer (disk/memory).
  /// 3. Else, fallback to Windows print dialog.

  static Future<void> printPdf({
    String? printerId,
    required Uint8List pdfData,
    String jobName = 'Flutter PDF Job',
  }) async {
    String? targetPrinter = printerId ?? _selectedPrinter?.id;

    try {
      if (targetPrinter != null) {
        await _channel.invokeMethod('printPdf', {
          'printerId': targetPrinter,
          'pdfData': pdfData,
          'jobName': jobName,
        });
      } else {
        await _channel.invokeMethod('showPrintDialog', {
          'pdfData': pdfData,
          'jobName': jobName,
        });
      }
    } on PlatformException catch (e) {
      debugPrint('Printing failed: ${e.message}');
      rethrow;
    }
  }
}
