// Filip Jenis, xjenisf00

import 'minigame.dart';
import 'upgrade.dart';

class ClickerGameState {
  final int availableUxPoints;
  final double productionUxPointsPerSecond;
  final double bonusUxPointsPerSecond;
  final List<MiniGame> availableMiniGames;
  final List<Upgrade> upgradesList;

  const ClickerGameState({
    required this.availableUxPoints,
    required this.productionUxPointsPerSecond,
    this.bonusUxPointsPerSecond = 0,
    required this.availableMiniGames,
    required this.upgradesList
  });

  factory ClickerGameState.initial() {
    return ClickerGameState(
      availableUxPoints: 10,
      productionUxPointsPerSecond: 2,
      bonusUxPointsPerSecond: 0,
      availableMiniGames: const [
        MiniGame(id: 'winapi', title: 'WinApi message lab')
      ],
      upgradesList: const [
        Upgrade(count: 10, name: '0BIT', uxPointsPerSecond: 5)
      ]
    );
  }

    ClickerGameState copyWith({
      int? availableUxPoints,
      double? productionUxPointsPerSecond,
      double? bonusUxPointsPerSecond,
      List<MiniGame>? availableMiniGames,
      List<Upgrade>? upgradesList
    }) {
      return ClickerGameState(
        availableUxPoints: availableUxPoints ?? this.availableUxPoints,
        productionUxPointsPerSecond:
            productionUxPointsPerSecond ?? this.productionUxPointsPerSecond,
        bonusUxPointsPerSecond: bonusUxPointsPerSecond ?? this.bonusUxPointsPerSecond,
        availableMiniGames: availableMiniGames ?? this.availableMiniGames,
        upgradesList: upgradesList ?? this.upgradesList
      );
    }
}