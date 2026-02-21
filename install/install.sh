#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
CONFIG_DIR="$HOME/.config/vader5"

info() { echo -e "\033[1;34m==>\033[0m $1"; }
success() { echo -e "\033[1;32m==>\033[0m $1"; }
error() { echo -e "\033[1;31m==>\033[0m $1" >&2; }

build() {
    info "Building..."
    cmake -B "$BUILD_DIR" -S "$PROJECT_DIR" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$BUILD_DIR" -j"$(nproc)"
    success "Build complete"
}

trigger_udev() {
    for dev in $(udevadm trigger --dry-run --verbose --subsystem-match=usb --attr-match=idVendor=37d7 --attr-match=idProduct=2401); do
        sudo udevadm trigger -c add --subsystem-match=hidraw --parent-match="$dev";
    done
}

install_udev() {
    info "Installing udev rules (requires sudo)..."
    sudo cp -t /etc/udev/rules.d/ "$SCRIPT_DIR/99-vader5.rules"
    sudo udevadm control --reload-rules
    trigger_udev
    success "udev rules installed"
}

install_config() {
    if [[ ! -f "$CONFIG_DIR/config.toml" ]]; then
        info "Creating config directory..."
        mkdir -p "$CONFIG_DIR"
        cp "$PROJECT_DIR/config/config.toml" "$CONFIG_DIR/config.toml"
        success "Config created at $CONFIG_DIR/config.toml"
    else
        info "Config already exists at $CONFIG_DIR/config.toml"
    fi
}

install_bin() {
    info "Installing binaries to /usr/local/bin (requires sudo)..."
    sudo install -m 755 "$BUILD_DIR/vader5d" /usr/local/bin/
    sudo install -m 755 "$BUILD_DIR/vader5-debug" /usr/local/bin/
    success "Binaries installed"
}

install_systemd() {
    info "Installing systemd service (requires sudo)..."
    sudo cp -t /etc/systemd/system/ "$SCRIPT_DIR/vader5d@.service"
    sudo cp -t /etc/udev/rules.d/ "$SCRIPT_DIR/99-vader5-systemd.rules" 
    sudo systemctl daemon-reload
    sudo udevadm control --reload-rules
    trigger_udev
    success "systemd service installed"
}

uninstall() {
    info "Uninstalling..."
    sudo systemctl stop system-vader5d.slice 2>/dev/null || true
    sudo rm -f /etc/systemd/system/vader5d.service
    sudo rm -f /etc/systemd/system/vader5d@.service
    sudo rm -f /usr/local/bin/vader5d /usr/local/bin/vader5-debug
    sudo rm -f /etc/udev/rules.d/99-vader5.rules /etc/udev/rules.d/99-vader5-systemd.rules
    sudo udevadm control --reload-rules
    sudo systemctl daemon-reload
    success "Uninstalled"
}

usage() {
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build      Build the project"
    echo "  udev       Install udev rules only"
    echo "  config     Create user config"
    echo "  install    Full install (build + udev + config + binaries + systemd)"
    echo "  uninstall  Remove installed files"
    echo ""
    echo "Without arguments: build + udev + config"
}

case "${1:-}" in
    build)     build ;;
    udev)      install_udev ;;
    config)    install_config ;;
    install)   build && install_udev && install_config && install_bin && install_systemd ;;
    uninstall) uninstall ;;
    -h|--help) usage ;;
    "")        build && install_udev && install_config ;;
    *)         error "Unknown command: $1"; usage; exit 1 ;;
esac
