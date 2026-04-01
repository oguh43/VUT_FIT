// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';

// Widget that displays the current bonus rate as a small chip
class BonusPointsChip extends StatelessWidget {
  final double bonus;

  const BonusPointsChip({super.key, required this.bonus});

  @override
  Widget build(BuildContext context) {
    return Container(
      key: const ValueKey('bonus'),
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
      decoration: BoxDecoration(
        color: const Color(0xFFDFF6DD),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Colors.black12),
      ),
      child: Text('Bonus: +${(bonus * 100).toStringAsFixed(0)}%'),
    );
  }
}
