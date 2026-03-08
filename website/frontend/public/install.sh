#!/bin/bash
set -e

echo "====================================="
echo "  Quanta Programming Language Updater"
echo "====================================="

if [ "$EUID" -ne 0 ]; then
  echo "Please run this installer as root (e.g. using sudo)."
  exit 1
fi

tmp_dir=$(mktemp -d)
echo "[*] Cloning latest Quanta source code..."
git clone https://github.com/rohankumarrawat/Quanta.git "$tmp_dir/Quanta" -q

cd "$tmp_dir/Quanta"
echo "[*] Building Quanta..."
mkdir -p build && cd build
cmake .. > /dev/null
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2) > /dev/null

echo "[*] Installing to /usr/local/bin/quanta..."
cp quanta /usr/local/bin/quanta
chmod +x /usr/local/bin/quanta

echo "[*] Cleaning up..."
rm -rf "$tmp_dir"

echo "✅ Quanta successfully updated to the latest version!"
quanta --version
