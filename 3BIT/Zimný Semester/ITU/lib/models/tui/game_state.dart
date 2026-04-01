/*******************************************************************************
*                                                                              *
*                        Brno University of Technology                         *
*                      Faculty of Information Technology                       *
*                                                                              *
*                       Tvorba uživatelských rozhraní                          *
*                                                                              *
*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
*                                   Brno 2025                                  *
*                                                                              *
*               Implementation of the TUI main screen controller               *
*                                                                              *
*******************************************************************************/
import 'dart:async';
import 'package:flutter/foundation.dart';
import '../../repositories/gamerepository.dart';
import '../mac_system7/clickergamestate.dart';
import '../mac_system7/upgradeoffer.dart';

class GameState extends ChangeNotifier {
  final GameRepository _repository;

  ClickerGameState? _currentState;
  List<UpgradeOffer>? _shopOffers;
  bool _isLoading = false;
  String? _error;
  Timer? _passiveTimer;

  GameState(this._repository) {
    _initialize();
  }

  // Getters
  ClickerGameState? get currentState => _currentState;
  List<UpgradeOffer>? get shopOffers => _shopOffers;
  bool get isLoading => _isLoading;
  String? get error => _error;

  double get uxPoints => _currentState?.availableUxPoints.toDouble() ?? 0;
  double get productionRate => _currentState?.productionUxPointsPerSecond ?? 0;
  double get bonusRate => _currentState?.bonusUxPointsPerSecond ?? 0;

  Future<void> _initialize() async {
    await fetchGameState();
    await fetchShopOffers();
    _startPassiveGeneration();
  }

  /// Fetch current game state from the server
  Future<void> fetchGameState() async {
    _setLoading(true);
    try {
      _currentState = await _repository.fetchGameState();
      _error = null;
    } catch (e) {
      _error = 'Failed to fetch game state: $e';
      debugPrint(_error);
    } finally {
      _setLoading(false);
    }
  }

  /// Fetch available shop offers
  Future<void> fetchShopOffers() async {
    try {
      _shopOffers = await _repository.fetchShopOffers();
      notifyListeners();
    } catch (e) {
      _error = 'Failed to fetch shop offers: $e';
      debugPrint(_error);
      notifyListeners();
    }
  }

  /// Click the design button to gain UX points
  Future<void> clickDesignButton() async {
    try {
      _currentState = await _repository.buttonClick();
      _error = null;
      notifyListeners();
    } catch (e) {
      _error = 'Failed to click button: $e';
      debugPrint(_error);
      notifyListeners();
    }
  }

  /// Buy an upgrade item from the shop
  Future<void> buyUpgrade(String itemId) async {
    if (_isLoading) return;

    _setLoading(true);
    try {
      _currentState = await _repository.buyItem(itemId);
      _error = null;

      // Refresh shop offers to get updated costs
      await fetchShopOffers();
    } catch (e) {
      _error = 'Failed to buy upgrade: $e';
      debugPrint(_error);
    } finally {
      _setLoading(false);
    }
  }

  /// Complete a minigame with a multiplier bonus
  Future<void> completeMinigame(double multiplier) async {
    if (_isLoading) return;

    _setLoading(true);
    try {
      _currentState = await _repository.completeMinigame(multiplier);
      _error = null;
    } catch (e) {
      _error = 'Failed to complete minigame: $e';
      debugPrint(_error);
    } finally {
      _setLoading(false);
    }
  }

  /// Get upgrade offer by ID
  UpgradeOffer? getUpgradeOfferById(String id) {
    if (_shopOffers == null) return null;
    try {
      return _shopOffers!.firstWhere((offer) => offer.id == id);
    } catch (e) {
      return null;
    }
  }

  /// Check if user can afford an upgrade
  bool canAfford(int cost) {
    return uxPoints >= cost;
  }

  /// Start passive UX point generation by polling the server
  void _startPassiveGeneration() {
    _passiveTimer = Timer.periodic(const Duration(seconds: 2), (timer) async {
      if (!_isLoading) {
        try {
          _currentState = await _repository.fetchGameState();
          notifyListeners();
        } catch (e) {
          // Silently fail for passive updates
          debugPrint('Passive update failed: $e');
        }
      }
    });
  }

  void _setLoading(bool loading) {
    _isLoading = loading;
    notifyListeners();
  }

  @override
  void dispose() {
    _passiveTimer?.cancel();
    super.dispose();
  }
}
