// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';

class WindowStateManager {
  // Singleton instance
  static final WindowStateManager _instance = WindowStateManager._internal();
  factory WindowStateManager() => _instance;
  WindowStateManager._internal();

  // Window z-order (last item is on top)
  List<String> _windowZOrder = [
    'upgrades',
    'ux_counter',
    'design_button',
    'upgrade_shop',
    'tech',
  ];

  // Track which windows are hidden
  final Map<String, bool> _hiddenWindows = {
    'upgrades': false,
    'ux_counter': false,
    'design_button': false,
    'upgrade_shop': false,
    'tech': false,
  };

  // Track window positions (x, y)
  final Map<String, Offset?> _windowPositions = {
    'upgrades': null,
    'ux_counter': null,
    'design_button': null,
    'upgrade_shop': null,
    'tech': null,
  };

  // Getters
  List<String> get windowZOrder => List.from(_windowZOrder);
  Map<String, bool> get hiddenWindows => Map.from(_hiddenWindows);
  Map<String, Offset?> get windowPositions => Map.from(_windowPositions);

  // Update methods
  void updateWindowPosition(String windowId, double x, double y) {
    _windowPositions[windowId] = Offset(x, y);
  }

  void bringToFront(String windowId) {
    _windowZOrder.remove(windowId);
    _windowZOrder.add(windowId);
  }

  void hideWindow(String windowId) {
    _hiddenWindows[windowId] = true;
  }

  void showWindow(String windowId) {
    _hiddenWindows[windowId] = false;
    bringToFront(windowId);
  }

  bool isWindowHidden(String windowId) {
    return _hiddenWindows[windowId] ?? false;
  }

  Offset? getWindowPosition(String windowId) {
    return _windowPositions[windowId];
  }

  // Reset all state to defaults (useful for testing or reset functionality)
  void reset() {
    _windowZOrder = [
      'upgrades',
      'ux_counter',
      'design_button',
      'upgrade_shop',
      'tech',
    ];
    _hiddenWindows.updateAll((key, value) => false);
    _windowPositions.updateAll((key, value) => null);
  }
}
