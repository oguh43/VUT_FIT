// Stefan Dubnicka, xdubnis00

import 'package:flutter/material.dart';

class MacSystem7DraggableWindow extends StatefulWidget {
  final String title;
  final Widget child;
  final double initialX;
  final double initialY;
  final double? width;
  final double? height;
  final VoidCallback? onTap;
  final VoidCallback? onClose;
  final void Function(double x, double y)? onPositionChanged;
  final bool showDecorations;
  final double decorationScale;

  const MacSystem7DraggableWindow({
    super.key,
    required this.title,
    required this.child,
    this.initialX = 0,
    this.initialY = 0,
    this.width,
    this.height,
    this.onTap,
    this.onClose,
    this.onPositionChanged,
    this.showDecorations = true,
    this.decorationScale = 1.0,
  });

  @override
  State<MacSystem7DraggableWindow> createState() =>
      _MacSystem7DraggableWindowState();
}

class _MacSystem7DraggableWindowState extends State<MacSystem7DraggableWindow> {
  late double _x;
  late double _y;

  @override
  void initState() {
    super.initState();
    _x = widget.initialX;
    _y = widget.initialY;
  }

  @override
  Widget build(BuildContext context) {
    return Positioned(
      left: _x,
      top: _y,
      child: GestureDetector(
        behavior: HitTestBehavior.opaque,
        onTapDown: (_) {
          // Bring to front immediately when any interaction starts
          widget.onTap?.call();
        },
        onPanStart: (details) {
          // Ensure window is brought to front at start of drag
          widget.onTap?.call();
        },
        onPanUpdate: (details) {
          setState(() {
            _x += details.delta.dx;
            _y += details.delta.dy;

            // Keep window within screen bounds
            final screenSize = MediaQuery.of(context).size;
            final windowWidth = widget.width ?? 300;
            final windowHeight = widget.height ?? 200;

            _x = _x.clamp(0.0, screenSize.width - windowWidth);
            _y = _y.clamp(0.0, screenSize.height - windowHeight);
          });
        },
        onPanEnd: (details) {
          // Notify parent of position change when drag ends
          widget.onPositionChanged?.call(_x, _y);
        },
        child: widget.showDecorations
            ? Container(
                width: widget.width,
                height: widget.height,
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
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    // Draggable title bar
                    Container(
                      color: const Color(0xFFE0E0E0),
                      padding: EdgeInsets.symmetric(
                        horizontal: 8 * widget.decorationScale,
                        vertical: 2 * widget.decorationScale,
                      ),
                      child: Row(
                        children: [
                          Container(
                            width:
                                (widget.width ?? 300) *
                                0.06 *
                                widget.decorationScale,
                            height:
                                (widget.width ?? 300) *
                                0.06 *
                                widget.decorationScale,
                            decoration: BoxDecoration(
                              border: Border.all(
                                color: Colors.black54,
                                width: 1,
                              ),
                              color: Colors.grey[300],
                            ),
                          ),
                          SizedBox(
                            width:
                                (widget.width ?? 300) *
                                0.02 *
                                widget.decorationScale,
                          ),
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
                                        height: 3 * widget.decorationScale,
                                      ),
                                    ),
                                  ),
                                ),
                                Padding(
                                  padding: EdgeInsets.symmetric(
                                    horizontal: 12 * widget.decorationScale,
                                  ),
                                  child: Text(
                                    widget.title,
                                    style: TextStyle(
                                      fontFamily: 'Courier',
                                      fontWeight: FontWeight.w700,
                                      fontSize:
                                          (widget.width ?? 300) *
                                          0.045 *
                                          widget.decorationScale,
                                    ),
                                    overflow: TextOverflow.ellipsis,
                                  ),
                                ),
                                Expanded(
                                  child: Column(
                                    children: List.generate(
                                      6,
                                      (i) => Divider(
                                        thickness: 2,
                                        color: Colors.black12,
                                        height: 3 * widget.decorationScale,
                                      ),
                                    ),
                                  ),
                                ),
                              ],
                            ),
                          ),
                          GestureDetector(
                            onTap: widget.onClose,
                            child: Container(
                              width:
                                  (widget.width ?? 300) *
                                  0.06 *
                                  widget.decorationScale,
                              height:
                                  (widget.width ?? 300) *
                                  0.06 *
                                  widget.decorationScale,
                              decoration: BoxDecoration(
                                border: Border.all(
                                  color: Colors.black12,
                                  width: 1,
                                ),
                                color: Colors.grey[200],
                              ),
                            ),
                          ),
                        ],
                      ),
                    ),
                    Container(height: 1, color: Colors.black),
                    // Window content
                    Flexible(
                      child: Padding(
                        padding: EdgeInsets.all(12 * widget.decorationScale),
                        child: widget.child,
                      ),
                    ),
                  ],
                ),
              )
            : SizedBox(
                width: widget.width,
                height: widget.height,
                child: widget.child,
              ),
      ),
    );
  }
}
