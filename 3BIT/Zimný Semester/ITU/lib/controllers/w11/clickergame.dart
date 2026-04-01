// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../models/mac_system7/clickergamestate.dart';
import '../../models/mac_system7/minigame.dart';
import '../../repositories/gamerepository.dart';
import '../../views/mac_system7/mac_system7_cors_minigame.dart';

// Controller for the whole clicker game (the W11 deign-like one)
class W11ClickerGame{
  // Used to fetch and update the local game state
  final GameRepository repository;

  // Represents the local game states and notifies listeners in views about any change
  final ValueNotifier<ClickerGameState> state = ValueNotifier<ClickerGameState>(ClickerGameState.initial());  

  // Creates the controller and fetches data from backend
  W11ClickerGame(this.repository) {
    loadFromApi();
  }

  Future<void> loadFromApi() async {
    final apiState = await repository.fetchGameState();
    state.value = apiState;
  }

  // Main clicker-game button pressed
  Future<void> onButtonPressed() async {
    final newState = await repository.buttonClick();
    state.value = newState;
  }

  // Upgrades button pressed
  void onNavigateToUpgradeShop(BuildContext context){
    Navigator.of(context).pushNamed('/w11/upgrades');
  }

  // Handles navigation to the selected minigame
  void onMiniGameSelected(BuildContext context, MiniGame minigame) {
    if (minigame.id == 'cors') {
      Navigator.of(context).push(
        MaterialPageRoute(builder: (_) => MacSystem7CorsMinigame(repository: repository)),
      );
    } else if (minigame.id == 'winapi'){
      Navigator.of(context).pushNamed('/w11/minigame');
    }
  }
}