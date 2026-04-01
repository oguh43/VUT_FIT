// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';
import '../../controllers/mac_system7/upgradeshop.dart';
import '../../models/mac_system7/upgradeoffer.dart';
import '../../models/mac_system7/clickergamestate.dart';
import 'mac_system7_upgrade_offer_row.dart';

class MacSystem7UpgradeShopScreen extends StatelessWidget {
  final UpgradeShop controller;

  const MacSystem7UpgradeShopScreen({super.key, required this.controller});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF2CA6A4),
      body: SafeArea(
        child: Stack(
          children: [
            Column(
              children: [
                _buildMenuBar(context),
              ],
            ),
            Center(
              child: Container(
                margin: const EdgeInsets.symmetric(horizontal: 64, vertical: 64),
                padding: const EdgeInsets.all(0),
                decoration: BoxDecoration(
                  color: Colors.white,
                  border: Border.all(color: Colors.black, width: 2),
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black.withOpacity(0.2),
                      offset: const Offset(2, 2),
                      blurRadius: 0,
                    ),
                  ],
                ),
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    _windowHeader(),
                    Padding(
                      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                      child: ValueListenableBuilder<ClickerGameState>(
                        valueListenable: controller.gameController.state,
                        builder: (_, state, __) => Text(
                          'Available UX points: ${state.availableUxPoints}',
                          style: const TextStyle(
                            fontSize: 18,
                            fontFamily: 'Courier',
                            fontWeight: FontWeight.w700,
                            color: Colors.black,
                          ),
                        ),
                      ),
                    ),
                    _tableHeader(),
                    Expanded(
                      child: ValueListenableBuilder<List<UpgradeOffer>>(
                        valueListenable: controller.items,
                        builder: (context, items, _) => ListView.builder(
                          itemCount: items.length,
                          itemBuilder: (context, index) => MacSystem7UpgradeOfferRow(
                            upgradeOffer: items[index],
                            onBuy: () => controller.buy(context, items[index]),
                            isEven: index % 2 == 0,
                            showCost: true,
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _windowHeader() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Container(
          color: const Color(0xFFE0E0E0),
          padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
          child: Row(
            children: [
              Container(
                width: 18,
                height: 18,
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.black54, width: 1),
                  color: Colors.grey[300],
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: Row(
                  children: [
                    Expanded(
                      child: Column(
                        children: List.generate(6, (i) => Divider(thickness: 2, color: Colors.black12, height: 3)),
                      ),
                    ),
                    Padding(
                      padding: const EdgeInsets.symmetric(horizontal: 12),
                      child: Text('Upgrades', style: TextStyle(fontFamily: 'Courier', fontWeight: FontWeight.w700, fontSize: 16)),
                    ),
                    Expanded(
                      child: Column(
                        children: List.generate(6, (i) => Divider(thickness: 2, color: Colors.black12, height: 3)),
                      ),
                    ),
                  ],
                ),
              ),
              Container(
                width: 18,
                height: 18,
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.black12, width: 1),
                  color: Colors.grey[200],
                ),
              ),
            ],
          ),
        ),
        Container(
          height: 1,
          color: Colors.black,
        ),
      ],
    );
  }

  Widget _tableHeader() {
    return Container(
      color: Colors.white,
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
      child: Row(
        children: [
          Expanded(flex: 5, child: Text('Name', style: TextStyle(fontFamily: 'Courier', fontWeight: FontWeight.w700, color: Colors.black))),
          Expanded(flex: 3, child: Text('Production', style: TextStyle(fontFamily: 'Courier', fontWeight: FontWeight.w700, color: Colors.black))),
          Expanded(flex: 3, child: Text('Cost/Unit', style: TextStyle(fontFamily: 'Courier', fontWeight: FontWeight.w700, color: Colors.black))),
          SizedBox(width: 100)
        ],
      ),
    );
  }

  Widget _buildMenuBar(BuildContext context) {
    return InkWell(
      onTap: () {
        Navigator.of(context).pushNamedAndRemoveUntil('/', (route) => false);
      },
      child: Container(
        color: Colors.white,
        padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
        child: Row(
          children: [
            const Icon(Icons.info_outline, size: 18, color: Colors.black),
            const SizedBox(width: 8),
            ...['File', 'Edit', 'View', 'Label', 'Special'].map((item) => Padding(
              padding: const EdgeInsets.symmetric(horizontal: 8),
              child: Text(item, style: const TextStyle(fontFamily: 'Courier', fontWeight: FontWeight.w700, fontSize: 14)),
            )),
            const Spacer(),
            Text(_getTimeString(), style: const TextStyle(fontFamily: 'Courier', fontWeight: FontWeight.w700, fontSize: 14)),
            const SizedBox(width: 8),
            const Icon(Icons.help_outline, size: 18, color: Colors.black),
          ],
        ),
      ),
    );
  }

  String _getTimeString() {
    final now = DateTime.now();
    return '${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')} PM';
  }
}