#!/usr/bin/env bash
set -xeuo pipefail

flakes="--extra-experimental-features nix-command --extra-experimental-features flakes"

make -j $NIX_BUILD_CORES

sudo umount /tmp/nix/store || true
#trap 'sudo umount /tmp/nix/store' EXIT

sudo rm -rf /tmp/nix
sudo rm -rf /tmp/nix-upper
sudo rm -rf /tmp/nix-work
sudo rm -rf /tmp/nix-lower
mkdir -p /tmp/nix/store
mkdir -p /tmp/nix-upper
mkdir -p /tmp/nix-work
mkdir -p /tmp/nix-lower/nix/store

export NIX_CONFIG="extra-experimental-features = nix-command flakes"

# static lower dir
#
# nix build 'github:nixos/nixpkgs/af21c31b2a1ec5d361ed8050edd0303c31306397#hello'
#
# helloOutPath=$(nix eval --raw 'github:nixos/nixpkgs/af21c31b2a1ec5d361ed8050edd0303c31306397#hello.outPath')
#
# cp -r --parents "${helloOutPath}" /tmp/nix-lower/
# curl https://cache.nixos.org/2g3jazymqbjw9c390c3b7vw8xq3r8iny.narinfo -o /tmp/nix-lower/nix/store/2g3jazymqbjw9c390c3b7vw8xq3r8iny.narinfo
#
# ls -la /tmp/nix-lower/nix/store/
#
# sudo mount -t overlay overlay \
#     -o lowerdir="/tmp/nix-lower/nix/store" \
#     -o upperdir="/tmp/nix-upper" \
#     -o workdir="/tmp/nix-work" \
#     "/tmp/nix/store"

# ./outputs/out/bin/nix build -vvvv -L --builders '' --max-jobs 0 --show-trace --store 'local-overlay?root=/tmp&lower-store=local-cache:%3Froot=%2Ftmp%2Fnix-lower&upper-layer=/tmp/nix-upper' 'github:nixos/nixpkgs/af21c31b2a1ec5d361ed8050edd0303c31306397#hello'

# ersa version

sudo mount -t overlay overlay \
    -o lowerdir="/var/lib/ersa/nix/store" \
    -o upperdir="/tmp/nix-upper" \
    -o workdir="/tmp/nix-work" \
    "/tmp/nix/store"

outPath=$(sudo -E ./outputs/out/bin/nix eval --raw -vvvv -L --show-trace --store 'local-overlay?root=/tmp&lower-store=local-cache:%3Froot=%2Fvar%2Flib%2Fersa&upper-layer=/tmp/nix-upper' 'github:nixos/nixpkgs/af21c31b2a1ec5d361ed8050edd0303c31306397#hello.outPath')

if [ "$outPath" = "/nix/store/2g3jazymqbjw9c390c3b7vw8xq3r8iny-hello-2.12.1" ]; then
    echo "succeess"
else
    echo "failure"
fi
