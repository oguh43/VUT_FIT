// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';
import 'dart:async';
import '../../models/mac_system7/cors_request.dart';
import '../../repositories/gamerepository.dart';

class MacSystem7CorsMinigame extends StatefulWidget {
  final GameRepository repository;

  const MacSystem7CorsMinigame({super.key, required this.repository});

  @override
  State<MacSystem7CorsMinigame> createState() => _MacSystem7CorsMinigameState();
}

class _MacSystem7CorsMinigameState extends State<MacSystem7CorsMinigame> {
  late List<CorsRequest> requests;
  int currentIndex = 0;
  int score = 0;
  bool gameOver = false;
  double multiplier = 1.0;
  Timer? _requestTimer;
  Timer? _countdownTimer;
  bool _isAnswering = false;
  int _remainingSeconds = 5;

  @override
  void initState() {
    super.initState();
    requests = _generateRequests();
    _startGame();
  }

  @override
  void dispose() {
    _requestTimer?.cancel();
    _countdownTimer?.cancel();
    super.dispose();
  }

  List<CorsRequest> _generateRequests() {
    return [
      CorsRequest(
        origin: 'https://example.com',
        method: 'GET',
        headers: {'Origin': 'https://example.com'},
        shouldAllow: true,
      ),
      CorsRequest(
        origin: 'https://evil.com',
        method: 'POST',
        headers: {'Origin': 'https://evil.com'},
        shouldAllow: false,
      ),
      CorsRequest(
        origin: 'https://sub.example.com',
        method: 'GET',
        headers: {'Origin': 'https://sub.example.com'},
        shouldAllow: false, // subdomain
      ),
      CorsRequest(
        origin: 'http://example.com',
        method: 'GET',
        headers: {'Origin': 'http://example.com'},
        shouldAllow: false, // different protocol
      ),
      CorsRequest(
        origin: 'https://example.com:8080',
        method: 'GET',
        headers: {'Origin': 'https://example.com:8080'},
        shouldAllow: false, // different port
      ),
      CorsRequest(
        origin: 'https://example.com',
        method: 'GET',
        headers: {}, // missing origin
        shouldAllow: false,
      ),
      CorsRequest(
        origin: 'https://trusted.com',
        method: 'GET',
        headers: {'Origin': 'https://trusted.com'},
        shouldAllow: true,
      ),
      CorsRequest(
        origin: 'https://example.com',
        method: 'OPTIONS',
        headers: {'Origin': 'https://example.com'},
        shouldAllow: true,
      ),
      CorsRequest(
        origin: 'https://phishing.com',
        method: 'GET',
        headers: {'Origin': 'https://phishing.com'},
        shouldAllow: false,
      ),
      CorsRequest(
        origin: 'https://example.com',
        method: 'PUT',
        headers: {'Origin': 'https://example.com'},
        shouldAllow: false, // method not allowed
      ),
    ];
  }

  void _startGame() {
    _showNextRequest();
  }

