#!/usr/bin/env bash
set -eu

SCRIPT_PATH="$(realpath "${BASH_SOURCE[0]}")"
export OUTPUT_ROOT="$(dirname "$SCRIPT_PATH")/tech-demo-static-nix"

buildNix() {
    if [[ -L "$OUTPUT_ROOT" ]]; then
        echo -e "\e[33m"
        echo "Skipping build because output link already exists."
        echo "If you want to force a rebuild, remove this symlink:"
        echo "    $OUTPUT_ROOT"
        echo -e "\e[0m"
    else
        (
            cd $(dirname "$SCRIPT_PATH")
            nix build ".#nix-static" --out-link tech-demo-static-nix
        )
    fi
}

case "${TECH_DEMO_MODE:-start-demo}" in
    start-demo)
        buildNix
        set -x
        TECH_DEMO_MODE=setup-namespace unshare \
            --mount --map-root-user "$SCRIPT_PATH"
        exit 0
        ;;

    setup-namespace)
        set -x
        export TMPFS_ROOT=$(mktemp -d)
        mount -t tmpfs tmpfs "$TMPFS_ROOT"
        cd "$TMPFS_ROOT"
        mkdir -p lower/nix
        mount --bind -o ro /nix lower/nix
        mkdir -p upper work
        mount -t overlay overlay \
            -o lowerdir=$PWD/lower/nix/store \
            -o upperdir=$PWD/upper \
            -o workdir=$PWD/work \
            /nix/store
        tar -cf .nix-state.tar --ignore-failed-read -C /nix/var nix/profiles
        mount -t tmpfs tmpfs /nix/var
        tar -xf .nix-state.tar --no-same-owner -C /nix/var
        set +x

        export STORE_URI=$(tr -d '\n ' <<<"local-overlay?check-mount=false
            &lower-store=$(echo "$PWD/lower?read-only=true" | jq -Rr @uri)
            &upper-layer=$PWD/upper
        ")

        bash --noprofile --rcfile <(echo "
            alias ls='ls --color=auto'
            export PATH=\"$OUTPUT_ROOT/bin:\$PATH\"
            export NIX_CONFIG=\"
                store = $STORE_URI
                build-users-group =
                use-xdg-base-directories = true
                drop-supplementary-groups = false
                experimental-features = nix-command flakes read-only-local-store
            \"
            export NIX_PATH=\"nixpkgs=/nix/var/nix/profiles/per-user/root/channels/nixpkgs\"

            list-upper() {
                \$(nix-build -E '
                    let pkgs = import <nixpkgs> { };
                    in pkgs.tree
                ')/bin/tree -L 1 "$PWD/upper"
            }

            list-mounts() {
                grep '$TMPFS_ROOT' /proc/self/mounts
            }

            echo -e '
                \\e[1;37mNix Layered Store: Tech Demo Instructions\\e[0m

                You are now in the demo environment.
                Type \\e[1;37mexit\\e[0m to return to your normal shell.

                The PATH variable has been set so that all Nix commands
                you run will support the new layered store functionality.
                You can confirm this by running \\e[1;37mwhich nix\\e[0m.
                
            \\e[0m'
        ")
        exit 0
        ;;
esac
