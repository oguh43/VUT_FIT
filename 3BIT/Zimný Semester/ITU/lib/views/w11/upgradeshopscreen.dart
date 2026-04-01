// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../controllers/w11/upgradeshop.dart';
import '../../controllers/w11/clickergame.dart';
import '../../models/mac_system7/upgradeoffer.dart';
import '../../models/mac_system7/clickergamestate.dart';
import 'widgets/upgradecard.dart';
import 'widgets/upgradescolumn.dart';
import 'widgets/upgradeofferrow.dart';
import 'widgets/sidedrawer.dart';

// Screen showing the W11 upgrade shop
class UpgradeShopScreen extends StatelessWidget {
  final W11UpgradeShop controller;

  const UpgradeShopScreen({super.key, required this.controller});

  @override
  Widget build(BuildContext context) {
    final W11ClickerGame gameController = Navigator.of(context).widget.onGenerateRoute == null ? W11ClickerGame(controller.repository) : controller.gameController;
    return Scaffold(
      backgroundColor: const Color(0xFFF3F3F3),
      drawer: const SideDrawer(),
      body: SafeArea(
        top: true,
        bottom: false,
        right: false,
        child: Stack(
          children: [
            Positioned.fill(
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  UpgradesColumn(controller: gameController),
                  const SizedBox(width: 24),
                  Expanded(
                    child: Padding(
                      padding: const EdgeInsets.only(top: 8.0),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const Padding(
                            padding: EdgeInsets.only(top: 8.0),
                            child: Text('Upgrade Shop',
                              style: const TextStyle(
                                fontSize: 28,
                                fontWeight: FontWeight.w700
                              )
                            )
                          ),
                          const SizedBox(height: 8),
                          Expanded(
                            child: SizedBox.expand(
                              child: Container(
                                padding: const EdgeInsets.fromLTRB(16, 12, 16, 24),
                                decoration: BoxDecoration(
                                  color: Colors.white,
                                  borderRadius: BorderRadius.only(
                                    topLeft: Radius.circular(8)
                                  ),
                                  border: Border.all(color: Colors.black12),
                                  boxShadow: const [
                                    BoxShadow(
                                      color: Colors.black12,
                                      blurRadius: 8,
                                      offset: Offset(0, 2)
                                    )
                                  ]
                                ),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    // Notifier with current shop offers displayed in the list
                                    ValueListenableBuilder<ClickerGameState>(
                                      valueListenable: gameController.state,
                                      builder: (_, state, __) => Text(
                                        'Available UX points: ${state.availableUxPoints}',
                                        style: const TextStyle(fontSize: 16),
                                      ),
                                    ),
                                    const SizedBox(height: 16),
                                    const Padding(
                                      padding: EdgeInsets.symmetric(horizontal: 12.0),
                                      child: Row(
                                        children: [
                                          Expanded(flex: 5, child: Text('Class')),
                                          Expanded(flex: 3, child: Text('Cost')),
                                          Expanded(flex: 3, child: Text('Effectivity')),
                                          SizedBox(width: 96)
                                        ]
                                      )
                                    ),
                                    const SizedBox(height: 8),
                                    Expanded(
                                      // Notifier with current shop offers displayed in the list
                                      child: ValueListenableBuilder<List<UpgradeOffer>>(
                                        valueListenable: controller.items,
                                        builder: (context, items, _) => Column(
                                          children: [
                                            const SizedBox(height: 8),
                                            Expanded(
                                              // Notifier holding the offer id that caused the last error (used for row highlighting)
                                              child: ValueListenableBuilder<String?>(
                                                valueListenable: controller.errorOfferId,
                                                builder: (context, errorId, __) => ListView.separated(
                                                  itemCount: items.length,
                                                  separatorBuilder: (_, __) => const SizedBox(height: 16),
                                                  itemBuilder: (_, index) => UpgradeOfferRow(
                                                    upgradeOffer: items[index],
                                                    onBuy: () => controller.buy(context, items[index]),
                                                    highlight: items[index].id == errorId,
                                                    errorMessage: items[index].id == errorId ? controller.errorMessage.value : null,
                                                  )
                                                )
                                              )
                                            )
                                          ]
                                        )
                                      )
                                    )
                                  ]
                                )
                              )
                            )
                          )
                        ]
                      )
                    )
                  )
                ]
              )
            ),
            Positioned(
              top: 8,
              right: 16,
              child: FilledButton(
                onPressed: () => controller.onBackToMainMenu(context),
                style: FilledButton.styleFrom(
                  backgroundColor: Colors.white,
                  overlayColor: const Color(0xFFD6D6D6),
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(8)
                  ),
                  shadowColor: Colors.black12
                ),
                child: const Text(
                  'Back to main menu',
                  style: TextStyle(color: Colors.black)
                ),
              )
            )
          ]
        )
      ),
    );
  }
}