  void _showNextRequest() {
    if (currentIndex >= requests.length) {
      // Game over
      if (mounted) {
        setState(() {
          gameOver = true;
          multiplier = 1.0 + (score / requests.length) * 0.5;
        });
      }
      return;
    }

    // Reset countdown
    setState(() {
      _remainingSeconds = 5;
    });

    // Start countdown timer that updates every second
    _countdownTimer?.cancel();
    _countdownTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
      if (mounted && _remainingSeconds > 0) {
        setState(() {
          _remainingSeconds--;
        });
      }
    });

    // Set timer for 5 seconds
    _requestTimer?.cancel();
    _requestTimer = Timer(const Duration(seconds: 5), () {
      if (!_isAnswering && mounted) {
        _countdownTimer?.cancel();
        // Time's up, count as wrong
        setState(() {
          currentIndex++;
        });
        _showNextRequest();
      }
    });
  }

  void _decide(bool allow) {
    if (_isAnswering || gameOver) return;

    _isAnswering = true;
    _requestTimer?.cancel();
    _countdownTimer?.cancel();

    final correct = requests[currentIndex].isCorrectDecision(allow);
    if (correct) {
      setState(() {
        score++;
      });
    }

    // Move to next request after a brief delay
    Future.delayed(const Duration(milliseconds: 300), () {
      if (mounted) {
        setState(() {
          currentIndex++;
          _isAnswering = false;
        });
        _showNextRequest();
      }
    });
  }

  Widget _buildGameOverWindowHeader() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Container(
          color: const Color(0xFFE0E0E0),
          padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
          child: Row(
            children: [
              Container(
                width: 18,
                height: 18,
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.black54, width: 1),
                  color: Colors.grey[300],
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: Row(
                  children: [
                    Expanded(
                      child: Column(
                        children: List.generate(
                          6,
                          (i) => Divider(
                            thickness: 2,
                            color: Colors.black12,
                            height: 3,
                          ),
                        ),
                      ),
                    ),
                    Padding(
                      padding: const EdgeInsets.symmetric(horizontal: 12),
                      child: Text(
                        'Game Over',
                        style: TextStyle(
                          fontFamily: 'Courier',
                          fontWeight: FontWeight.w700,
                          fontSize: 16,
                        ),
                      ),
                    ),
                    Expanded(
                      child: Column(
                        children: List.generate(
                          6,
                          (i) => Divider(
                            thickness: 2,
                            color: Colors.black12,
                            height: 3,
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
              Container(
                width: 18,
                height: 18,
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.black12, width: 1),
                  color: Colors.grey[200],
                ),
              ),
            ],
          ),
        ),
        Container(height: 1, color: Colors.black),
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF2CA6A4),
      body: SafeArea(
        child: Stack(
          children: [
            if (!gameOver)
              Center(
                child: Container(
                  margin: const EdgeInsets.symmetric(horizontal: 32),
                  constraints: const BoxConstraints(maxWidth: 500),
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      // Score display
                      Container(
                        padding: const EdgeInsets.symmetric(
                          horizontal: 16,
                          vertical: 8,
                        ),
                        decoration: BoxDecoration(
                          color: Colors.white,
                          border: Border.all(color: Colors.black, width: 2),
                          boxShadow: [
                            BoxShadow(
                              color: Colors.black.withOpacity(0.2),
                              offset: const Offset(2, 2),
                              blurRadius: 0,
                            ),
                          ],
                        ),
                        child: Text(
                          'Score: $score / ${requests.length}',
                          style: const TextStyle(
                            fontSize: 18,
                            fontFamily: 'Courier',
                            fontWeight: FontWeight.bold,
                            color: Colors.black,
                          ),
                          textAlign: TextAlign.center,
                        ),
                      ),
                      const SizedBox(height: 24),
                      // Current request card
                      if (currentIndex < requests.length)
                        _RequestCard(
                          request: requests[currentIndex],
                          onAllow: () => _decide(true),
                          onBlock: () => _decide(false),
                        ),
                      const SizedBox(height: 16),
                      // Progress bar
                      if (currentIndex < requests.length)
                        Container(
                          padding: const EdgeInsets.all(16),
                          decoration: BoxDecoration(
                            color: Colors.white,
                            border: Border.all(color: Colors.black, width: 2),
                            boxShadow: [
                              BoxShadow(
                                color: Colors.black.withOpacity(0.2),
                                offset: const Offset(2, 2),
                                blurRadius: 0,
                              ),
                            ],
                          ),
                          child: Column(
                            children: [
                              Text(
                                'Time Remaining: ${_remainingSeconds}s',
                                style: const TextStyle(
                                  fontSize: 14,
                                  fontFamily: 'Courier',
                                  fontWeight: FontWeight.bold,
                                  color: Colors.black,
                                ),
                              ),
                              const SizedBox(height: 8),
                              LinearProgressIndicator(
                                value: _remainingSeconds / 5.0,
                                backgroundColor: Colors.grey[300],
                                valueColor: AlwaysStoppedAnimation<Color>(
                                  Colors.green[400]!,
                                ),
                                minHeight: 8,
                              ),
                            ],
                          ),
                        ),
                    ],
                  ),
                ),
              ),
            if (gameOver)
              Center(
                child: Container(
                  margin: const EdgeInsets.symmetric(
                    horizontal: 32,
                    vertical: 16,
                  ),
                  padding: const EdgeInsets.all(0),
                  decoration: BoxDecoration(
                    color: Colors.white,
                    border: Border.all(color: Colors.black, width: 2),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black.withOpacity(0.2),
                        offset: const Offset(2, 2),
                        blurRadius: 0,
                      ),
                    ],
                  ),
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [
                      _buildGameOverWindowHeader(),
                      Padding(
                        padding: const EdgeInsets.symmetric(
                          horizontal: 16,
                          vertical: 16,
                        ),
                        child: Column(
                          mainAxisSize: MainAxisSize.min,
                          children: [
                            Text(
                              'Game Over',
                              style: TextStyle(
                                fontSize: 24,
                                fontWeight: FontWeight.bold,
                                fontFamily: 'Courier',
                              ),
                            ),
                            const SizedBox(height: 16),
                            Text(
                              'Score: $score / ${requests.length}',
                              style: const TextStyle(
                                fontSize: 18,
                                fontFamily: 'Courier',
                              ),
                            ),
                            const SizedBox(height: 8),
                            Text(
                              'Multiplier: ${multiplier.toStringAsFixed(2)}x',
                              style: const TextStyle(
                                fontSize: 18,
                                fontFamily: 'Courier',
                              ),
                            ),
                            const SizedBox(height: 16),
                            OutlinedButton(
                              onPressed: () async {
                                try {
                                  // Call the API to submit the bonus
                                  await widget.repository.completeMinigame(
                                    multiplier,
                                  );
                                } catch (e) {
                                  print('Error completing minigame: $e');
                                  // Show error to user if needed
                                  if (context.mounted) {
                                    ScaffoldMessenger.of(context).showSnackBar(
                                      SnackBar(
                                        content: Text('Error saving bonus: $e'),
                                      ),
                                    );
                                  }
                                }
                                if (context.mounted) {
                                  Navigator.of(context).pop();
                                }
                              },
                              style: OutlinedButton.styleFrom(
                                side: const BorderSide(
                                  width: 1.5,
                                  color: Colors.black12,
                                ),
                                backgroundColor: Colors.white,
                                shape: RoundedRectangleBorder(
                                  borderRadius: BorderRadius.zero,
                                ),
                                textStyle: const TextStyle(
                                  fontSize: 18,
                                  fontWeight: FontWeight.w400,
                                ),
                              ),
                              child: const Text('Back'),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
                ),
              ),
          ],
        ),
      ),
    );
  }
}

class _RequestCard extends StatelessWidget {
  final CorsRequest request;
  final VoidCallback onAllow;
  final VoidCallback onBlock;

  const _RequestCard({
    required this.request,
    required this.onAllow,
    required this.onBlock,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(0),
      decoration: BoxDecoration(
        color: Colors.white,
        border: Border.all(color: Colors.black, width: 2),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.2),
            offset: const Offset(2, 2),
            blurRadius: 0,
          ),
        ],
      ),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          _buildWindowHeader(),
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
            child: Text(
              request.displayText,
              style: const TextStyle(
                fontSize: 14,
                fontFamily: 'Courier',
                fontWeight: FontWeight.w700,
                color: Colors.black,
              ),
            ),
          ),
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  onPressed: onAllow,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.green,
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.zero,
                    ),
                    textStyle: const TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.w500,
                    ),
                  ),
                  child: const Text('Allow'),
                ),
                OutlinedButton(
                  onPressed: onBlock,
                  style: OutlinedButton.styleFrom(
                    side: const BorderSide(width: 1.5, color: Colors.black12),
                    backgroundColor: Colors.white,
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.zero,
                    ),
                    textStyle: const TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.w400,
                    ),
                  ),
                  child: const Text('Block'),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildWindowHeader() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Container(
          color: const Color(0xFFE0E0E0),
          padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
          child: Row(
            children: [
              Container(
                width: 18,
                height: 18,
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.black54, width: 1),
                  color: Colors.grey[300],
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: Row(
                  children: [
                    Expanded(
                      child: Column(
                        children: List.generate(
                          6,
                          (i) => Divider(
                            thickness: 2,
                            color: Colors.black12,
                            height: 3,
                          ),
                        ),
                      ),
                    ),
                    Padding(
                      padding: const EdgeInsets.symmetric(horizontal: 12),
                      child: Text(
                        'CORS Request',
                        style: TextStyle(
                          fontFamily: 'Courier',
                          fontWeight: FontWeight.w700,
                          fontSize: 14,
                        ),
                      ),
                    ),
                    Expanded(
                      child: Column(
                        children: List.generate(
                          6,
                          (i) => Divider(
                            thickness: 2,
                            color: Colors.black12,
                            height: 3,
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
              Container(
                width: 18,
                height: 18,
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.black12, width: 1),
                  color: Colors.grey[200],
                ),
              ),
            ],
          ),
        ),
        Container(height: 1, color: Colors.black),
      ],
    );
  }
}
