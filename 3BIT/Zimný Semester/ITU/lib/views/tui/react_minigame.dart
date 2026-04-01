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
*                  Implementation of the react minigame widget                 *
*                                                                              *
*******************************************************************************/
import 'dart:async';
import 'dart:math';
import 'package:flutter/material.dart';
import '../../repositories/gamerepository.dart';

class ReactMinigame extends StatefulWidget {
  final GameRepository repository;

  const ReactMinigame({Key? key, required this.repository}) : super(key: key);

  @override
  State<ReactMinigame> createState() => _ReactMinigameState();
}

class _ReactMinigameState extends State<ReactMinigame> {
  static const int gridSize = 8;
  final List<List<bool>> _grid = List.generate(
    gridSize,
    (_) => List.generate(gridSize, (_) => false),
  );

  final List<BlockPiece> _availablePieces = [];
  bool _gameComplete = false;
  int _score = 0;
  int _piecesPlaced = 0;
  final int _targetPieces = 8;
  int _startTime = 0;
  bool _isSubmitting = false;

  @override
  void initState() {
    super.initState();
    _initializeGame();
  }

  void _initializeGame() {
    _startTime = DateTime.now().millisecondsSinceEpoch;

    // Clear grid
    for (var row in _grid) {
      for (var i = 0; i < row.length; i++) {
        row[i] = false;
      }
    }

    // Generate random pieces
    _availablePieces.clear();
    final random = Random();
    for (int i = 0; i < 6; i++) {
      _availablePieces.add(_generateRandomPiece(random));
    }

    _hoverRow = null;
    _hoverCol = null;
    _draggingPiece = null;
    _gameComplete = false;
    _score = 0;
    _piecesPlaced = 0;
    _isSubmitting = false;
  }

  BlockPiece _generateRandomPiece(Random random) {
    final shapes = [
      // L-shape
      [
        [true, false],
        [true, false],
        [true, true],
      ],
      // T-shape
      [
        [true, true, true],
        [false, true, false],
      ],
      // Square
      [
        [true, true],
        [true, true],
      ],
      // Line (3)
      [
        [true, true, true],
      ],
      // Line (2)
      [
        [true, true],
      ],
      // Z-shape
      [
        [true, true, false],
        [false, true, true],
      ],
      // S-shape
      [
        [false, true, true],
        [true, true, false],
      ],
      // Corner
      [
        [true, true],
        [true, false],
      ],
      // Single
      [
        [true],
      ],
    ];

    final colors = [
      Colors.red[400]!,
      Colors.blue[400]!,
      Colors.green[400]!,
      Colors.orange[400]!,
      Colors.purple[400]!,
      Colors.teal[400]!,
      Colors.pink[400]!,
      Colors.amber[400]!,
    ];

    final shapeIndex = random.nextInt(shapes.length);
    final colorIndex = random.nextInt(colors.length);

    return BlockPiece(
      shape: shapes[shapeIndex],
      color: colors[colorIndex],
      id: random.nextInt(1000000),
    );
  }

  // Track hover position for preview
  int? _hoverRow;
  int? _hoverCol;
  BlockPiece? _draggingPiece;

  bool _canPlacePiece(BlockPiece piece, int startRow, int startCol) {
    for (int r = 0; r < piece.shape.length; r++) {
      for (int c = 0; c < piece.shape[r].length; c++) {
        if (piece.shape[r][c]) {
          final gridRow = startRow + r;
          final gridCol = startCol + c;

          // Check bounds
          if (gridRow < 0 ||
              gridRow >= gridSize ||
              gridCol < 0 ||
              gridCol >= gridSize) {
            return false;
          }

          // Check if cell is already filled
          if (_grid[gridRow][gridCol]) {
            return false;
          }
        }
      }
    }
    return true;
  }

