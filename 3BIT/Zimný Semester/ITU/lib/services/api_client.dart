import 'dart:convert';
import 'package:http/http.dart' as http;

class ApiException implements Exception {
  final int statusCode;
  final String body;
  ApiException(this.statusCode, this.body);
  @override
  String toString() => 'ApiException($statusCode): $body';
}

class ApiClient {
  final String baseUrl;
  final http.Client _client;

  ApiClient({
    // Add the required keyword to baseUrl
    required this.baseUrl,
    http.Client? client,
  }) : _client = client ?? http.Client();

  Future<dynamic> get(String path) async {
    final uri = Uri.parse('$baseUrl$path');
    final response = await _client.get(uri);
    _check(response);
    return jsonDecode(response.body);
  }

  Future<dynamic> post(String path, {Map<String, dynamic>? body}) async {
    final uri = Uri.parse('$baseUrl$path');
    final response = await _client.post(
      uri,
      headers: {'Content-Type': 'application/json'},
      body: body != null ? jsonEncode(body) : null,
    );
    _check(response);
    return jsonDecode(response.body);
  }

  void _check(http.Response r) {
    if (r.statusCode < 200 || r.statusCode >= 300) {
      print('API error ${r.statusCode}: ${r.body}');
      throw ApiException(r.statusCode, r.body);
    }
  }
}
