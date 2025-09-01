import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'g_windows_print_platform_interface.dart';

/// An implementation of [GWindowsPrintPlatform] that uses method channels.
class MethodChannelGWindowsPrint extends GWindowsPrintPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('g_windows_print');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