  void _placePiece(BlockPiece piece, int startRow, int startCol) {
    for (int r = 0; r < piece.shape.length; r++) {
      for (int c = 0; c < piece.shape[r].length; c++) {
        if (piece.shape[r][c]) {
          _grid[startRow + r][startCol + c] = true;
        }
      }
    }

    _availablePieces.remove(piece);
    _piecesPlaced++;

    // Clear completed rows and columns
    _clearCompleteLines();

    if (_availablePieces.length < 3) {
      _availablePieces.add(_generateRandomPiece(Random()));
    }

    if (_piecesPlaced >= _targetPieces) {
      _completeGame();
    }

    setState(() {
      _hoverRow = null;
      _hoverCol = null;
      _draggingPiece = null;
    });
  }

  void _clearCompleteLines() {
    int linesCleared = 0;

    // Check rows
    for (int r = 0; r < gridSize; r++) {
      if (_grid[r].every((cell) => cell)) {
        for (int c = 0; c < gridSize; c++) {
          _grid[r][c] = false;
        }
        linesCleared++;
      }
    }

    // Check columns
    for (int c = 0; c < gridSize; c++) {
      bool columnFull = true;
      for (int r = 0; r < gridSize; r++) {
        if (!_grid[r][c]) {
          columnFull = false;
          break;
        }
      }
      if (columnFull) {
        for (int r = 0; r < gridSize; r++) {
          _grid[r][c] = false;
        }
        linesCleared++;
      }
    }

    if (linesCleared > 0) {
      _score += linesCleared * 50;
    }
  }

  void _completeGame() {
    final endTime = DateTime.now().millisecondsSinceEpoch;
    final timeSeconds = (endTime - _startTime) / 1000;

    // Base score for completion
    int finalScore = 300;

    // Bonus for lines cleared
    finalScore += _score;

    // Time multiplier
    if (timeSeconds < 30) {
      finalScore = (finalScore * 1.5).round();
    } else if (timeSeconds < 60) {
      finalScore = (finalScore * 1.2).round();
    }

    _score = finalScore;

    setState(() {
      _gameComplete = true;
    });
  }

  void _giveUp() {
    _score = -200;
    setState(() {
      _gameComplete = true;
    });
  }

  Future<void> _completeAndExit() async {
    if (_isSubmitting) return;

    setState(() {
      _isSubmitting = true;
    });

    // Calculate multiplier based on score
    // Positive scores get bonus multiplier, negative scores get penalty
    double multiplier = _score > 0 ? (_score / 100.0) : -2.0;

    try {
      await widget.repository.completeMinigame(multiplier);

      if (mounted) {
        Navigator.pop(context);
      }
    } catch (e) {
      // Show error dialog
      if (mounted) {
        showDialog(
          context: context,
          builder: (context) => AlertDialog(
            title: const Text('Error'),
            content: Text('Failed to submit minigame result: $e'),
            actions: [
              TextButton(
                onPressed: () => Navigator.pop(context),
                child: const Text('OK'),
              ),
            ],
          ),
        );
      }
    } finally {
      if (mounted) {
        setState(() {
          _isSubmitting = false;
        });
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFE3F2FD),
      appBar: AppBar(
        title: const Text('React Block Puzzle'),
        backgroundColor: const Color(0xFF1976D2),
        foregroundColor: Colors.white,
      ),
      body: _gameComplete ? _buildResultView() : _buildGameView(),
    );
  }

