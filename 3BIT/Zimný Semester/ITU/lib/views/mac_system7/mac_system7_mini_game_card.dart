// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';
import '../../models/mac_system7/minigame.dart';

class MacSystem7MiniGameCard extends StatelessWidget {
  final MiniGame minigame;
  final VoidCallback onClick;

  const MacSystem7MiniGameCard({
    super.key,
    required this.minigame,
    required this.onClick
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onClick,
      child: Container(
        decoration: BoxDecoration(
          color: Colors.white,
          border: Border.all(color: Colors.black, width: 1),
        ),
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 18),
          child: Center(
            child: Text(
              minigame.title,
              textAlign: TextAlign.center,
              style: const TextStyle(
                fontSize: 24,
                fontWeight: FontWeight.w700,
                fontFamily: 'Courier',
                color: Colors.black,
              ),
            ),
          ),
        ),
      ),
    );
  }
}