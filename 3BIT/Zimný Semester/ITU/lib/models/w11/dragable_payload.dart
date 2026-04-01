// Filip Jenis, xjenisf00

import 'winapimessage.dart';

class DragablePayload {
  final WinAPIMessage message;
  final String? fromScreenId;

  const DragablePayload({
    required this.message,
    this.fromScreenId,
  });
}