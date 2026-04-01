// Stefan Dubnicka, xdubnis00

class Upgrade {
  final String name;
  final int count;
  final double uxPointsPerSecond;

  const Upgrade({
    required this.name,
    required this.count,
    required this.uxPointsPerSecond
  });

  String get label => '$name: ${count}x';

  String get uxPointsProductionLabel => '${uxPointsPerSecond.toStringAsFixed(0)} UX/s';
}