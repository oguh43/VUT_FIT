// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import 'dart:async';
import '../../controllers/w11/clickergame.dart';
import '../../models/mac_system7/clickergamestate.dart';
import '../../models/mac_system7/minigame.dart';
import 'widgets/minigamecard.dart';
import 'widgets/upgradecard.dart';
import 'widgets/upgradescolumn.dart';
import 'widgets/sidedrawer.dart';
import 'widgets/bonuspointschip.dart';

// Main W11 screen widget — composes upgrades, main action and tech/minigames
class MainScreen extends StatefulWidget {
  final W11ClickerGame controller;
  const MainScreen({super.key, required this.controller});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  // Timer used to periodically refresh game state from the controller
  Timer? _pollTimer;

  // Initialize the screen: load initial state and start periodic polling
  @override
  void initState() {
    super.initState();
    widget.controller.loadFromApi();
    _pollTimer = Timer.periodic(const Duration(seconds: 1), (_) {
      widget.controller.loadFromApi();
    });
  }

  @override
  void dispose() {
    _pollTimer?.cancel();
    super.dispose();
  }

  // Build the main W11 scaffold layout and listen to controller.state
  @override
  Widget build(BuildContext context) {
    final double width = MediaQuery.sizeOf(context).width;
    return Scaffold(
      backgroundColor: const Color(0xFFF3F3F3),
      drawer: const SideDrawer(),
      body: SafeArea(
        top: true,
        bottom: false,
        // Rebuild UI on state changes from controller.state
        child: ValueListenableBuilder<ClickerGameState>(
          valueListenable: widget.controller.state,
          builder: (context, gameState, _) {
            return Padding(
              padding: const EdgeInsets.fromLTRB(16, 16, 16, 0),
              child: Column(
                children: [
                  Expanded(
                    child: Stack(
                      children: [
                        // Layout: three-column row — upgrades column, main action column, and tech/minigames
                        Positioned.fill(
                          child: Row(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              // Sidebar showing upgrades (interns). Subscribes to controller state in its own widget
                              UpgradesColumn(controller: widget.controller),
                              const SizedBox(width: 24),
                              // Central action area: shows available UX points and primary "Design!" button
                              _buildMainColumn(context, gameState),
                              const SizedBox(width: 24),
                              // Right column listing available mini-games; tapping triggers onMiniGameSelected
                              _buildTechColumn(context, gameState),
                            ],
                          ),
                        ),
                        Positioned(
                          left: 0,
                          right: 0,
                          bottom: 0,
                          child: Align(
                            alignment: Alignment.bottomLeft,
                            child: Padding(
                              padding: EdgeInsets.only(left: width - 310),
                              // Bottom bar showing current production and optional bonus chip
                              child: _buildBottomBar(gameState),
                            ),
                          ),
                        ),
                      ],
                    ),
                  ),
                  _buildTaskbar(),
                ],
              ),
            );
          },
        ),
      ),
    );
  }

  // Central action area: shows available UX points and primary "Design!" button
  Widget _buildMainColumn(BuildContext context, ClickerGameState state) {
    final titleStyle = _titleStyle();

    return Expanded(
      flex: 3,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('Manage', style: titleStyle),
          const SizedBox(height: 8),
          Expanded(
            child: SizedBox.expand(
              child: Container(
                padding: const EdgeInsets.fromLTRB(16, 12, 16, 24),
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius: BorderRadius.vertical(top: Radius.circular(8)),
                  border: Border.all(color: Colors.black12),
                  boxShadow: const [
                    BoxShadow(
                      color: Colors.black12,
                      blurRadius: 8,
                      offset: Offset(0, 2),
                    ),
                  ],
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'Available UX points: ${state.availableUxPoints}',
                      style: const TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.w400,
                      ),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'Production: ${state.productionUxPointsPerSecond.toStringAsFixed(1)} UX/s',
                      style: const TextStyle(
                        fontSize: 14,
                        fontWeight: FontWeight.w400,
                        color: Colors.black54,
                      ),
                    ),
                    const SizedBox(height: 4),
                    Text(
                      '${((state.productionUxPointsPerSecond * (1.0 + state.bonusUxPointsPerSecond)) * 0.1).round().clamp(1, double.infinity).toInt()} UX/click',
                      style: const TextStyle(
                        fontSize: 14,
                        fontWeight: FontWeight.w400,
                        color: Colors.black54,
                      ),
                    ),
                    if (state.bonusUxPointsPerSecond > 0) ...[
                      const SizedBox(height: 4),
                      Text(
                        'Bonus: +${(state.bonusUxPointsPerSecond * 100).toStringAsFixed(1)}%',
                        style: const TextStyle(
                          fontSize: 14,
                          fontWeight: FontWeight.w600,
                          color: Color(0xFF107C10),
                        ),
                      ),
                    ],
                    const SizedBox(height: 4),
                    Text(
                      'Total income: ${(state.productionUxPointsPerSecond * (1.0 + state.bonusUxPointsPerSecond)).toStringAsFixed(1)} UX/s',
                      style: const TextStyle(
                        fontSize: 14,
                        fontWeight: FontWeight.w400,
                        color: Colors.black54,
                      ),
                    ),
                    const Spacer(),
                    Center(
                      child: SizedBox(
                        width: 260,
                        height: 90,
                        child: ElevatedButton(
                          onPressed: widget.controller.onButtonPressed,
                          style: ElevatedButton.styleFrom(
                            backgroundColor: const Color(0xFF005FB8),
                            shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(8),
                            ),
                            textStyle: const TextStyle(
                              fontSize: 28,
                              fontWeight: FontWeight.w400,
                            ),
                          ),
                          child: const Text(
                            'Design!',
                            style: TextStyle(color: Colors.white),
                          ),
                        ),
                      ),
                    ),
                    const SizedBox(height: 40),
                    Center(
                      child: SizedBox(
                        width: 260,
                        height: 80,
                        child: OutlinedButton(
                          onPressed: () => widget.controller
                              .onNavigateToUpgradeShop(context),
                          style: OutlinedButton.styleFrom(
                            side: const BorderSide(
                              width: 1.5,
                              color: Colors.black12,
                            ),
                            backgroundColor: Colors.white,
                            overlayColor: const Color(0xFFD6D6D6),
                            shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(8),
                            ),
                            textStyle: const TextStyle(
                              fontSize: 28,
                              fontWeight: FontWeight.w400,
                            ),
                          ),
                          child: const Text(
                            'Upgrades',
                            style: TextStyle(color: Colors.black),
                          ),
                        ),
                      ),
                    ),
                    const Spacer(flex: 2),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

  // Right column listing available mini-games; tapping triggers onMiniGameSelected
  Widget _buildTechColumn(BuildContext context, ClickerGameState state) {
    final titleStyle = _titleStyle();

    return SizedBox(
      width: 260,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('Tech', style: titleStyle),
          const SizedBox(height: 16),
          Expanded(
            child: ListView.separated(
              itemCount: state.availableMiniGames.length + 1,
              separatorBuilder: (_, __) => const SizedBox(height: 16),
              itemBuilder: (context, index) {
                if (index < state.availableMiniGames.length) {
                  final MiniGame minigame = state.availableMiniGames[index];
                  return MiniGameCard(
                    minigame: minigame,
                    onClick: () =>
                        widget.controller.onMiniGameSelected(context, minigame),
                  );
                } else {
                  // Add React minigame
                  return MiniGameCard(
                    minigame: const MiniGame(
                      id: 'react',
                      title: 'React Challenge',
                    ),
                    onClick: () =>
                        Navigator.of(context).pushNamed('/w11/react'),
                  );
                }
              },
            ),
          ),
        ],
      ),
    );
  }

  // Bottom bar showing current production and optional bonus chip
  Widget _buildBottomBar(ClickerGameState state) {
    final bonus = state.bonusUxPointsPerSecond;
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Text(
          'Production: ${state.productionUxPointsPerSecond.toStringAsFixed(0)} UX/s',
          style: const TextStyle(fontSize: 14, fontWeight: FontWeight.w400),
        ),
        if (bonus > 0) const SizedBox(width: 12),
        if (bonus > 0) BonusPointsChip(bonus: bonus),
      ],
    );
  }

  // Centralized text style for section titles
  TextStyle _titleStyle() {
    return const TextStyle(fontSize: 28, fontWeight: FontWeight.w700);
  }

  Widget _buildTaskbar() {
    return Container(
      height: 48,
      decoration: BoxDecoration(
        color: const Color(0xFF202020),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.3),
            blurRadius: 8,
            offset: const Offset(0, -2),
          ),
        ],
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
      color: const Color(0xFF2D2D2D),
      elevation: 2,
      borderRadius: BorderRadius.circular(4),
      child: InkWell(
        onTap: () async {
          // Update the UI on the server
          await widget.controller.repository.setCurrentUI(uiType);

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
