// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../../models/mac_system7/upgradeoffer.dart';

// Row widget that shows a single UpgradeOffer with cost, production and a Buy button
class UpgradeOfferRow extends StatelessWidget{
  final UpgradeOffer upgradeOffer;
  final VoidCallback onBuy;
  final bool highlight;
  final String? errorMessage;

  const UpgradeOfferRow({
    super.key,
    required this.upgradeOffer,
    required this.onBuy,
    this.highlight = false,
    this.errorMessage
  });

  @override
  Widget build(BuildContext context) {
    return Column( 
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Container(
          decoration: BoxDecoration(
            boxShadow: const [
              BoxShadow(
                color: Colors.black12,
                blurRadius: 8,
                offset: Offset(0, 2)
              )
            ]
          ),
          child: Card(
            elevation: 6,
            shadowColor: Colors.black12,
            color: highlight ? const Color(0xFFFFECEC) : const Color(0xFFF3F3F3),
            shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
            child: Padding(
              padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 12),
              child: Row(
                children: [
                  Expanded(flex: 5, child: Text(upgradeOffer.name)),
                  Expanded(flex: 3, child: Text('${upgradeOffer.cost} UX')),
                  Expanded(flex: 3, child: Text('${upgradeOffer.productionUxPointsPerSecond} UX/s')),
                  SizedBox(
                    width: 96,
                    child: ElevatedButton(
                      onPressed: onBuy,
                      style: ElevatedButton.styleFrom(
                        backgroundColor: const Color(0xFF005FB8)
                      ),
                      child: const Text('Buy', style: TextStyle(color: Colors.white, fontWeight: FontWeight.w400))
                    )
                  )
                ]
              )
            )
          )
        ),
        if (errorMessage != null)
          Padding(
            padding: const EdgeInsets.only(left: 12.0, top: 6.0),
            child: Text(errorMessage!, style: const TextStyle(color: Colors.red, fontSize: 12))
          )
      ]
    );
  }
}