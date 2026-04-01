// Stefan Dubnicka, xdubnis00

class CorsRequest {
  final String origin;
  final String method;
  final Map<String, String> headers;
  final bool shouldAllow; // correct decision

  const CorsRequest({
    required this.origin,
    required this.method,
    required this.headers,
    required this.shouldAllow,
  });

  // Simple CORS check logic (simplified for the game)
  bool isCorrectDecision(bool allow) {
    return allow == shouldAllow;
  }

  // For display
  String get displayText {
    return 'Origin: $origin\nMethod: $method\nHeaders: ${headers.entries.map((e) => '${e.key}: ${e.value}').join(', ')}';
  }
}