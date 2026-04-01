// Stefan Dubnicka, xdubnis00
import 'package:flutter/material.dart';
import '../../models/mac_system7/upgradeoffer.dart';
import '../../models/mac_system7/upgrade.dart';
import '../../models/mac_system7/clickergamestate.dart';
import 'clickergame.dart';
import '../../repositories/gamerepository.dart';

class UpgradeShop{
  final ClickerGame gameController;
  final GameRepository repository;

  UpgradeShop(this.gameController, this.repository) {
    loadFromApi();
  }

  final ValueNotifier<List<UpgradeOffer>> items = ValueNotifier<List<UpgradeOffer>>(<UpgradeOffer>[]);

  void loadFromApi() async {
    items.value = await repository.fetchShopOffers();
  }

  Future<void> buy(BuildContext context, UpgradeOffer upgradeOffer) async {
    final newState = await repository.buyItem(upgradeOffer.id);
    gameController.state.value = newState;
  }
}