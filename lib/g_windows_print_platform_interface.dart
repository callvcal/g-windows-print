import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'g_windows_print_method_channel.dart';

abstract class GWindowsPrintPlatform extends PlatformInterface {
  /// Constructs a GWindowsPrintPlatform.
  GWindowsPrintPlatform() : super(token: _token);

  static final Object _token = Object();

  static GWindowsPrintPlatform _instance = MethodChannelGWindowsPrint();

  /// The default instance of [GWindowsPrintPlatform] to use.
  ///
  /// Defaults to [MethodChannelGWindowsPrint].
  static GWindowsPrintPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [GWindowsPrintPlatform] when
  /// they register themselves.
  static set instance(GWindowsPrintPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
