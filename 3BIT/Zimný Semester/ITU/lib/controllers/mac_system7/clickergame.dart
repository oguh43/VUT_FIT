// Stefan Dubnicka, xdubnis00
import 'package:flutter/material.dart';
import 'dart:async';
import '../../models/mac_system7/clickergamestate.dart';
import '../../models/mac_system7/minigame.dart';
import '../../repositories/gamerepository.dart';
import '../../views/mac_system7/mac_system7_cors_minigame.dart';

class ClickerGame{
  final GameRepository repository;

  final ValueNotifier<ClickerGameState> state = ValueNotifier<ClickerGameState>(ClickerGameState.initial());

  ClickerGame(this.repository) {
    loadFromApi();
    Timer.periodic(const Duration(milliseconds: 200), (timer) {
      loadFromApi();
    });
  }

  Future<void> loadFromApi() async {
    final apiState = await repository.fetchGameState();
    state.value = apiState;
  }

  Future<void> onButtonPressed() async {
    final newState = await repository.buttonClick();
    state.value = newState;
  }

  void onNavigateToUpgradeShop(BuildContext context, {String route = '/upgrades'}){
    Navigator.of(context).pushNamed(route);
  }

  void onMiniGameSelected(BuildContext context, MiniGame minigame) {
    if (minigame.id == 'cors') {
      Navigator.of(context).push(
        MaterialPageRoute(builder: (_) => MacSystem7CorsMinigame(repository: repository)),
      );
    } else if (minigame.id == 'winapi'){
      Navigator.of(context).pushNamed('/w11/minigame');
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Open minigame: ${minigame.title}'))
      );
    }
  }
}