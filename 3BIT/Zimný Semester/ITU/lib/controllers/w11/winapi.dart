// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../models/w11/winapimessage.dart';
import '../../models/w11/winapiscreen.dart';
import '../../repositories/gamerepository.dart';

// Controller for the WinAPI minigame: screen/message assignment logic
class WinAPIController {
  // Used to load screens/messages and submit results
  final GameRepository repository;

  // Notifies the UI with the list of WinAPIScreen objects for the current mini-game
  final ValueNotifier<List<WinAPIScreen>> screens = ValueNotifier<List<WinAPIScreen>>([]);
  // Pool of unassigned WinAPIMessage items available for assignment
  final ValueNotifier<List<WinAPIMessage>> messages = ValueNotifier<List<WinAPIMessage>>([]);
  // Map of screenId -> list of assigned WinAPIMessages representing current user placement
  final ValueNotifier<Map<String, List<WinAPIMessage>>> assignments = ValueNotifier<Map<String, List<WinAPIMessage>>>({});

  // When true prevents further edits after results are submitted
  final ValueNotifier<bool> locked = ValueNotifier<bool>(false);
  // Stores server verification results keyed by message id or pair id (true == ok)
  final ValueNotifier<Map<String, bool>> resultsFromServer = ValueNotifier<Map<String, bool>>({});

  WinAPIController(this.repository);

  // Loads screens and messages for the given miniGameId and resets local state
  Future<void> loadForMiniGame(String miniGameId) async {
    try {
      final scr = await repository.fetchScreens(miniGameId);
      final msgs = await repository.fetchMessages(miniGameId);

      screens.value = scr;
      messages.value = msgs;
      assignments.value = {
        for (final s in scr) s.id: <WinAPIMessage>[],
      };
      locked.value = false;
      resultsFromServer.value = {};
    } catch (e) {
      // error handling
    }
  }

  // Remove message from available pool and append to target screen's assignment list
  void assignMessageToScreen(WinAPIMessage msg, WinAPIScreen screen) {
    final remaining = List<WinAPIMessage>.from(messages.value)
      ..removeWhere((m) => m.id == msg.id);
    messages.value = remaining;

    final map = Map<String, List<WinAPIMessage>>.from(assignments.value);
    map[screen.id] = List<WinAPIMessage>.from(map[screen.id] ?? [])..add(msg);
    assignments.value = map;
  }

  // Removes a message from a screen assignment and returns it to the messages pool
  void unassignMessageFromScreen(String screenId, WinAPIMessage msg) {
    final map = Map<String, List<WinAPIMessage>>.from(assignments.value);
    map[screenId] = List<WinAPIMessage>.from(map[screenId] ?? [])
      ..removeWhere((m) => m.id == msg.id);
    assignments.value = map;

    messages.value = List<WinAPIMessage>.from(messages.value)..add(msg);
  }

  // Remove from source list and add to destination list, then update notifier
  void moveMessageBetweenScreen({
    required String from,
    required String to,
    required WinAPIMessage msg,
  }) {
    if (from == to) return;
    final map = Map<String, List<WinAPIMessage>>.from(assignments.value);

    map[from] = List<WinAPIMessage>.from(map[from] ?? [])
      ..removeWhere((m) => m.id == msg.id);
    map[to] = List<WinAPIMessage>.from(map[to] ?? [])..add(msg);

    assignments.value = map;
  }

  // Navigate back to the main menu
  void onBackToMainMenu(BuildContext context){
    Navigator.of(context).pop();
  }

  // Collects current assignments and submits them to the repository, then records server results
  Future<void> submitResults() async {
    final result = <Map<String, dynamic>>[];
    assignments.value.forEach((screenId, msgs) {
      for (final msg in msgs) {
        result.add({
          'message': msg.id,
          'screen': screenId,
        });
      }
    });

    try {
      final response = await repository.submitWinApiResult(result);
      final Map<String, bool> resMap = {};
      for (final r in response) {
        r.forEach((key, value) {
          resMap[key] = value == 'ok';
        });
      }
      resultsFromServer.value = resMap;

      locked.value = true;
    } catch (e) {
      // error handling
    }
  }
}