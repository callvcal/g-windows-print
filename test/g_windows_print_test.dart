import 'package:flutter_test/flutter_test.dart';
import 'package:g_windows_print/g_windows_print.dart';
import 'package:g_windows_print/g_windows_print_platform_interface.dart';
import 'package:g_windows_print/g_windows_print_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockGWindowsPrintPlatform
    with MockPlatformInterfaceMixin
    implements GWindowsPrintPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final GWindowsPrintPlatform initialPlatform = GWindowsPrintPlatform.instance;

  test('$MethodChannelGWindowsPrint is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelGWindowsPrint>());
  });

  test('getPlatformVersion', () async {
    GWindowsPrint gWindowsPrintPlugin = GWindowsPrint();
    MockGWindowsPrintPlatform fakePlatform = MockGWindowsPrintPlatform();
    GWindowsPrintPlatform.instance = fakePlatform;

    expect(await gWindowsPrintPlugin.getPlatformVersion(), '42');
  });
}
