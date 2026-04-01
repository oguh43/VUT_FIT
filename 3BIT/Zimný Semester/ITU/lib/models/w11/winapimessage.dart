// Filip Jenis, xjenisf00

class WinAPIMessage{
  final String id;
  final String title;
  final String? subtitle;

  const WinAPIMessage({
    required this.id,
    required this.title,
    this.subtitle
  });
}