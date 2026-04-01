// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../../models/mac_system7/minigame.dart';

// A tappable card that represents a mini-game. Shows the mini-game title and notifies when selected
class MiniGameCard extends StatelessWidget {
  final MiniGame minigame;
  final VoidCallback onClick;

  const MiniGameCard({
    super.key,
    required this.minigame,
    required this.onClick
  });

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 0,
      color: Colors.white,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(8)
      ),
      child: InkWell(
        borderRadius: BorderRadius.circular(8),
        onTap: onClick,
        hoverColor: const Color(0xFFD6D6D6),
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 26),
          child: Center(
            child: Text(
              minigame.title,
              textAlign: TextAlign.center,
              style: const TextStyle(
                fontSize: 22,
                fontWeight: FontWeight.w500
              ),
            )
          )
        )
      )
    );
  }
}