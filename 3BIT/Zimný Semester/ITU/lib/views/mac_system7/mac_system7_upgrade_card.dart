// Stefan Dubnicka, xdubnis00

import 'dart:math';
import 'package:flutter/material.dart';
import '../../models/mac_system7/upgrade.dart';

class MacSystem7UpgradeCard extends StatefulWidget {
  final Upgrade upgrade;
  final bool unconstrained;

  const MacSystem7UpgradeCard({
    super.key,
    required this.upgrade,
    this.unconstrained = false,
  });

  @override
  State<MacSystem7UpgradeCard> createState() => _MacSystem7UpgradeCardState();
}

class _MacSystem7UpgradeCardState extends State<MacSystem7UpgradeCard>
    with TickerProviderStateMixin {
  late AnimationController _uxCounterController;
  late Animation<double> _uxCounterScaleAnimation;
  bool _controllerInitialized = false;

  @override
  void initState() {
    super.initState();
    _updateAnimation();
  }

  @override
  void didUpdateWidget(MacSystem7UpgradeCard oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.upgrade.uxPointsPerSecond !=
        widget.upgrade.uxPointsPerSecond) {
      _updateAnimation();
    }
  }

  void _updateAnimation() {
    // Only dispose if controller has been initialized
    if (_controllerInitialized) {
      _uxCounterController.dispose();
    }

    // Calculate duration for logarithmic scaling of bounces per second
    // log(uxPointsPerSecond + 1) gives smooth scaling: 1→0.69, 10→2.99, 100→5.0 bounces/sec
    final bouncesPerSecond = widget.upgrade.uxPointsPerSecond > 0
        ? log(widget.upgrade.uxPointsPerSecond + 1)
        : 0.25;
    final millisecondsPerBounce = (1000 / bouncesPerSecond).round();

    _uxCounterController = AnimationController(
      duration: Duration(milliseconds: millisecondsPerBounce),
      vsync: this,
    )..repeat(reverse: true); // Continuous bouncing

    _uxCounterScaleAnimation = Tween<double>(begin: 1.0, end: 1.1).animate(
      CurvedAnimation(parent: _uxCounterController, curve: Curves.elasticOut),
    );

    _controllerInitialized = true;
  }

  @override
  void dispose() {
    _uxCounterController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final card = Padding(
      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 20),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.center,
        children: [
          Expanded(
            child: Row(
              children: [
                Expanded(
                  child: Text(
                    widget.upgrade.name,
                    style: const TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.w700,
                      fontFamily: 'Courier',
                      color: Colors.black,
                    ),
                    overflow: TextOverflow.ellipsis,
                  ),
                ),
                Container(
                  width: 1,
                  margin: const EdgeInsets.symmetric(vertical: 2),
                  color: Colors.black26,
                ),
                const SizedBox(width: 8),
                Text(
                  '${widget.upgrade.count}x',
                  style: const TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.w700,
                    fontFamily: 'Courier',
                    color: Colors.black,
                  ),
                ),
              ],
            ),
          ),
          Container(
            width: 1,
            margin: const EdgeInsets.symmetric(vertical: 2),
            color: Colors.black26,
          ),
          const SizedBox(width: 16),
          SizedBox(
            width: 120,
            child: Align(
              alignment: Alignment.centerRight,
              child: AnimatedBuilder(
                animation: _uxCounterScaleAnimation,
                builder: (context, child) {
                  return Transform.scale(
                    scale: _uxCounterScaleAnimation.value,
                    child: child,
                  );
                },
                child: Text(
                  '${widget.upgrade.uxPointsPerSecond} UX/s',
                  style: const TextStyle(
                    fontSize: 16,
                    fontWeight: FontWeight.w700,
                    fontFamily: 'Courier',
                    color: Colors.black,
                  ),
                  overflow: TextOverflow.ellipsis,
                ),
              ),
            ),
          ),
        ],
      ),
    );

    if (widget.unconstrained) {
      return Container(
        decoration: BoxDecoration(
          color: Colors.white,
          border: Border.all(color: Colors.black, width: 1),
          boxShadow: [
            BoxShadow(
              color: Colors.black.withOpacity(0.2),
              offset: const Offset(2, 2),
              blurRadius: 0,
            ),
          ],
        ),
        child: card,
      );
    } else {
      return Container(
        decoration: BoxDecoration(
          color: Colors.white,
          border: Border.all(color: Colors.black, width: 1),
          boxShadow: [
            BoxShadow(
              color: Colors.black.withOpacity(0.2),
              offset: const Offset(2, 2),
              blurRadius: 0,
            ),
          ],
        ),
        child: card,
      );
    }
  }
}
