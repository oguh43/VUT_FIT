// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import 'dart:async';
import 'dart:convert';
import '../../services/api_client.dart';
import '../../models/mac_system7/upgradeoffer.dart';
import '../../models/w11/upgrade.dart';
import '../../models/mac_system7/clickergamestate.dart';
import 'clickergame.dart';
import '../../repositories/gamerepository.dart';

// Controller for the upgrade shop (the W11 deign-like one)
class W11UpgradeShop{
  // Reference to the parent game controller
  final W11ClickerGame gameController;
  // Used to fetch shop offers and perform purchase requests
  final GameRepository repository;

  W11UpgradeShop(this.gameController, this.repository) {
    loadFromApi();
  }

  // Represents the current list of shop offers and notifies listeners in views about any change
  final ValueNotifier<List<UpgradeOffer>> items = ValueNotifier<List<UpgradeOffer>>(<UpgradeOffer>[]);

  // Transient error message shown when a purchase fails (cleared automatically after a timeout)
  final ValueNotifier<String?> errorMessage = ValueNotifier<String?>(null);
  // D of the offer that caused the last error — used for highlighting the failed item in the UI
  final ValueNotifier<String?> errorOfferId = ValueNotifier<String?>(null);

  void loadFromApi() async {
    items.value = await repository.fetchShopOffers();
  }

  // Attempts to buy the given upgradeOffer, on success clears any existing error, on failure parses the API error (if present), sets errorMessage and errorOfferId, then clears them after 3 seconds
  Future<void> buy(BuildContext context, UpgradeOffer upgradeOffer) async {
    try {
      final newState = await repository.buyItem(upgradeOffer.id);
      errorMessage.value = null;
      errorOfferId.value = null;
    } catch (e) {
      String msg = 'Failed to buy item';
      if (e is ApiException) {
        try {
          final parsed = jsonDecode(e.body);
          if (parsed is Map && parsed['error'] != null) {
            msg = parsed['error'].toString();
          } else {
            msg = e.body;
          }
        } catch (_) {
          msg = e.body;
        }
      } else {
        msg = e.toString();
      }
      errorMessage.value = msg;
      errorOfferId.value = upgradeOffer.id;
      Timer(const Duration(seconds: 3), () {
        if (errorOfferId.value == upgradeOffer.id) {
          errorMessage.value = null;
          errorOfferId.value = null;
        }
      });
    }
  }

  // Navigate back to the main screen
  void onBackToMainMenu(BuildContext context){
    Navigator.of(context).pop();
  }
}