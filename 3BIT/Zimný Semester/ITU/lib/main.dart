// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:ux_clicker/controllers/w11/winapi.dart';
import 'package:ux_clicker/views/w11/winapiscreen.dart';
import 'services/api_client.dart';
import 'repositories/gamerepository.dart';
import 'controllers/mac_system7/clickergame.dart';
import 'controllers/mac_system7/upgradeshop.dart';
import 'controllers/w11/clickergame.dart';
import 'controllers/w11/upgradeshop.dart';
import 'views/w11/mainscreen.dart';
import 'views/w11/upgradeshopscreen.dart';
import 'views/mac_system7/mac_system7_mainscreen.dart';
import 'views/mac_system7/mac_system7_upgrade_shop_screen.dart';
import 'views/tui/tui_screen.dart';
import 'views/tui/react_minigame.dart';
import 'models/tui/game_state.dart' as tui;

void main() {
  final apiClient = ApiClient(baseUrl: 'http://localhost:16808');
  final gameRepository = GameRepository(apiClient);

  final controller = ClickerGame(gameRepository);
  final W11controller = W11ClickerGame(gameRepository);
  runApp(
    UXClicker(
      controller: controller,
      w11controller: W11controller,
      gameRepository: gameRepository,
    ),
  );
}

class UXClicker extends StatelessWidget {
  final ClickerGame controller;
  final W11ClickerGame w11controller;
  final GameRepository gameRepository;

  const UXClicker({
    super.key,
    required this.controller,
    required this.w11controller,
    required this.gameRepository,
  });
  // turn off the debug banner

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'UX Clicker',
      debugShowCheckedModeBanner: false,
      routes: {
        '/': (_) => ChangeNotifierProvider(
          create: (_) => tui.GameState(gameRepository),
          child: TUIScreen(repository: gameRepository),
        ),
        '/mac': (_) => MacSystem7MainScreen(controller: controller),
        '/mac/upgrades': (_) => MacSystem7UpgradeShopScreen(
          controller: UpgradeShop(controller, controller.repository),
        ),
        '/mac/react': (_) => ReactMinigame(repository: gameRepository),
        '/w11': (_) => MainScreen(controller: w11controller),
        '/w11/upgrades': (_) => UpgradeShopScreen(
          controller: W11UpgradeShop(w11controller, w11controller.repository),
        ),
        '/w11/minigame': (_) => MiniGameScreen(
          controller: WinAPIController(w11controller.repository),
          miniGameId: 'winapi',
        ),
        '/w11/react': (_) => ReactMinigame(repository: gameRepository),
        '/tui/react': (_) => ReactMinigame(repository: gameRepository),
      },
      initialRoute: '/',
    );
  }
}
