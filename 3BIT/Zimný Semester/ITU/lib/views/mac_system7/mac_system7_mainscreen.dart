// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';
import '../../controllers/mac_system7/clickergame.dart';
import '../../models/mac_system7/clickergamestate.dart';
import '../../models/mac_system7/upgradeoffer.dart';
import '../../models/mac_system7/minigame.dart';
import '../../models/mac_system7/window_state_manager.dart';
import 'mac_system7_mini_game_card.dart';
import 'mac_system7_upgrade_card.dart';
import 'mac_system7_draggable_window.dart';

class MacSystem7MainScreen extends StatefulWidget {
  final ClickerGame controller;

  const MacSystem7MainScreen({super.key, required this.controller});

  @override
  State<MacSystem7MainScreen> createState() => _MacSystem7MainScreenState();
}

class _MacSystem7MainScreenState extends State<MacSystem7MainScreen>
    with TickerProviderStateMixin, AutomaticKeepAliveClientMixin {
  late AnimationController _progressController;
  late AnimationController _buttonController;
  late Animation<double> _buttonScaleAnimation;
  late AnimationController _uxCounterController;
  late Animation<double> _uxCounterScaleAnimation;
  int _previousUxPoints = 0;
  int _clickCount = 0;
  List<UpgradeOffer>? _cachedShopOffers;
  bool _isLoadingOffers = false;

  // Use singleton window state manager to persist state across navigation
  final WindowStateManager _windowStateManager = WindowStateManager();

  @override
  void initState() {
    super.initState();
    _progressController = AnimationController(
      duration: const Duration(milliseconds: 200),
      vsync: this,
    );

    _buttonController = AnimationController(
      duration: const Duration(milliseconds: 150),
      vsync: this,
    );

    _buttonScaleAnimation = Tween<double>(begin: 1.0, end: 0.95).animate(
      CurvedAnimation(parent: _buttonController, curve: Curves.easeInOut),
    );

    _uxCounterController = AnimationController(
      duration: const Duration(milliseconds: 300),
      vsync: this,
    );

    _uxCounterScaleAnimation = Tween<double>(begin: 1.0, end: 1.1).animate(
      CurvedAnimation(parent: _uxCounterController, curve: Curves.elasticOut),
    );

    // Load shop offers once on init
    _loadShopOffers();
  }

  @override
  void dispose() {
    _progressController.dispose();
    _buttonController.dispose();
    _uxCounterController.dispose();
    super.dispose();
  }

  Future<void> _loadShopOffers() async {
    if (_isLoadingOffers) return;
    _isLoadingOffers = true;
    try {
      final offers = await widget.controller.repository.fetchShopOffers();
      if (mounted) {
        setState(() {
          _cachedShopOffers = offers;
        });
      }
    } finally {
      _isLoadingOffers = false;
    }
  }

  @override
  bool get wantKeepAlive => true;

  @override
  Widget build(BuildContext context) {
    super.build(context); // Required for AutomaticKeepAliveClientMixin
    return Scaffold(
      backgroundColor: const Color(0xFF2CA6A4), // Teal Mac System 7 background
      body: SafeArea(
        child: Column(
          children: [
            _buildMenuBar(),
            Expanded(
              child: LayoutBuilder(
                builder: (context, constraints) {
                  return ValueListenableBuilder<ClickerGameState>(
                    valueListenable: widget.controller.state,
                    builder: (context, gameState, _) {
                      final screenWidth = constraints.maxWidth;
                      final screenHeight = constraints.maxHeight;

                      return Stack(
                        children: [
                          // Desktop icons on the left
                          _buildDesktopIcons(screenWidth, screenHeight),
                          // Reorder windows based on z-order
                          ..._windowStateManager.windowZOrder
                              .where(
                                (id) => !_windowStateManager.isWindowHidden(id),
                              )
                              .map((windowId) {
                                switch (windowId) {
                                  case 'upgrades':
                                    final defaultPos = Offset(
                                      screenWidth * 0.15,
                                      screenHeight * 0.05,
                                    );
                                    final pos =
                                        _windowStateManager.getWindowPosition(
                                          'upgrades',
                                        ) ??
                                        defaultPos;
                                    return MacSystem7DraggableWindow(
                                      key: const ValueKey('upgrades'),
                                      title: 'Upgrades',
                                      initialX: pos.dx,
                                      initialY: pos.dy,
                                      width: screenWidth * 0.25,
                                      height: screenHeight * 0.6,
                                      onTap: () => _bringToFront('upgrades'),
                                      onClose: () => _hideWindow('upgrades'),
                                      onPositionChanged: (x, y) =>
                                          _updateWindowPosition(
                                            'upgrades',
                                            x,
                                            y,
                                          ),
                                      child: ListView.separated(
                                        itemCount:
                                            gameState.upgradesList.length,
                                        separatorBuilder: (_, __) =>
                                            const SizedBox(height: 16),
                                        itemBuilder: (context, index) {
                                          return MacSystem7UpgradeCard(
                                            upgrade:
                                                gameState.upgradesList[index],
                                            unconstrained: true,
                                          );
                                        },
                                      ),
                                    );
                                  case 'ux_counter':
                                    final defaultPos = Offset(
                                      screenWidth * 0.42,
                                      screenHeight * 0.05,
                                    );
                                    final pos =
                                        _windowStateManager.getWindowPosition(
                                          'ux_counter',
                                        ) ??
                                        defaultPos;
                                    return MacSystem7DraggableWindow(
                                      key: const ValueKey('ux_counter'),
                                      title: 'UX Counter',
                                      initialX: pos.dx,
                                      initialY: pos.dy,
                                      width: screenWidth * 0.22,
                                      height: screenHeight * 0.25,
                                      onTap: () => _bringToFront('ux_counter'),
                                      onClose: () => _hideWindow('ux_counter'),
                                      onPositionChanged: (x, y) =>
                                          _updateWindowPosition(
                                            'ux_counter',
                                            x,
                                            y,
                                          ),
                                      child: _buildUxCounter(
                                        context,
                                        gameState,
                                      ),
                                    );
                                  case 'design_button':
                                    final defaultPos = Offset(
                                      screenWidth * 0.42,
                                      screenHeight * 0.32,
                                    );
                                    final pos =
                                        _windowStateManager.getWindowPosition(
                                          'design_button',
                                        ) ??
                                        defaultPos;
                                    return MacSystem7DraggableWindow(
                                      key: const ValueKey('design_button'),
                                      title: 'Design Button',
                                      initialX: pos.dx,
                                      initialY: pos.dy,
                                      width: screenWidth * 0.22,
                                      height: screenHeight * 0.17,
                                      showDecorations: false,
                                      onTap: () =>
                                          _bringToFront('design_button'),
                                      onClose: () =>
                                          _hideWindow('design_button'),
                                      onPositionChanged: (x, y) =>
                                          _updateWindowPosition(
                                            'design_button',
                                            x,
                                            y,
                                          ),
                                      child: _buildDesignButton(
                                        context,
                                        gameState,
                                      ),
                                    );
                                  case 'upgrade_shop':
                                    final defaultPos = Offset(
                                      screenWidth * 0.42,
                                      screenHeight * 0.51,
                                    );
                                    final pos =
                                        _windowStateManager.getWindowPosition(
                                          'upgrade_shop',
                                        ) ??
                                        defaultPos;
                                    return MacSystem7DraggableWindow(
                                      key: const ValueKey('upgrade_shop'),
                                      title: 'Hire Staff',
                                      initialX: pos.dx,
                                      initialY: pos.dy,
                                      width: screenWidth * 0.4,
                                      height: screenHeight * 0.47,
                                      decorationScale: 0.6,
                                      onTap: () =>
                                          _bringToFront('upgrade_shop'),
                                      onClose: () =>
                                          _hideWindow('upgrade_shop'),
                                      onPositionChanged: (x, y) =>
                                          _updateWindowPosition(
                                            'upgrade_shop',
                                            x,
                                            y,
                                          ),
                                      child: _buildUpgradeShop(
                                        context,
                                        gameState,
                                      ),
                                    );
                                  case 'tech':
                                    final defaultPos = Offset(
                                      screenWidth * 0.7,
                                      screenHeight * 0.05,
                                    );
                                    final pos =
                                        _windowStateManager.getWindowPosition(
                                          'tech',
                                        ) ??
                                        defaultPos;
                                    return MacSystem7DraggableWindow(
                                      key: const ValueKey('tech'),
                                      title: 'Tech',
                                      initialX: pos.dx,
                                      initialY: pos.dy,
                                      width: screenWidth * 0.26,
                                      height: screenHeight * 0.4,
                                      onTap: () => _bringToFront('tech'),
                                      onClose: () => _hideWindow('tech'),
                                      onPositionChanged: (x, y) =>
                                          _updateWindowPosition('tech', x, y),
                                      child: SingleChildScrollView(
                                        child: Column(
                                          crossAxisAlignment:
                                              CrossAxisAlignment.start,
                                          children: [
                                            ...gameState.availableMiniGames.map(
                                              (minigame) => Padding(
                                                padding: const EdgeInsets.only(
                                                  bottom: 12,
                                                ),
                                                child: MacSystem7MiniGameCard(
                                                  minigame: minigame,
                                                  onClick: () => widget
                                                      .controller
                                                      .onMiniGameSelected(
                                                        context,
                                                        minigame,
                                                      ),
                                                ),
                                              ),
                                            ),
                                            // Add React minigame
                                            Padding(
                                              padding: const EdgeInsets.only(
                                                bottom: 12,
                                              ),
                                              child: MacSystem7MiniGameCard(
                                                minigame: const MiniGame(
                                                  id: 'react',
                                                  title: 'React Challenge',
                                                ),
                                                onClick: () => Navigator.of(
                                                  context,
                                                ).pushNamed('/mac/react'),
                                              ),
                                            ),
                                          ],
                                        ),
                                      ),
                                    );
                                  default:
                                    return const SizedBox.shrink();
                                }
                              })
                              .toList(),
                        ],
                      );
                    },
                  );
                },
              ),
            ),
            _buildTaskbar(),
          ],
        ),
      ),
    );
  }

  void _bringToFront(String windowId) {
    setState(() {
      _windowStateManager.bringToFront(windowId);
    });
  }

  void _hideWindow(String windowId) {
    setState(() {
      _windowStateManager.hideWindow(windowId);
    });
  }

  void _showWindow(String windowId) {
    setState(() {
      _windowStateManager.showWindow(windowId);
    });
  }

  void _updateWindowPosition(String windowId, double x, double y) {
    _windowStateManager.updateWindowPosition(windowId, x, y);
  }

  Widget _buildDesktopIcons(double screenWidth, double screenHeight) {
    final iconSize = screenWidth * 0.06;
    final fontSize = screenWidth * 0.011;

    final icons = [
      {'id': 'upgrades', 'label': 'Upgrades', 'icon': Icons.people},
      {'id': 'ux_counter', 'label': 'UX Counter', 'icon': Icons.analytics},
      {'id': 'upgrade_shop', 'label': 'Upgrade Shop', 'icon': Icons.store},
      {'id': 'tech', 'label': 'Tech', 'icon': Icons.computer},
    ];

    return Positioned(
      left: 10,
      top: 10,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.center,
        children: icons.map((iconData) {
          final windowId = iconData['id'] as String;
          final isHidden = _windowStateManager.isWindowHidden(windowId);
          final iconColor = isHidden ? Colors.grey[400]! : Colors.white;

          return Padding(
            padding: const EdgeInsets.only(bottom: 20),
            child: GestureDetector(
              onTap: () {
                if (isHidden) {
                  _showWindow(windowId);
                } else {
                  _hideWindow(windowId);
                }
              },
              child: Container(
                width: iconSize,
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Icon(
                      iconData['icon'] as IconData,
                      size: iconSize * 0.6,
                      color: iconColor,
                    ),
                    const SizedBox(height: 4),
                    Text(
                      iconData['label'] as String,
                      style: TextStyle(
                        fontFamily: 'Courier',
                        fontWeight: FontWeight.w700,
                        fontSize: fontSize,
                        color: iconColor,
                        shadows: const [
                          Shadow(
                            color: Colors.black,
                            offset: Offset(1, 1),
                            blurRadius: 2,
                          ),
                        ],
                      ),
                      textAlign: TextAlign.center,
                    ),
                  ],
                ),
              ),
            ),
          );
        }).toList(),
      ),
    );
  }

  Widget _buildMenuBar() {
    return LayoutBuilder(
      builder: (context, constraints) {
        final fontSize = (constraints.maxWidth * 0.012).clamp(11.0, 14.0);
        final iconSize = (constraints.maxWidth * 0.015).clamp(14.0, 18.0);

        return Container(
          color: Colors.white,
          padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
          child: Row(
            children: [
              Icon(Icons.info_outline, size: iconSize, color: Colors.black),
              const SizedBox(width: 8),
              ...['File', 'Edit', 'View', 'Label', 'Special'].map(
                (item) => Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 8),
                  child: Text(
                    item,
                    style: TextStyle(
                      fontFamily: 'Courier',
                      fontWeight: FontWeight.w700,
                      fontSize: fontSize,
                    ),
                  ),
                ),
              ),
              const Spacer(),
              Text(
                _getTimeString(),
                style: TextStyle(
                  fontFamily: 'Courier',
                  fontWeight: FontWeight.w700,
                  fontSize: fontSize,
                ),
              ),
              const SizedBox(width: 8),
              Icon(Icons.help_outline, size: iconSize, color: Colors.black),
            ],
          ),
        );
      },
    );
  }

  String _getTimeString() {
    final now = DateTime.now();
    return '${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')} PM';
  }

  Widget _buildTaskbar() {
    return Container(
      height: 48,
      color: const Color(0xFFCCCCCC),
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
      color: Colors.white,
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
              Icon(icon, size: 20, color: Colors.black87),
              const SizedBox(width: 8),
              Text(
                label,
                style: const TextStyle(
                  fontFamily: 'Courier',
                  fontWeight: FontWeight.w600,
                  fontSize: 13,
                  color: Colors.black87,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildUxCounter(BuildContext context, ClickerGameState state) {
    // Trigger animation if UX points increased
    if (state.availableUxPoints > _previousUxPoints) {
      _uxCounterController.forward().then(
        (_) => _uxCounterController.reverse(),
      );
    }
    _previousUxPoints = state.availableUxPoints;

    return LayoutBuilder(
      builder: (context, constraints) {
        final largeFontSize = constraints.maxWidth * 0.12;
        final smallFontSize = constraints.maxWidth * 0.075;
        final progressBarWidth = constraints.maxWidth * 0.7;

        return Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  AnimatedBuilder(
                    animation: _uxCounterScaleAnimation,
                    builder: (context, child) {
                      return Transform.scale(
                        scale: _uxCounterScaleAnimation.value,
                        child: child,
                      );
                    },
                    child: Text(
                      '${state.availableUxPoints}',
                      style: TextStyle(
                        fontSize: largeFontSize,
                        fontWeight: FontWeight.w700,
                        fontFamily: 'Courier',
                        color: Colors.black,
                      ),
                    ),
                  ),
                  const SizedBox(width: 8),
                  Text(
                    '[UX]',
                    style: TextStyle(
                      fontSize: largeFontSize,
                      fontWeight: FontWeight.w700,
                      fontFamily: 'Courier',
                      color: Colors.black,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 8),
              Text(
                '${state.productionUxPointsPerSecond} UX/s',
                style: TextStyle(
                  fontSize: smallFontSize,
                  fontWeight: FontWeight.w200,
                  fontFamily: 'Courier',
                  color: Colors.black,
                ),
              ),
              const SizedBox(height: 4),
              Text(
                '${((state.productionUxPointsPerSecond * (1.0 + state.bonusUxPointsPerSecond)) * 0.1).round().clamp(1, double.infinity).toInt()} UX/click',
                style: TextStyle(
                  fontSize: smallFontSize * 0.9,
                  fontWeight: FontWeight.w300,
                  fontFamily: 'Courier',
                  color: Colors.blue[700],
                ),
              ),
              if (state.bonusUxPointsPerSecond > 0) ...[
                const SizedBox(height: 4),
                Text(
                  '+${(state.bonusUxPointsPerSecond * 100).toStringAsFixed(1)}% (bonus)',
                  style: TextStyle(
                    fontSize: smallFontSize * 0.8,
                    fontWeight: FontWeight.w600,
                    fontFamily: 'Courier',
                    color: Colors.green[700],
                  ),
                ),
              ],
              const SizedBox(height: 16),
              SizedBox(
                width: progressBarWidth,
                child: LinearProgressIndicator(
                  value: _progressController.value,
                  backgroundColor: Colors.grey[300],
                  valueColor: AlwaysStoppedAnimation<Color>(Colors.green[400]!),
                ),
              ),
            ],
          ),
        );
      },
    );
  }

  Widget _buildDesignButton(BuildContext context, ClickerGameState state) {
    return LayoutBuilder(
      builder: (context, constraints) {
        final titleFontSize = constraints.maxWidth * 0.057;
        final buttonTextSize = constraints.maxWidth * 0.1;

        return Center(
          child: SizedBox(
            width: double.infinity,
            child: AnimatedBuilder(
              animation: _buttonScaleAnimation,
              builder: (context, child) {
                return Transform.scale(
                  scale: _buttonScaleAnimation.value,
                  child: child,
                );
              },
              child: TextButton(
                onPressed: () {
                  _buttonController.forward().then(
                    (_) => _buttonController.reverse(),
                  );
                  _clickCount++;
                  _progressController.animateTo((_clickCount % 10) / 10.0);
                  widget.controller.onButtonPressed();
                },
                style: TextButton.styleFrom(
                  backgroundColor: const Color(0xFFCCFFCC),
                  foregroundColor: Colors.black,
                  side: const BorderSide(color: Colors.transparent, width: 0),
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(2),
                  ),
                  padding: EdgeInsets.zero,
                ),
                child: Container(
                  decoration: BoxDecoration(
                    color: const Color(0xFFCCFFCC),
                    border: Border.all(color: Colors.black, width: 2),
                    borderRadius: BorderRadius.circular(2),
                  ),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [
                      Container(
                        color: const Color(0xFFCCFFCC),
                        padding: const EdgeInsets.symmetric(
                          horizontal: 8,
                          vertical: 4,
                        ),
                        child: Center(
                          child: Text(
                            'Button',
                            style: TextStyle(
                              fontFamily: 'Courier',
                              fontWeight: FontWeight.w700,
                              fontSize: titleFontSize,
                              color: Colors.black,
                            ),
                          ),
                        ),
                      ),
                      Container(height: 1, color: Colors.black),
                      Padding(
                        padding: const EdgeInsets.symmetric(vertical: 8),
                        child: Center(
                          child: Text(
                            '<Design!>',
                            style: TextStyle(
                              fontSize: buttonTextSize,
                              fontWeight: FontWeight.w700,
                              fontFamily: 'Courier',
                              color: Colors.black,
                              decoration: TextDecoration.none,
                            ),
                          ),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        );
      },
    );
  }

  Widget _buildUpgradeShop(BuildContext context, ClickerGameState state) {
    if (_cachedShopOffers == null) {
      return const Center(child: CircularProgressIndicator());
    }

    final offers = _cachedShopOffers!;
    return LayoutBuilder(
      builder: (context, constraints) {
        final headerFontSize = constraints.maxWidth * 0.032;
        final itemFontSize = constraints.maxWidth * 0.028;
        final buttonFontSize = constraints.maxWidth * 0.028;

        return Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Available UX: ${state.availableUxPoints}',
              style: TextStyle(
                fontFamily: 'Courier',
                fontWeight: FontWeight.w700,
                fontSize: headerFontSize,
                color: Colors.black,
              ),
            ),
            const SizedBox(height: 12),
            Expanded(
              child: ListView.separated(
                itemCount: offers.length,
                separatorBuilder: (_, __) => const SizedBox(height: 8),
                itemBuilder: (context, index) {
                  final offer = offers[index];
                  final canAfford = state.availableUxPoints >= offer.cost;
                  return Container(
                    decoration: BoxDecoration(
                      color: index.isEven
                          ? const Color(0xFFF4F4F4)
                          : const Color(0xFFE0E0E0),
                      border: Border.all(color: Colors.black, width: 1),
                    ),
                    padding: const EdgeInsets.all(8),
                    child: Row(
                      children: [
                        Expanded(
                          flex: 3,
                          child: Text(
                            offer.name,
                            style: TextStyle(
                              fontFamily: 'Courier',
                              fontWeight: FontWeight.w700,
                              fontSize: itemFontSize,
                              color: Colors.black,
                            ),
                          ),
                        ),
                        Expanded(
                          flex: 2,
                          child: Text(
                            '${offer.productionUxPointsPerSecond} UX/s',
                            style: TextStyle(
                              fontFamily: 'Courier',
                              fontSize: itemFontSize,
                              color: Colors.black,
                            ),
                          ),
                        ),
                        Expanded(
                          flex: 2,
                          child: Text(
                            'Cost: ${offer.cost}',
                            style: TextStyle(
                              fontFamily: 'Courier',
                              fontSize: itemFontSize,
                              color: Colors.black,
                            ),
                          ),
                        ),
                        TextButton(
                          onPressed: canAfford
                              ? () async {
                                  final newState = await widget
                                      .controller
                                      .repository
                                      .buyItem(offer.id);
                                  widget.controller.state.value = newState;
                                  // Reload shop offers after purchase
                                  _loadShopOffers();
                                }
                              : null,
                          style: TextButton.styleFrom(
                            backgroundColor: canAfford
                                ? Colors.white
                                : Colors.grey[400],
                            foregroundColor: Colors.black,
                            padding: EdgeInsets.symmetric(
                              horizontal: constraints.maxWidth * 0.024,
                              vertical: 4,
                            ),
                            shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(2),
                            ),
                          ),
                          child: Text(
                            'Hire',
                            style: TextStyle(
                              fontFamily: 'Courier',
                              fontWeight: FontWeight.w700,
                              fontSize: buttonFontSize,
                              color: canAfford
                                  ? Colors.black
                                  : Colors.grey[600],
                            ),
                          ),
                        ),
                      ],
                    ),
                  );
                },
              ),
            ),
          ],
        );
      },
    );
  }
}
