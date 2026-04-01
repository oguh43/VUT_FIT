/*******************************************************************************
*                                                                              *
*                        Brno University of Technology                         *
*                      Faculty of Information Technology                       *
*                                                                              *
*                       Tvorba uživatelských rozhraní                          *
*                                                                              *
*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
*                                   Brno 2025                                  *
*                                                                              *
*                 Implementation of the tui main window widget                 *
*                                                                              *
*******************************************************************************/
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../models/tui/game_state.dart';
import '../../models/mac_system7/upgradeoffer.dart';
import '../../repositories/gamerepository.dart';
import 'react_minigame.dart';

class TUIScreen extends StatefulWidget {
  final GameRepository? repository;

  const TUIScreen({Key? key, this.repository}) : super(key: key);

  @override
  State<TUIScreen> createState() => _TUIScreenState();
}

class _TUIScreenState extends State<TUIScreen> {
  bool _showUpgrades = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF0000AA),
      body: SafeArea(
        child: Consumer<GameState>(
          builder: (context, gameState, child) {
            // Show loading indicator if initial load
            if (gameState.currentState == null && gameState.isLoading) {
              return const Center(
                child: CircularProgressIndicator(color: Colors.white),
              );
            }

            // Show error if present
            if (gameState.error != null && gameState.currentState == null) {
              return Center(
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Icon(Icons.error, color: Colors.red, size: 48),
                    const SizedBox(height: 16),
                    Text(
                      'Error: ${gameState.error}',
                      style: const TextStyle(color: Colors.white),
                      textAlign: TextAlign.center,
                    ),
                    const SizedBox(height: 16),
                    ElevatedButton(
                      onPressed: () => gameState.fetchGameState(),
                      child: const Text('Retry'),
                    ),
                  ],
                ),
              );
            }

            if (_showUpgrades) {
              return _buildUpgradesView(gameState);
            } else {
              return _buildMainView(gameState);
            }
          },
        ),
      ),
    );
  }

  Widget _buildMainView(GameState gameState) {
    final state = gameState.currentState;
    if (state == null) return const SizedBox.shrink();

    final upgrades = state.upgradesList.where((u) => u.count > 0).toList();
    final minigames = state.availableMiniGames;
    final hasMinigame = minigames.isNotEmpty;

    return Column(
      children: [
        // Header
        Container(
          width: double.infinity,
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
          decoration: BoxDecoration(
            color: Colors.grey[700],
            border: const Border(
              bottom: BorderSide(color: Colors.white, width: 2),
            ),
          ),
          child: Row(
            children: [
              _buildTabButton('Main view', true),
              const SizedBox(width: 16),
              _buildTabButton('Upgrades', false),
            ],
          ),
        ),

        Expanded(
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                // Left panel - Upgrades
                Expanded(
                  flex: 1,
                  child: _buildBox(
                    'Upgrades',
                    Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: upgrades.isEmpty
                          ? [
                              const Text(
                                'None yet',
                                style: TextStyle(
                                  color: Colors.white,
                                  fontFamily: 'Courier',
                                  fontSize: 14,
                                ),
                              ),
                            ]
                          : upgrades.map((u) {
                              return Padding(
                                padding: const EdgeInsets.only(bottom: 4),
                                child: Text(
                                  '${u.name}: ${u.count}\n${u.uxPointsPerSecond.toStringAsFixed(0)} UX/s',
                                  style: const TextStyle(
                                    color: Colors.white,
                                    fontFamily: 'Courier',
                                    fontSize: 14,
                                  ),
                                ),
                              );
                            }).toList(),
                    ),
                  ),
                ),

                const SizedBox(width: 16),

                // Center panel - Manage
                Expanded(
                  flex: 2,
                  child: _buildBox(
                    'Manage',
                    Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        Text(
                          'Available UX points: ${state.availableUxPoints}',
                          style: const TextStyle(
                            color: Colors.white,
                            fontFamily: 'Courier',
                            fontSize: 16,
                          ),
                          textAlign: TextAlign.center,
                        ),
                        const SizedBox(height: 8),
                        Text(
                          'Production: ${state.productionUxPointsPerSecond.toStringAsFixed(1)} UX/s',
                          style: const TextStyle(
                            color: Colors.greenAccent,
                            fontFamily: 'Courier',
                            fontSize: 14,
                          ),
                          textAlign: TextAlign.center,
                        ),
                        const SizedBox(height: 4),
                        Text(
                          '${((state.productionUxPointsPerSecond * (1.0 + state.bonusUxPointsPerSecond)) * 0.1).round().clamp(1, double.infinity).toInt()} UX/click',
                          style: const TextStyle(
                            color: Colors.cyanAccent,
                            fontFamily: 'Courier',
                            fontSize: 14,
                          ),
                          textAlign: TextAlign.center,
                        ),
                        if (state.bonusUxPointsPerSecond > 0)
                          Text(
                            'Bonus: +${(state.bonusUxPointsPerSecond * 100).toStringAsFixed(1)}%',
                            style: const TextStyle(
                              color: Colors.yellowAccent,
                              fontFamily: 'Courier',
                              fontSize: 14,
                            ),
                            textAlign: TextAlign.center,
                          ),
                        const SizedBox(height: 40),
                        ElevatedButton(
                          onPressed: gameState.isLoading
                              ? null
                              : () => gameState.clickDesignButton(),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.white,
                            foregroundColor: const Color(0xFF0000AA),
                            padding: const EdgeInsets.symmetric(
                              horizontal: 32,
                              vertical: 16,
                            ),
                            shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(0),
                            ),
                          ),
                          child: gameState.isLoading
                              ? const SizedBox(
                                  width: 20,
                                  height: 20,
                                  child: CircularProgressIndicator(
                                    strokeWidth: 2,
                                  ),
                                )
                              : const Text(
                                  'Design!',
                                  style: TextStyle(
                                    fontFamily: 'Courier',
                                    fontSize: 18,
                                    fontWeight: FontWeight.bold,
                                  ),
                                ),
                        ),
                        const SizedBox(height: 40),
                        TextButton(
                          onPressed: () {
                            setState(() {
                              _showUpgrades = true;
                            });
                          },
                          style: TextButton.styleFrom(
                            foregroundColor: Colors.white,
                          ),
                          child: const Text(
                            '[ Upgrades ]',
                            style: TextStyle(
                              fontFamily: 'Courier',
                              fontSize: 16,
                              decoration: TextDecoration.underline,
                            ),
                          ),
                        ),
                      ],
                    ),
                  ),
                ),

                const SizedBox(width: 16),

                // Right panel - Tech
                Expanded(
                  flex: 1,
                  child: _buildBox(
                    'Tech',
                    Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        if (hasMinigame)
                          ...minigames.map((minigame) {
                            return Padding(
                              padding: const EdgeInsets.only(bottom: 16),
                              child: Column(
                                children: [
                                  Text(
                                    '${minigame.title}\nminigame',
                                    style: const TextStyle(
                                      color: Colors.white,
                                      fontFamily: 'Courier',
                                      fontSize: 14,
                                    ),
                                    textAlign: TextAlign.center,
                                  ),
                                  const SizedBox(height: 8),
                                  ElevatedButton(
                                    onPressed: () {
                                      if (minigame.id == 'winapi') {
                                        Navigator.of(
                                          context,
                                        ).pushNamed('/w11/minigame');
                                      } else if (minigame.id == 'cors') {
                                        // TODO: Add CORS minigame route
                                        ScaffoldMessenger.of(
                                          context,
                                        ).showSnackBar(
                                          const SnackBar(
                                            content: Text(
                                              'CORS minigame coming soon!',
                                            ),
                                          ),
                                        );
                                      }
                                    },
                                    style: ElevatedButton.styleFrom(
                                      backgroundColor: Colors.white,
                                      foregroundColor: const Color(0xFF0000AA),
                                      shape: RoundedRectangleBorder(
                                        borderRadius: BorderRadius.circular(0),
                                      ),
                                      padding: const EdgeInsets.symmetric(
                                        horizontal: 24,
                                        vertical: 12,
                                      ),
                                    ),
                                    child: const Text(
                                      'Play!',
                                      style: TextStyle(
                                        fontFamily: 'Courier',
                                        fontSize: 14,
                                        fontWeight: FontWeight.bold,
                                      ),
                                      textAlign: TextAlign.center,
                                    ),
                                  ),
                                ],
                              ),
                            );
                          }).toList(),
                        // Add React minigame button
                        Padding(
                          padding: const EdgeInsets.only(bottom: 16),
                          child: Column(
                            children: [
                              const Text(
                                'React Challenge\nminigame',
                                style: TextStyle(
                                  color: Colors.white,
                                  fontFamily: 'Courier',
                                  fontSize: 14,
                                ),
                                textAlign: TextAlign.center,
                              ),
                              const SizedBox(height: 8),
                              ElevatedButton(
                                onPressed: () {
                                  Navigator.push(
                                    context,
                                    MaterialPageRoute(
                                      builder: (context) => ReactMinigame(
                                        repository: widget.repository!,
                                      ),
                                    ),
                                  );
                                },
                                style: ElevatedButton.styleFrom(
                                  backgroundColor: Colors.white,
                                  foregroundColor: const Color(0xFF0000AA),
                                  shape: RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(0),
                                  ),
                                  padding: const EdgeInsets.symmetric(
                                    horizontal: 24,
                                    vertical: 12,
                                  ),
                                ),
                                child: const Text(
                                  'Play!',
                                  style: TextStyle(
                                    fontFamily: 'Courier',
                                    fontSize: 14,
                                    fontWeight: FontWeight.bold,
                                  ),
                                  textAlign: TextAlign.center,
                                ),
                              ),
                            ],
                          ),
                        ),
                        if (!hasMinigame)
                          const Text(
                            'No minigames\navailable yet',
                            style: TextStyle(
                              color: Colors.white,
                              fontFamily: 'Courier',
                              fontSize: 14,
                            ),
                            textAlign: TextAlign.center,
                          ),
                      ],
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),

        // Footer
        Container(
          width: double.infinity,
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
          decoration: BoxDecoration(
            color: Colors.grey[700],
            border: const Border(
              top: BorderSide(color: Colors.white, width: 2),
            ),
          ),
          child: const Text(
            'v0.2a - API Edition',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Courier',
              fontSize: 12,
            ),
          ),
        ),

        // Taskbar
        _buildTaskbar(),
      ],
    );
  }

  Widget _buildUpgradesView(GameState gameState) {
    final state = gameState.currentState;
    final shopOffers = gameState.shopOffers;

    if (state == null) return const SizedBox.shrink();

    final ownedUpgrades = state.upgradesList.where((u) => u.count > 0).toList();

    return Column(
      children: [
        // Header
        Container(
          width: double.infinity,
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
          decoration: BoxDecoration(
            color: Colors.grey[700],
            border: const Border(
              bottom: BorderSide(color: Colors.white, width: 2),
            ),
          ),
          child: Row(
            children: [
              _buildTabButton('Main view', false),
              const SizedBox(width: 16),
              _buildTabButton('Upgrades', true),
            ],
          ),
        ),

        Expanded(
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                // Left panel - Current upgrades
                Expanded(
                  flex: 1,
                  child: _buildBox(
                    'Upgrades',
                    Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: ownedUpgrades.isEmpty
                          ? [
                              const Text(
                                'None yet',
                                style: TextStyle(
                                  color: Colors.white,
                                  fontFamily: 'Courier',
                                  fontSize: 14,
                                ),
                              ),
                            ]
                          : ownedUpgrades.map((u) {
                              return Padding(
                                padding: const EdgeInsets.only(bottom: 4),
                                child: Text(
                                  '${u.name}: ${u.count}\n${u.uxPointsPerSecond.toStringAsFixed(0)} UX/s',
                                  style: const TextStyle(
                                    color: Colors.white,
                                    fontFamily: 'Courier',
                                    fontSize: 14,
                                  ),
                                ),
                              );
                            }).toList(),
                    ),
                  ),
                ),

                const SizedBox(width: 16),

                // Right panel - Upgrade shop
                Expanded(
                  flex: 3,
                  child: _buildBox(
                    'Upgrade Shop',
                    shopOffers == null
                        ? const Center(
                            child: CircularProgressIndicator(
                              color: Colors.white,
                            ),
                          )
                        : SingleChildScrollView(
                            child: Table(
                              border: TableBorder.all(color: Colors.white),
                              columnWidths: const {
                                0: FlexColumnWidth(2),
                                1: FlexColumnWidth(1),
                                2: FlexColumnWidth(1),
                              },
                              children: [
                                TableRow(
                                  decoration: BoxDecoration(
                                    color: Colors.grey[800],
                                  ),
                                  children: [
                                    _buildTableCell('Class', isHeader: true),
                                    _buildTableCell('Cost', isHeader: true),
                                    _buildTableCell(
                                      'Production',
                                      isHeader: true,
                                    ),
                                  ],
                                ),
                                ...shopOffers.whereType<UpgradeOffer>().map((
                                  offer,
                                ) {
                                  final canAfford = gameState.canAfford(
                                    offer.cost,
                                  );
                                  return TableRow(
                                    children: [
                                      _buildTableCell(
                                        '[ ${offer.name} ]',
                                        canBuy: canAfford,
                                        onTap: canAfford && !gameState.isLoading
                                            ? () {
                                                gameState.buyUpgrade(offer.id);
                                              }
                                            : null,
                                      ),
                                      _buildTableCell('${offer.cost} UX'),
                                      _buildTableCell(
                                        '${offer.productionUxPointsPerSecond.toStringAsFixed(1)} UX/s',
                                      ),
                                    ],
                                  );
                                }).toList(),
                              ],
                            ),
                          ),
                  ),
                ),
              ],
            ),
          ),
        ),

        // Footer
        Container(
          width: double.infinity,
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
          decoration: BoxDecoration(
            color: Colors.grey[700],
            border: const Border(
              top: BorderSide(color: Colors.white, width: 2),
            ),
          ),
          child: const Text(
            'v0.2a - API Edition',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Courier',
              fontSize: 12,
            ),
          ),
        ),

        // Taskbar
        _buildTaskbar(),
      ],
    );
  }

  Widget _buildBox(String title, Widget content) {
    return Container(
      decoration: BoxDecoration(
        border: Border.all(color: Colors.white, width: 2),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Container(
            padding: const EdgeInsets.all(8),
            decoration: BoxDecoration(
              color: Colors.grey[800],
              border: const Border(
                bottom: BorderSide(color: Colors.white, width: 2),
              ),
            ),
            child: Text(
              title,
              style: const TextStyle(
                color: Colors.white,
                fontFamily: 'Courier',
                fontSize: 16,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
          Expanded(
            child: Padding(padding: const EdgeInsets.all(8.0), child: content),
          ),
        ],
      ),
    );
  }

  Widget _buildTabButton(String label, bool isActive) {
    return GestureDetector(
      onTap: () {
        setState(() {
          _showUpgrades = label == 'Upgrades';
        });
      },
      child: Text(
        label,
        style: TextStyle(
          color: Colors.white,
          fontFamily: 'Courier',
          fontSize: 14,
          decoration: isActive ? TextDecoration.underline : null,
          fontWeight: isActive ? FontWeight.bold : FontWeight.normal,
        ),
      ),
    );
  }

  Widget _buildTableCell(
    String text, {
    bool isHeader = false,
    bool canBuy = false,
    VoidCallback? onTap,
  }) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: const EdgeInsets.all(8),
        color: canBuy ? Colors.green[900] : null,
        child: Text(
          text,
          style: TextStyle(
            color: canBuy ? Colors.greenAccent : Colors.white,
            fontFamily: 'Courier',
            fontSize: 12,
            fontWeight: isHeader ? FontWeight.bold : FontWeight.normal,
          ),
          textAlign: TextAlign.center,
        ),
      ),
    );
  }

  Widget _buildTaskbar() {
    return Container(
      height: 48,
      decoration: BoxDecoration(
        color: Colors.grey[900],
        border: Border(top: BorderSide(color: Colors.white, width: 2)),
      ),
      child: Row(
        children: [
          const SizedBox(width: 8),
          _buildTaskbarButton('TUI', Icons.terminal, 'tui'),
          const SizedBox(width: 4),
          _buildTaskbarButton('Mac System 7', Icons.computer, 'mac_system7'),
          const SizedBox(width: 4),
          _buildTaskbarButton('Windows 11', Icons.window, 'win11'),
          const Spacer(),
        ],
      ),
    );
  }

  Widget _buildTaskbarButton(String label, IconData icon, String uiType) {
    return Material(
      color: Colors.grey[800],
      elevation: 2,
      borderRadius: BorderRadius.circular(4),
      child: InkWell(
        onTap: () async {
          // Call API to update UI state if repository is available
          if (widget.repository != null) {
            try {
              await widget.repository!.setCurrentUI(uiType);
            } catch (e) {
              print('Error setting UI: $e');
            }
          }

          // Navigate to the appropriate screen
          if (mounted) {
            switch (uiType) {
              case 'mac_system7':
                Navigator.of(context).pushReplacementNamed('/mac');
                break;
              case 'win11':
                Navigator.of(context).pushReplacementNamed('/w11');
                break;
              case 'tui':
                Navigator.of(context).pushReplacementNamed('/');
                break;
            }
          }
        },
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
          child: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              Icon(icon, size: 20, color: Colors.white70),
              const SizedBox(width: 8),
              Text(
                label,
                style: const TextStyle(
                  fontFamily: 'Courier',
                  fontWeight: FontWeight.w600,
                  fontSize: 13,
                  color: Colors.white70,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
