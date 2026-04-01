// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../../models/mac_system7/clickergamestate.dart';
import '../../../controllers/w11/clickergame.dart';
import '../../../models/mac_system7/upgrade.dart';
import 'upgradecard.dart';

// Sidebar column that shows bought upgrades (Interns)
class UpgradesColumn extends StatelessWidget{
  // Controller that exposes ClickerGameState and actions related to upgrades
  final W11ClickerGame controller;

  const UpgradesColumn({super.key, required this.controller});

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 260,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children:[
              Builder(
                builder: (context) => IconButton(
                  icon: const Icon(Icons.menu),
                  onPressed: () => Scaffold.of(context).openDrawer(),
                )
              ),
              const SizedBox(width: 4),
              Text('Upgrades', style: const TextStyle(
                fontSize: 28,
                fontWeight: FontWeight.w700
              ))
            ],
          ),
          const SizedBox(height: 16),
          Expanded(
            child: ValueListenableBuilder<ClickerGameState>(
              valueListenable: controller.state,
              builder: (_, state, __){
                return ListView.separated(
                  itemCount: state.upgradesList.length,
                  separatorBuilder: (_, __) => const SizedBox(height: 16),
                  itemBuilder: (context, index) {
                    return UpgradeCard(upgrade: state.upgradesList[index]);
                  },
                );
              }
            ),
          )
        ]
      )
    );
  }
}