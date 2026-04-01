// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';

// Left-side navigation drawer for the W11 UI
class SideDrawer extends StatelessWidget {
  const SideDrawer({super.key});

  // Returns true when the current route name equals [routeName]
  bool isCurrentRoute(BuildContext context, String routeName) {
    return ModalRoute.of(context)?.settings.name == routeName;
  }

  // Close the drawer and navigate to [routeName] unless already on it
  void redirectTo(BuildContext context, String routeName) {
    Navigator.pop(context);
    if (isCurrentRoute(context, routeName)) return;
    Navigator.of(context).pushNamed(routeName);
  }

  @override
  Widget build(BuildContext context) {
    final mainScreen = isCurrentRoute(context, '/w11');
    final upgradeShopScreen = isCurrentRoute(context, '/w11/upgrades');

    return Drawer(
      width: 260,
      backgroundColor: const Color(0xFFF3F3F3),
      shape: const RoundedRectangleBorder(
        borderRadius: BorderRadius.zero
      ),
      clipBehavior: Clip.hardEdge,
      child: SafeArea(
        child: ListTileTheme(
          selectedColor: const Color(0xFFF3F3F3),
          selectedTileColor: const Color(0xFFD6D6D6),
          child: ListView(
            padding: const EdgeInsets.all(8),
            children: [
              ListTile(
                title: const Text('Main menu', style: TextStyle(color: Colors.black)),
                selected: mainScreen,
                onTap: () => redirectTo(context, '/w11'),
              ),
              const Divider(),
              ListTile(
                title: const Text('Upgrades', style: TextStyle(color: Colors.black)),
                selected: upgradeShopScreen,
                onTap: () => redirectTo(context, '/w11/upgrades'),
              )
            ],
          ),
        )
      )
    );
  }
}