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

    lowerUri=$(echo "/tmp/test/lower" | jq -Rr @uri)
    outputs/out/bin/nix-build -E "let pkgs = import <nixpkgs> { }; in pkgs.openssl" \
        --store "local-overlay?root=/tmp/test/merged&lower-store=$lowerUri&upper-layer=/tmp/test/upper&check-mount=false" --debug

    set +x; echo -e "\n\e[36mtest finished\e[0m"
'
