// Stefan Dubnicka, xdubnis00
import '../services/api_client.dart';
import '../models/mac_system7/clickergamestate.dart';
import '../models/mac_system7/minigame.dart';
import '../models/mac_system7/upgrade.dart';
import '../models/mac_system7/upgradeoffer.dart';
import '../models/w11/winapimessage.dart';
import '../models/w11/winapiscreen.dart';

class GameRepository {
  final ApiClient api;

  GameRepository(this.api);

  Future<ClickerGameState> fetchGameState() async {
    final data = await api.get('/game/state');
    return ClickerGameState(
      availableUxPoints: data['availableUxPoints'],
      productionUxPointsPerSecond: (data['productionUxPointsPerSecond'] as num)
          .toDouble(),
      bonusUxPointsPerSecond: (data['bonusUxPointsPerSecond'] as num)
          .toDouble(),
      upgradesList: (data['upgradesList'] as List)
          .map(
            (u) => Upgrade(
              name: u['name'],
              count: u['count'],
              uxPointsPerSecond: (u['uxPointsPerSecond'] as num).toDouble(),
            ),
          )
          .toList(),
      availableMiniGames: (data['availableMiniGames'] as List)
          .map((m) => MiniGame(id: m['id'], title: m['title']))
          .toList(),
    );
  }

  Future<ClickerGameState> buttonClick() async {
    final data = await api.post('/game/design-click');
    return ClickerGameState(
      availableUxPoints: data['availableUxPoints'],
      productionUxPointsPerSecond: (data['productionUxPointsPerSecond'] as num)
          .toDouble(),
      bonusUxPointsPerSecond: (data['bonusUxPointsPerSecond'] as num)
          .toDouble(),
      upgradesList: (data['upgradesList'] as List)
          .map(
            (u) => Upgrade(
              name: u['name'],
              count: u['count'],
              uxPointsPerSecond: (u['uxPointsPerSecond'] as num).toDouble(),
            ),
          )
          .toList(),
      availableMiniGames: (data['availableMiniGames'] as List)
          .map((m) => MiniGame(id: m['id'], title: m['title']))
          .toList(),
    );
  }

  Future<List<UpgradeOffer>> fetchShopOffers() async {
    final data = await api.get('/shop/items');
    return (data as List)
        .map(
          (o) => UpgradeOffer(
            id: o['id'],
            name: o['name'],
            cost: o['cost'],
            productionUxPointsPerSecond:
                (o['productionUxPointsPerSecond'] as num).toDouble(),
          ),
        )
        .toList();
  }

  Future<ClickerGameState> buyItem(String itemId) async {
    final data = await api.post('/shop/buy', body: {'itemId': itemId});
    return ClickerGameState(
      availableUxPoints: data['availableUxPoints'],
      productionUxPointsPerSecond: (data['productionUxPointsPerSecond'] as num)
          .toDouble(),
      bonusUxPointsPerSecond: (data['bonusUxPointsPerSecond'] as num)
          .toDouble(),
      upgradesList: (data['upgradesList'] as List)
          .map(
            (u) => Upgrade(
              name: u['name'],
              count: u['count'],
              uxPointsPerSecond: (u['uxPointsPerSecond'] as num).toDouble(),
            ),
          )
          .toList(),
      availableMiniGames: (data['availableMiniGames'] as List)
          .map((m) => MiniGame(id: m['id'], title: m['title']))
          .toList(),
    );
  }

  Future<ClickerGameState> completeMinigame(double multiplier) async {
    final data = await api.post(
      '/game/minigame-complete',
      body: {'multiplier': multiplier},
    );
    return ClickerGameState(
      availableUxPoints: data['availableUxPoints'],
      productionUxPointsPerSecond: (data['productionUxPointsPerSecond'] as num)
          .toDouble(),
      bonusUxPointsPerSecond: (data['bonusUxPointsPerSecond'] as num)
          .toDouble(),
      upgradesList: (data['upgradesList'] as List)
          .map(
            (u) => Upgrade(
              name: u['name'],
              count: u['count'],
              uxPointsPerSecond: (u['uxPointsPerSecond'] as num).toDouble(),
            ),
          )
          .toList(),
      availableMiniGames: (data['availableMiniGames'] as List)
          .map((m) => MiniGame(id: m['id'], title: m['title']))
          .toList(),
    );
  }

  Future<List<WinAPIScreen>> fetchScreens(String miniGameId) async {
    final data = await api.get('/minigame/$miniGameId/screens');
    return (data as List<dynamic>)
        .map(
          (s) => WinAPIScreen(
            id: s['id'],
            title: s['title'],
            stateText: s['state'],
          ),
        )
        .toList();
  }

  Future<List<WinAPIMessage>> fetchMessages(String miniGameId) async {
    final data = await api.get('/minigame/$miniGameId/messages');
    return (data as List<dynamic>)
        .map(
          (m) => WinAPIMessage(
            id: m['id'],
            title: m['title'],
            subtitle: m['subtitle'],
          ),
        )
        .toList();
  }

  Future<List<Map<String, dynamic>>> submitWinApiResult(
    List<Map<String, dynamic>> result,
  ) async {
    final data = await api.post('/minigame/winapi', body: {'result': result});
    return (data as List<dynamic>).cast<Map<String, dynamic>>();
  }

  Future<ClickerGameState> setCurrentUI(String ui) async {
    final data = await api.post('/game/set-ui', body: {'ui': ui});
    return ClickerGameState(
      availableUxPoints: data['availableUxPoints'],
      productionUxPointsPerSecond: (data['productionUxPointsPerSecond'] as num)
          .toDouble(),
      bonusUxPointsPerSecond: (data['bonusUxPointsPerSecond'] as num)
          .toDouble(),
      upgradesList: (data['upgradesList'] as List)
          .map(
            (u) => Upgrade(
              name: u['name'],
              count: u['count'],
              uxPointsPerSecond: (u['uxPointsPerSecond'] as num).toDouble(),
            ),
          )
          .toList(),
      availableMiniGames: (data['availableMiniGames'] as List)
          .map((m) => MiniGame(id: m['id'], title: m['title']))
          .toList(),
    );
  }
}
