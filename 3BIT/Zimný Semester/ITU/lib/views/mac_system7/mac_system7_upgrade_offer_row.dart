// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';
import '../../models/mac_system7/upgradeoffer.dart';

class MacSystem7UpgradeOfferRow extends StatelessWidget {
  final UpgradeOffer upgradeOffer;
  final VoidCallback onBuy;
  final bool isEven;
  final bool showCost;

  const MacSystem7UpgradeOfferRow({
    super.key,
    required this.upgradeOffer,
    required this.onBuy,
    this.isEven = false,
    this.showCost = false,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: isEven ? const Color(0xFFF4F4F4) : const Color(0xFFE0E0E0), // alternate row color
        border: Border.all(color: Colors.black, width: 1),
      ),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
        child: Row(
          children: [
            Expanded(
              flex: 5,
              child: Text(
                upgradeOffer.name,
                style: const TextStyle(
                  fontFamily: 'Courier',
                  fontWeight: FontWeight.w700,
                  fontSize: 18,
                  color: Colors.black,
                ),
              ),
            ),
            Expanded(
              flex: 3,
              child: Text(
                '${upgradeOffer.productionUxPointsPerSecond} UX/s',
                style: const TextStyle(
                  fontFamily: 'Courier',
                  fontSize: 16,
                  color: Colors.black,
                ),
              ),
            ),
            if (showCost)
              Expanded(
                flex: 3,
                child: Text(
                  '${upgradeOffer.cost} UX',
                  style: const TextStyle(
                    fontFamily: 'Courier',
                    fontWeight: FontWeight.w700,
                    fontSize: 16,
                    color: Colors.black,
                  ),
                ),
              ),
            SizedBox(
              width: 100,
              child: GestureDetector(
                onTap: onBuy,
                child: Container(
                  padding: const EdgeInsets.symmetric(vertical: 4, horizontal: 8),
                  decoration: BoxDecoration(
                    border: Border.all(color: Colors.black, width: 1),
                    color: isEven ? const Color(0xFFF4F4F4) : const Color(0xFFE0E0E0),
                  ),
                  child: const Text(
                    '<buy>',
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      fontFamily: 'Courier',
                      fontWeight: FontWeight.w700,
                      fontSize: 16,
                      color: Colors.black,
                    ),
                  ),
                ),
              ),
            )
          ],
        ),
      ),
    );
  }
}