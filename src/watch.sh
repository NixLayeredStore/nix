#!/usr/bin/env bash
set -euxo pipefail

find src "(" -name "*.hh" -or -name "*.cc" ")" | entr bash -c '
    set -euo pipefail
    clear
    date
    echo
    set -x
    time make -j $NIX_BUILD_CORES
    make install
    set +x; echo -e "\n\e[32mbuild successful\e[0m\n\n"; set -x

    mkdir -p /tmp/gc-test
    sudo umount /tmp/gc-test/merged/nix/store || true
    sudo umount /tmp/gc-test/lower || true
    sudo umount /tmp/gc-test || true
    sudo mount -t tmpfs tmpfs /tmp/gc-test/
    tar -xzf /tmp/gc-test.persist.tar.gz -C /tmp/gc-test || true
    mkdir -p /tmp/gc-test/{lower,upper,merged,work}/nix/store
    nix-build() { outputs/out/bin/nix-build -E "let pkgs = import <nixpkgs> { }; in pkgs.$1" --store "$2"; }
    nix-build openssl "/tmp/gc-test/lower"
    sudo mount --bind -o ro /tmp/gc-test/lower /tmp/gc-test/lower
    sudo mount -t overlay overlay \
        -o lowerdir=/tmp/gc-test/lower/nix/store \
        -o upperdir=/tmp/gc-test/upper/nix/store \
        -o workdir=/tmp/gc-test/work \
        /tmp/gc-test/merged/nix/store
    lowerUri=$(echo "/tmp/gc-test/lower?read-only=true" | jq -Rr @uri)
    upperLayer=/tmp/gc-test/upper/nix/store
    storeUri="local-overlay?root=/tmp/gc-test/merged&lower-store=$lowerUri&upper-layer=$upperLayer&check-mount=true"
    nix-build openssl "$storeUri" --no-out-link
    #nix-build curl "$storeUri" --no-out-link
    nix-build cowsay "$storeUri" --no-out-link
    rm -f /tmp/gc-test.persist.tar.gz
    tar -czf /tmp/gc-test.persist.tar.gz -C /tmp/gc-test {lower,upper} merged/nix/var

    set +x; echo -e "\n\e[36mtest finished\e[0m"
'
