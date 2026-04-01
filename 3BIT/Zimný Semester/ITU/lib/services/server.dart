// Stefan Dubnicka, xdubnis00

// dart lib/services/server.dart

import 'dart:io';
import 'dart:convert';
import 'package:shelf/shelf.dart';
import 'package:shelf/shelf_io.dart' as shelf_io;
import 'package:shelf_router/shelf_router.dart';
import 'dart:async';

List<Map<String, dynamic>> upgradesList = [];

Map<String, dynamic> gameState = {
  "availableUxPoints": 0,
  "productionUxPointsPerSecond": 0,
  "bonusUxPointsPerSecond": 0,
  "currentUI": "tui", // mac_system7, win11, tui
  "upgradesList": upgradesList,
  "availableMiniGames": [
    {"id": "winapi", "title": "WinAPI"},
    {"id": "cors", "title": "AJAX CORS Challenge"},
  ],
};

List<Map<String, dynamic>> tuiShopOffers = [
  {"id": "vim", "name": "Vim", "cost": 20, "productionUxPointsPerSecond": 1},
  {"id": "tmux", "name": "Tmux", "cost": 100, "productionUxPointsPerSecond": 5},
  {
    "id": "zsh",
    "name": "Zsh Shell",
    "cost": 400,
    "productionUxPointsPerSecond": 20,
  },
  {
    "id": "emacs",
    "name": "Emacs",
    "cost": 1000,
    "productionUxPointsPerSecond": 50,
  },
  {
    "id": "neovim",
    "name": "Neovim",
    "cost": 2000,
    "productionUxPointsPerSecond": 100,
  },
];

List<Map<String, dynamic>> shopOffers = [
  {
    "id": "0bit",
    "name": "0BIT",
    "cost": 2000,
    "productionUxPointsPerSecond": 100,
  },
  {
    "id": "bachelor",
    "name": "Bachelor",
    "cost": 10000,
    "productionUxPointsPerSecond": 500,
  },
  {
    "id": "muni_fi",
    "name": "MUNI FI",
    "cost": 40000,
    "productionUxPointsPerSecond": 2000,
  },
  {
    "id": "2mit",
    "name": "2MIT",
    "cost": 100000,
    "productionUxPointsPerSecond": 5000,
  },
  {
    "id": "phd",
    "name": "PhD",
    "cost": 200000,
    "productionUxPointsPerSecond": 10000,
  },
];

List<Map<String, dynamic>> win11ShopOffers = [
  {
    "id": "explorer",
    "name": "Explorer.exe",
    "cost": 200000,
    "productionUxPointsPerSecond": 10000,
  },
  {
    "id": "edge",
    "name": "Edge Browser",
    "cost": 500000,
    "productionUxPointsPerSecond": 25000,
  },
  {
    "id": "teams",
    "name": "MS Teams",
    "cost": 1000000,
    "productionUxPointsPerSecond": 50000,
  },
  {
    "id": "vscode",
    "name": "VS Code",
    "cost": 1500000,
    "productionUxPointsPerSecond": 75000,
  },
  {
    "id": "wsl",
    "name": "WSL 2",
    "cost": 2000000,
    "productionUxPointsPerSecond": 100000,
  },
];

List<Map<String, dynamic>> winApiMessages = [
  {
    "id": "lbtn",
    "title": "WM_LBUTTONDOWN",
    "subtitle": "Click on 'Buy upgrade' button",
  },
  {"id": "paint", "title": "WM_PAINT", "subtitle": ""},
  {"id": "command", "title": "WM_COMMAND", "subtitle": "Button: Save Settings"},
];
List<Map<String, dynamic>> winApiScreens = [
  {"id": "main", "title": "MainWindow", "state": "OK"},
  {"id": "shop", "title": "ShopWindow", "state": "Waiting for repaint"},
  {"id": "stats", "title": "StatsWindow", "state": "OK"},
  {"id": "settings", "title": "SettingsWindow", "state": "OK"},
];

