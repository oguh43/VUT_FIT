// Filip Jenis, xjenisf00

import 'package:flutter/material.dart';
import '../../../models/w11/winapimessage.dart';

// Small card widget showing a WinAPIMessage title and optional subtitle, used in the W11 WinAPI mini-game message pool/assignments
class WinAPIMessageCard extends StatelessWidget {
  final WinAPIMessage message;
  final bool ghost;

  const WinAPIMessageCard({super.key, required this.message, this.ghost = false});

  @override
  Widget build(BuildContext context) {
    return Opacity(
      opacity: ghost ? 0.85 : 1.0,
      child: Card(
        elevation: 6,
        shadowColor: Colors.black12,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
        child: Container(
          width: 180,
          padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(message.title,
                  style: const TextStyle(
                      fontWeight: FontWeight.w600, fontSize: 14)),
              if (message.subtitle != null) ...[
                const SizedBox(height: 6),
                Text(
                  message.subtitle!,
                  style: const TextStyle(fontSize: 12, color: Colors.black54),
                ),
              ]
            ],
          ),
        ),
      ),
    );
  }
}