  Widget _buildGameView() {
    return Padding(
      padding: const EdgeInsets.all(16.0),
      child: Row(
        children: [
          // Left side - Available pieces
          Expanded(
            flex: 1,
            child: Container(
              padding: const EdgeInsets.all(16),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(12),
                boxShadow: [
                  BoxShadow(
                    color: Colors.black.withOpacity(0.1),
                    blurRadius: 8,
                    offset: const Offset(0, 2),
                  ),
                ],
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text(
                    'Pieces',
                    style: TextStyle(
                      fontSize: 24,
                      fontWeight: FontWeight.bold,
                      color: Color(0xFF1976D2),
                    ),
                  ),
                  const SizedBox(height: 8),
                  Text(
                    'Drag to grid',
                    style: TextStyle(fontSize: 14, color: Colors.grey[600]),
                  ),
                  const SizedBox(height: 24),
                  Expanded(
                    child: ListView.builder(
                      itemCount: _availablePieces.length,
                      itemBuilder: (context, index) {
                        final piece = _availablePieces[index];
                        return Padding(
                          padding: const EdgeInsets.only(bottom: 16),
                          child: Draggable<BlockPiece>(
                            data: piece,
                            feedback: Material(
                              color: Colors.transparent,
                              child: Transform.scale(
                                scale: 1.2,
                                child: _buildPiecePreview(piece),
                              ),
                            ),
                            childWhenDragging: Opacity(
                              opacity: 0.3,
                              child: _buildPiecePreview(piece),
                            ),
                            onDragStarted: () {
                              setState(() {
                                _draggingPiece = piece;
                              });
                            },
                            onDragEnd: (details) {
                              setState(() {
                                _draggingPiece = null;
                                _hoverRow = null;
                                _hoverCol = null;
                              });
                            },
                            child: _buildPiecePreview(piece),
                          ),
                        );
                      },
                    ),
                  ),
                  const Divider(),
                  Text(
                    'Progress: $_piecesPlaced/$_targetPieces',
                    style: const TextStyle(
                      fontSize: 16,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 8),
                  Text(
                    'Score: $_score',
                    style: TextStyle(fontSize: 14, color: Colors.grey[700]),
                  ),
                ],
              ),
            ),
          ),

          const SizedBox(width: 16),

          // Right side - Game grid
          Expanded(
            flex: 2,
            child: Container(
              padding: const EdgeInsets.all(24),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(12),
                boxShadow: [
                  BoxShadow(
                    color: Colors.black.withOpacity(0.1),
                    blurRadius: 8,
                    offset: const Offset(0, 2),
                  ),
                ],
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  const Text(
                    'Fit the blocks into the grid!',
                    style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(height: 8),
                  Text(
                    'Drag pieces from the left and drop them onto the grid',
                    style: TextStyle(fontSize: 14, color: Colors.grey[600]),
                  ),
                  const SizedBox(height: 24),
                  Expanded(
                    child: AspectRatio(
                      aspectRatio: 1,
                      child: GridView.builder(
                        physics: const NeverScrollableScrollPhysics(),
                        gridDelegate:
                            const SliverGridDelegateWithFixedCrossAxisCount(
                              crossAxisCount: gridSize,
                              childAspectRatio: 1,
                              crossAxisSpacing: 2,
                              mainAxisSpacing: 2,
                            ),
                        itemCount: gridSize * gridSize,
                        itemBuilder: (context, index) {
                          final row = index ~/ gridSize;
                          final col = index % gridSize;
                          return _buildGridCell(row, col);
                        },
                      ),
                    ),
                  ),
                  const SizedBox(height: 24),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      ElevatedButton(
                        onPressed: _giveUp,
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.red,
                          foregroundColor: Colors.white,
                          padding: const EdgeInsets.symmetric(
                            horizontal: 24,
                            vertical: 16,
                          ),
                        ),
                        child: const Text('Give Up (-200 UX)'),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildPiecePreview(BlockPiece piece) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      mainAxisAlignment: MainAxisAlignment.center,
      children: List.generate(piece.shape.length, (r) {
        return Row(
          mainAxisSize: MainAxisSize.min,
          mainAxisAlignment: MainAxisAlignment.center,
          children: List.generate(piece.shape[r].length, (c) {
            return Container(
              width: 24,
              height: 24,
              margin: const EdgeInsets.all(1),
              decoration: BoxDecoration(
                color: piece.shape[r][c] ? piece.color : Colors.transparent,
                borderRadius: BorderRadius.circular(3),
                border: piece.shape[r][c]
                    ? Border.all(color: Colors.white.withOpacity(0.3), width: 1)
                    : null,
              ),
            );
          }),
        );
      }),
    );
  }

  Widget _buildGridCell(int row, int col) {
    final isFilled = _grid[row][col];

    return DragTarget<BlockPiece>(
      onWillAcceptWithDetails: (details) {
        final piece = details.data;
        if (isFilled) return false;
        return _canPlacePiece(piece, row, col);
      },
      onAcceptWithDetails: (details) {
        _placePiece(details.data, row, col);
      },
      onMove: (details) {
        setState(() {
          _hoverRow = row;
          _hoverCol = col;
        });
      },
      onLeave: (data) {
      },
      builder: (context, candidateData, rejectedData) {
        // Determine cell state
        Color cellColor;
        Color borderColor = Colors.grey[400]!;
        double borderWidth = 1;

        if (isFilled) {
          cellColor = Colors.grey[700]!;
        } else if (_draggingPiece != null &&
            _hoverRow != null &&
            _hoverCol != null) {
          final piece = _draggingPiece!;
          final canPlace = _canPlacePiece(piece, _hoverRow!, _hoverCol!);

          final isPreviewCell = _isPartOfPiece(
            piece,
            row,
            col,
            _hoverRow!,
            _hoverCol!,
          );

          if (isPreviewCell && canPlace) {
            cellColor = piece.color.withOpacity(0.7);
            borderColor = piece.color;
            borderWidth = 2;
          } else if (canPlace && row == _hoverRow && col == _hoverCol) {
            cellColor = Colors.green[100]!;
          } else {
            cellColor = Colors.grey[200]!;
          }
        } else {
          cellColor = Colors.grey[200]!;
        }

        return Container(
          decoration: BoxDecoration(
            color: cellColor,
            border: Border.all(color: borderColor, width: borderWidth),
          ),
        );
      },
    );
  }

  bool _isPartOfPiece(
    BlockPiece piece,
    int cellRow,
    int cellCol,
    int startRow,
    int startCol,
  ) {
    for (int r = 0; r < piece.shape.length; r++) {
      for (int c = 0; c < piece.shape[r].length; c++) {
        if (piece.shape[r][c]) {
          if (startRow + r == cellRow && startCol + c == cellCol) {
            return true;
          }
        }
      }
    }
    return false;
  }

  Widget _buildResultView() {
    final isSuccess = _score > 0;

    return Center(
      child: Container(
        padding: const EdgeInsets.all(32),
        margin: const EdgeInsets.all(32),
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(16),
          boxShadow: [
            BoxShadow(
              color: Colors.black.withOpacity(0.2),
              blurRadius: 10,
              offset: const Offset(0, 4),
            ),
          ],
        ),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Icon(
              isSuccess ? Icons.check_circle : Icons.error,
              size: 64,
              color: isSuccess ? Colors.green : Colors.red,
            ),
            const SizedBox(height: 24),
            Text(
              isSuccess ? 'Puzzle Complete!' : 'Gave Up',
              style: const TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 16),
            if (isSuccess)
              Text(
                'Great job fitting all the blocks!',
                style: TextStyle(fontSize: 16, color: Colors.grey[600]),
              )
            else
              Text(
                'Better luck next time!',
                style: TextStyle(fontSize: 16, color: Colors.grey[600]),
              ),
            const SizedBox(height: 16),
            Text(
              isSuccess ? 'Your Score:' : 'Penalty:',
              style: TextStyle(fontSize: 16, color: Colors.grey[600]),
            ),
            const SizedBox(height: 8),
            Text(
              '${_score > 0 ? '+' : ''}$_score',
              style: TextStyle(
                fontSize: 48,
                fontWeight: FontWeight.bold,
                color: isSuccess ? Colors.green : Colors.red,
              ),
            ),
            const SizedBox(height: 8),
            Text(
              'Multiplier: ${(_score > 0 ? (_score / 100.0) : -2.0).toStringAsFixed(2)}x',
              style: TextStyle(fontSize: 14, color: Colors.grey[600]),
            ),
            const SizedBox(height: 32),
            if (_isSubmitting)
              const CircularProgressIndicator()
            else
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  ElevatedButton(
                    onPressed: () {
                      setState(() {
                        _initializeGame();
                      });
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.grey[300],
                      foregroundColor: Colors.black,
                    ),
                    child: const Text('Play Again'),
                  ),
                  const SizedBox(width: 16),
                  ElevatedButton(
                    onPressed: _completeAndExit,
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.blue,
                      foregroundColor: Colors.white,
                    ),
                    child: const Text('Continue'),
                  ),
                ],
              ),
          ],
        ),
      ),
    );
  }
}

class BlockPiece {
  final List<List<bool>> shape;
  final Color color;
  final int id;

  BlockPiece({required this.shape, required this.color, required this.id});
}
