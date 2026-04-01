// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../controllers/w11/winapi.dart';
import '../../models/w11/winapiscreen.dart';
import '../../models/w11/winapimessage.dart';
import '../../models/w11/dragable_payload.dart';
import 'widgets/winapimessage.dart';
import 'widgets/winapiscreencard.dart';

// Mini-game screen for the WinAPI minigame: loads data, shows screens/messages, and submits results
class MiniGameScreen extends StatelessWidget {
  final WinAPIController controller;
  final String miniGameId;

  late final Future<void> _loadFuture;

  MiniGameScreen({
    super.key,
    required this.controller,
    required this.miniGameId,
  }) {
    _loadFuture = controller.loadForMiniGame(miniGameId);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFF3F3F3),
      body: SafeArea(
        top: true,
        bottom: false,
        child: FutureBuilder<void>(
          future: _loadFuture,
          builder: (context, snapshot) {
            // Start async load for the selected mini-game in the constructor
            if (snapshot.connectionState == ConnectionState.waiting &&
                controller.screens.value.isEmpty &&
                controller.messages.value.isEmpty) {
              return const Center(child: CircularProgressIndicator());
            }

            // Error state: display API error text and a Retry button that calls
            if (snapshot.hasError &&
                controller.screens.value.isEmpty &&
                controller.messages.value.isEmpty) {
              return Center(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    const Text(
                      'Failed to load mini-game data.',
                      style: TextStyle(fontSize: 16),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      snapshot.error.toString(),
                      style: const TextStyle(
                        fontSize: 12,
                        color: Colors.black54,
                      ),
                      textAlign: TextAlign.center,
                    ),
                    const SizedBox(height: 16),
                    OutlinedButton(
                      onPressed: () {
                        // re-try
                        controller.loadForMiniGame(miniGameId);
                      },
                      child: const Text('Retry'),
                    ),
                  ],
                ),
              );
            }

            // Main UI header: Back button (returns to main menu) and Submit button (disabled when locked)
            return Column(
              children: [
                Padding(
                  padding: const EdgeInsets.all(12.0),
                  child: Row(
                    children: [
                      FilledButton(
                        style: FilledButton.styleFrom(
                          backgroundColor: Colors.white,
                          overlayColor: const Color(0xFFD6D6D6),
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(8)
                          ),
                          shadowColor: Colors.black12
                        ),
                        onPressed: () => controller.onBackToMainMenu(context),
                        child: const Text('Back to main menu', style: TextStyle(color: Colors.black)),
                      ),
                      const Spacer(),
                      ValueListenableBuilder<bool>(
                        valueListenable: controller.locked,
                        builder: (_, locked, __) => ElevatedButton(
                          style: ElevatedButton.styleFrom(
                            backgroundColor: const Color(0xFF005FB8),
                            shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(8)
                            )
                          ),
                          child: const Text('Submit', style: TextStyle(color: Colors.white)),
                          onPressed: locked ? null : () => controller.submitResults(),
                        ),
                      ),
                    ]
                  ),
                ),
                // Top area: Wrap of WinAPIScreenCard widgets; each card receives assigned messages and drag callbacks
                Expanded(
                  child: Container(
                    decoration: BoxDecoration(
                      color: Colors.white,
                      borderRadius: const BorderRadius.vertical(
                        top: Radius.circular(8)
                      ),
                      border: Border.all(color: Colors.black12),
                      boxShadow: const [
                        BoxShadow(
                          color: Colors.black12,
                          blurRadius: 8,
                          offset: Offset(0, 2)
                        )
                      ]
                    ),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.stretch,
                      children: [
                        Expanded(
                          child: Padding(
                            padding: const EdgeInsets.all(12.0),
                            child: ValueListenableBuilder<List<WinAPIScreen>>(
                              valueListenable: controller.screens,
                              builder: (_, screens, __) {
                                if (screens.isEmpty) {
                                  return const Center(
                                    child: Text(
                                      'No screens loaded.',
                                      style: TextStyle(fontSize: 14),
                                    ),
                                  );
                                }

                                return ValueListenableBuilder<bool>(
                                  valueListenable: controller.locked,
                                  builder: (_, locked, __){
                                    return Wrap(
                                      alignment: WrapAlignment.center,
                                      runAlignment: WrapAlignment.start,
                                      spacing: 32,
                                      runSpacing: 24,
                                      children: [
                                        for (final s in screens)
                                          ValueListenableBuilder<
                                              Map<String, List<WinAPIMessage>>>(
                                            valueListenable: controller.assignments,
                                            builder: (_, map, __) => WinAPIScreenCard(
                                              screen: s,
                                              assigned: map[s.id] ?? const [],
                                              locked: locked,
                                              resultsFromServer: controller.resultsFromServer.value,
                                              onAssignFromPool: (msg) => controller.assignMessageToScreen(msg, s),
                                              onMoveFromScreen: (fromId, msg) => controller.moveMessageBetweenScreen(from: fromId, to: s.id, msg: msg),
                                              onUnassign: (msg) => controller.unassignMessageFromScreen(s.id, msg)
                                            ),
                                          ),
                                      ],
                                    );
                                  }
                                );
                              },
                            ),
                          ),
                        ),
                        const SizedBox(height: 8),
                        // Bottom area: horizontal draggable pool of WinAPIMessageCards, supports dragging to screens and accepting drags back
                        _BottomMessages(controller: controller),
                        const SizedBox(height: 12),
                      ],
                    )
                  )
                ),
              ],
            );
          },
        ),
      ),
    );
  }
}

// Bottom message pool
class _BottomMessages extends StatelessWidget {
  final WinAPIController controller;

  const _BottomMessages({required this.controller});

  @override
  Widget build(BuildContext context) {
    return DragTarget<DragablePayload>(
      onWillAccept: (_) => true,
      onAccept: (payload) {
        // Pop back to pool if coming from a screen card
        if (payload.fromScreenId != null) {
          controller.unassignMessageFromScreen(payload.fromScreenId!, payload.message);
        }
      },
      builder: (context, candidate, rejected) {
        return ValueListenableBuilder<List<WinAPIMessage>>(
          valueListenable: controller.messages,
          builder: (_, items, __) {
            if (items.isEmpty) {
              return const Align(
                alignment: Alignment.centerLeft,
                child: Text('No messages left to assign.',
                    style: TextStyle(fontSize: 12, color: Colors.black54)),
              );
            }

            // Content centering, allowing scrolling if content is wider than screen
            return LayoutBuilder(
              builder: (context, constraints) {
                return SingleChildScrollView(
                  scrollDirection: Axis.horizontal,
                  child: ConstrainedBox(
                    constraints:
                        BoxConstraints(minWidth: constraints.maxWidth),
                    child: Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        for (final m in items)
                          Padding(
                            padding: const EdgeInsets.only(right: 24.0),
                            child: Draggable<DragablePayload>(
                              data: DragablePayload(message: m, fromScreenId: null),
                              feedback: Material(
                                  type: MaterialType.transparency,
                                  child: WinAPIMessageCard(message: m, ghost: true),
                              ),
                              childWhenDragging: Opacity(
                                opacity: 0.4,
                                child: WinAPIMessageCard(message: m),
                              ),
                              child: WinAPIMessageCard(message: m),
                            ),
                          ),
                      ],
                    ),
                  ),
                );
              },
            );
          },
        );
      },
    );
  }
}