double _availableUxPointsDouble = 0.0;

Response _json(data) => Response.ok(
  jsonEncode(data),
  headers: {'Content-Type': 'application/json'},
);

void _recalculateProduction() {
  gameState["productionUxPointsPerSecond"] = upgradesList.fold(
    0,
    (sum, u) => sum + (u["count"] as int) * (u["uxPointsPerSecond"] as int),
  );
}

void _startBonusDecayTimer() {
  Timer.periodic(const Duration(seconds: 1), (timer) {
    final currentBonus = (gameState["bonusUxPointsPerSecond"] as num)
        .toDouble();
    if (currentBonus > 0) {
      // Decay by 1% per second
      gameState["bonusUxPointsPerSecond"] = (currentBonus - 0.01).clamp(
        0.0,
        double.infinity,
      );
    }
  });
}

void main() async {
  final router = Router();

  // Start the bonus decay timer
  _startBonusDecayTimer();

  // GET /game/state
  router.get('/game/state', (Request request) {
    return _json(gameState);
  });

  // POST /game/design-click
  router.post('/game/design-click', (Request request) async {
    // Give UX based on 0.1 (10%) of passive income per second
    final baseProduction = (gameState["productionUxPointsPerSecond"] as int)
        .toDouble();
    final bonusMultiplier = (gameState["bonusUxPointsPerSecond"] as num)
        .toDouble();
    final totalProduction = baseProduction * (1.0 + bonusMultiplier);

    // Give 0.1 of passive production per click (minimum 1)
    final clickValue = (totalProduction * 0.1)
        .round()
        .clamp(1, double.infinity)
        .toInt();

    gameState["availableUxPoints"] =
        (gameState["availableUxPoints"] as int) + clickValue;
    return _json(gameState);
  });

  // POST /game/set-ui
  router.post('/game/set-ui', (Request request) async {
    final body = await request.readAsString();
    final data = jsonDecode(body);
    final ui = data['ui'];
    if (ui == 'mac_system7' || ui == 'win11' || ui == 'tui') {
      gameState["currentUI"] = ui;
      return _json(gameState);
    } else {
      return Response(
        400,
        body: jsonEncode({"error": "Invalid UI type"}),
        headers: {'Content-Type': 'application/json'},
      );
    }
  });

  // GET /shop/items
  router.get('/shop/items', (Request request) {
    final currentUI = gameState["currentUI"];
    List<Map<String, dynamic>> offers;

    switch (currentUI) {
      case "win11":
        offers = win11ShopOffers;
        break;
      case "tui":
        offers = tuiShopOffers;
        break;
      case "mac_system7":
      default:
        offers = shopOffers;
        break;
    }

    return _json(offers);
  });

  // POST /shop/buy
  router.post('/shop/buy', (Request request) async {
    final body = await request.readAsString();
    final data = jsonDecode(body);
    final itemId = data['itemId'];

    // Get the appropriate shop offers based on current UI
    final currentUI = gameState["currentUI"];
    List<Map<String, dynamic>> currentShopOffers;

    switch (currentUI) {
      case "win11":
        currentShopOffers = win11ShopOffers;
        break;
      case "tui":
        currentShopOffers = tuiShopOffers;
        break;
      case "mac_system7":
      default:
        currentShopOffers = shopOffers;
        break;
    }

    final offer = currentShopOffers.firstWhere(
      (o) => o['id'] == itemId,
      orElse: () => {},
    );

    if ((gameState["availableUxPoints"] as int) >= offer["cost"]) {
      gameState["availableUxPoints"] =
          (gameState["availableUxPoints"] as int) - offer["cost"];
      // check if exists in upgradesList
      final existingUpgrade = upgradesList.firstWhere(
        (u) => u["name"] == offer["name"],
        orElse: () => {},
      );
      if (existingUpgrade.isNotEmpty) {
        existingUpgrade["count"] = existingUpgrade["count"] + 1;
      } else {
        upgradesList.add({
          "name": offer["name"],
          "count": 1,
          "uxPointsPerSecond": offer["productionUxPointsPerSecond"],
        });
      }
      _recalculateProduction();
      return _json(gameState);
    } else {
      return Response(
        400,
        body: jsonEncode({"error": "Not enough points"}),
        headers: {'Content-Type': 'application/json'},
      );
    }
  });

  // POST /game/minigame-complete
  router.post('/game/minigame-complete', (Request request) async {
    final body = await request.readAsString();
    final data = jsonDecode(body);
    final multiplier = (data['multiplier'] as num).toDouble();

    // Set bonus as a percentage multiplier (e.g., 0.5 = 50% bonus)
    final bonusPercentage = multiplier * 0.5; // 50% bonus per 1.0 multiplier
    gameState["bonusUxPointsPerSecond"] =
        (gameState["bonusUxPointsPerSecond"] as num).toDouble() +
        bonusPercentage;

    print(
      "Multiplier: $multiplier, New bonus percentage: ${gameState['bonusUxPointsPerSecond']}",
    );

    return _json(gameState);
  });

  // GET /minigame/winapi/messages
  router.get('/minigame/winapi/messages', (Request request) {
    return _json(winApiMessages);
  });
  // GET /minigame/winapi/screens
  router.get('/minigame/winapi/screens', (Request request) {
    return _json(winApiScreens);
  });

  // POST /minigame/winapi
  router.post('/minigame/winapi', (Request request) async {
    final body = await request.readAsString();
    final data = jsonDecode(body);
    final result = data['result'];
    var correct = 0;
    List<Map<String, dynamic>> results = [];
    for (final item in result) {
      if (item["message"] == "lbtn") {
        if (item["screen"] == "shop") {
          correct++;
          results.add({item["message"]: "ok"});
        } else {
          results.add({item["message"]: "wrong"});
        }
      }
      if (item["message"] == "paint") {
        if (item["screen"] == "shop") {
          correct++;
          results.add({item["message"]: "ok"});
        } else {
          results.add({item["message"]: "wrong"});
        }
      }
      if (item["message"] == "command") {
        if (item["screen"] == "settings") {
          correct++;
          results.add({item["message"]: "ok"});
        } else {
          results.add({item["message"]: "wrong"});
        }
      }
    }
    if (correct == 3) {
      gameState["bonusUxPointsPerSecond"] = 0.5; // 50% bonus
    } else if (correct == 2) {
      gameState["bonusUxPointsPerSecond"] = 0.3; // 30% bonus
    } else if (correct == 1) {
      gameState["bonusUxPointsPerSecond"] = 0.1; // 10% bonus
    }
    Timer(const Duration(minutes: 3), () {
      gameState["bonusUxPointsPerSecond"] = 0.0;
    });
    return _json(results);
  });

  final handler = router.call;

  final server = await shelf_io.serve(handler, InternetAddress.anyIPv4, 16808);
  print('Server listening on port ${server.port}');

  Timer.periodic(const Duration(milliseconds: 200), (timer) {
    final baseProduction = (gameState["productionUxPointsPerSecond"] as int)
        .toDouble();
    final bonusMultiplier = (gameState["bonusUxPointsPerSecond"] as num)
        .toDouble();

    // Apply bonus as a percentage multiplier (1.0 + bonus)
    final totalProduction = baseProduction * (1.0 + bonusMultiplier);

    double uxToAdd = totalProduction / 5.0;
    _availableUxPointsDouble += uxToAdd;
    int uxInt = _availableUxPointsDouble.floor();
    if (uxInt > 0) {
      gameState["availableUxPoints"] =
          (gameState["availableUxPoints"] as int) + uxInt;
      _availableUxPointsDouble -= uxInt;
    }
  });
}
