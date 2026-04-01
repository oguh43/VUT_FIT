// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../../models/w11/winapiscreen.dart';
import '../../../models/w11/winapimessage.dart';
import '../../../models/w11/dragable_payload.dart';

// Card widget representing one WinAPI screen, manages display of assigned messages, drag/drop behaviour and visual result state
class WinAPIScreenCard extends StatelessWidget {
  final WinAPIScreen screen;
  final List<WinAPIMessage> assigned;
  final bool locked;
  final Map<String, bool> resultsFromServer;

  // Called when a message is dragged from the pool onto this screen
  final void Function(WinAPIMessage msg) onAssignFromPool;

  // Called when a message is dragged from another screen onto this screen
  final void Function(String fromScreenId, WinAPIMessage msg) onMoveFromScreen;

  // Called to unassign a message (close icon)
  final void Function(WinAPIMessage msg)? onUnassign;

  const WinAPIScreenCard({
    super.key,
    required this.screen,
    required this.assigned,
    required this.onAssignFromPool,
    required this.onMoveFromScreen,
    required this.locked,
    required this.resultsFromServer,
    this.onUnassign,
  });

  @override
  Widget build(BuildContext context) {
    return DragTarget<DragablePayload>(
      onWillAccept: (_) => true,
      onAccept: (payload) {
        if (payload.fromScreenId == null) {
          onAssignFromPool(payload.message);
        } else if (payload.fromScreenId != screen.id) {
          onMoveFromScreen(payload.fromScreenId!, payload.message);
        }
        // if fromScreenId == screen.id -> nič, zostáva na mieste
      },
      builder: (context, candidate, rejected) {
        final highlight = candidate.isNotEmpty;
        return AnimatedContainer(
          duration: const Duration(milliseconds: 120),
          width: 220,
          padding: const EdgeInsets.all(12),
          decoration: BoxDecoration(
            color: Colors.white,
            borderRadius: BorderRadius.circular(8),
            boxShadow: [
              BoxShadow(
                color: highlight ? Colors.blue.withOpacity(0.25) : Colors.black12,
                blurRadius: 16,
                offset: const Offset(0, 8),
              ),
            ],
            border: Border.all(
              color: highlight ? Colors.blue : Colors.transparent,
              width: 1.2,
            ),
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(screen.title,
                  style: const TextStyle(fontWeight: FontWeight.w600, fontSize: 16)),
              const SizedBox(height: 6),
              Text('State: ${screen.stateText}', style: const TextStyle(fontSize: 12)),
              if (assigned.isNotEmpty) ...[
                const SizedBox(height: 12),
                Wrap(
                  spacing: 6,
                  runSpacing: -6,
                  children: [
                    for (final msg in assigned)
                      if (locked)
                        _Chip(message: msg, locked: locked, resultsFromServer: resultsFromServer[msg.id] ?? false,)
                      else
                        LongPressDraggable<DragablePayload>(
                          data: DragablePayload(message: msg, fromScreenId: screen.id),
                          feedback: Material(
                            type: MaterialType.transparency,
                            child: _Chip(message: msg, ghost: true),
                          ),
                          childWhenDragging: Opacity(opacity: 0.4, child: _Chip(message: msg)),
                          child: _Chip(
                            message: msg,
                            onRemove: onUnassign == null ? null : () => onUnassign!(msg),
                          ),
                        ),
                  ],
                ),
              ],
            ],
          ),
        );
      },
    );
  }
}

// Small chip used to represent a WinAPIMessage
class _Chip extends StatelessWidget {
  final WinAPIMessage message;
  final bool ghost;
  final bool locked;
  final bool resultsFromServer;
  final VoidCallback? onRemove;

  const _Chip({required this.message, this.ghost = false, this.onRemove, this.locked = false, this.resultsFromServer = false});

  @override
  Widget build(BuildContext context) {
    final chip = Chip(
      label: Text(message.title, style: const TextStyle(fontSize: 12)),
      backgroundColor: locked ? resultsFromServer ? const Color(0xFFDFF6DD) : const Color(0xFFFDE7E9) : null,
      visualDensity: VisualDensity.compact,
      deleteIcon: onRemove != null ? const Icon(Icons.close, size: 16) : null,
      onDeleted: onRemove,
    );
    return ghost ? Opacity(opacity: .9, child: chip) : chip;
  }
}
