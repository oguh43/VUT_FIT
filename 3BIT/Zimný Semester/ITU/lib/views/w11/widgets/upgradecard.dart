// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../../models/mac_system7/upgrade.dart';

// A small card widget that displays an Upgrade label and its UX production info
class UpgradeCard extends StatelessWidget{
  final Upgrade upgrade;

  const UpgradeCard({super.key, required this.upgrade});

  @override
  Widget build(BuildContext context) {
    // Build a flat, rounded card matching the W11 visual style
    return Card(
      elevation: 0,
      color: Colors.white,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(8)
      ),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 14),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                const Icon(Icons.person, size: 24),
                const SizedBox(width: 8),
                Text(
                  upgrade.label,
                  style: const TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.w600
                  ),
                )
              ]
            ),
            const SizedBox(height: 8),
            Align(
              alignment: Alignment.centerRight,
              child:
                Text(
                  upgrade.uxPointsProductionLabel,
                  style: const TextStyle(
                    fontSize: 14,
                    color: Colors.black54
                  ),
                )
            )
          ],
        )
      )
    );
  }
}