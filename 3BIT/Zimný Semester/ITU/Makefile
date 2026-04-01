# config
PLATFORMS ?= linux,macos,windows
DEVICE    ?=

SHELL := /bin/sh

.PHONY: doctor bootstrap get run run-desktop format clean gen

doctor:
	@command -v flutter >/dev/null 2>&1 || { echo "Flutter not found in PATH"; exit 1; }
	@flutter doctor -v

bootstrap: doctor
	@flutter create --platforms=$(PLATFORMS) .
	@echo ">> (Best-effort) povolenie desktop platforiem"
	-@flutter config --enable-windows-desktop >/dev/null 2>&1 || true
	-@flutter config --enable-macos-desktop   >/dev/null 2>&1 || true
	-@flutter config --enable-linux-desktop   >/dev/null 2>&1 || true
	@$(MAKE) get

get:
	@echo ">> flutter pub get"
	@flutter pub get

run-device:
	@echo ">> flutter run $(if $(DEVICE),-d $(DEVICE),)"
	@flutter run $(if $(DEVICE),-d $(DEVICE),)

# Tries to select target from OS
run:
	@if [ "$$(uname -s)" = "Darwin" ] || [ "$$(uname -s)" = "Linux" ]; then \
	  nohup dart run ./lib/services/server.dart > server.log 2>&1 & \
	  sleep 0.2; \
	elif command -v cmd.exe >/dev/null 2>&1; then \
	  cmd.exe /C start /B dart run ./lib/services/server.dart; \
	else \
	  echo "Could not start server on this platform."; \
	fi
	@if [ "$$(uname -s)" = "Darwin" ]; then flutter run -d macos; \
	elif [ "$$(uname -s)" = "Linux" ]; then flutter run -d linux; \
	else flutter run -d windows; fi

format:
	@dart format lib

gen:
	@if grep -q "build_runner" pubspec.yaml; then \
	  echo ">> build_runner"; \
	  dart run build_runner build --delete-conflicting-outputs; \
	else \
	  echo "build_runner not in pubspec.yaml – skipping"; \
	fi

clean:
	@flutter clean
	@rm -rf pubspec.lock .dart_tool/ build/
