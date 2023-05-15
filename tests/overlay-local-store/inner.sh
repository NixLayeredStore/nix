#!/usr/bin/env bash

set -eu -o pipefail

set -x

source common.sh
tree -C $TEST_ROOT
env | grep /run/user/1000/nix-test/tests/overlay-local-store

export NIX_CONFIG='build-users-group = '

echo -e '\x1b[1;37mCreating testing directories\x1b[0m'

storeA="$TEST_ROOT/store-a"
storeBTop="$TEST_ROOT/store-b"
storeB="local-overlay?root=$TEST_ROOT/merged-store&lower-store=$storeA&upper-layer=$storeBTop"

mkdir -p "$TEST_ROOT"/{store-a,store-b,merged-store/nix/store,workdir}


echo -e '\x1b[1;37mMounting Overlay Store\x1b[0m'

echo -e '\x1b[1;37mInit lower store with some stuff\x1b[0m'
nix-store --store "$storeA" --add dummy

echo -e '\x1b[1;37mBuild something in lower store\x1b[0m'
drvPath=$(nix-instantiate --store $storeA ./hermetic.nix --arg busybox "$busybox" --arg seed 1)
path=$(nix-store --store "$storeA" --realise $drvPath)

mount -t overlay overlay \
  -o lowerdir="$storeA/nix/store" \
  -o upperdir="$storeBTop" \
  -o workdir="$TEST_ROOT/workdir" \
  "$TEST_ROOT/merged-store/nix/store" \
  || skipTest "overlayfs is not supported"

cleanupOverlay () {
  echo -e '\x1b[1;32mfinished\x1b[0m'
  env | grep '^NIX_' | sort
  highlight-hermetic-input() { sed "s^\(hermetic-input-\)\(.\)^\x1b[1;37m\1\x1b[3\2m\2\x1b[0m^g"; }
  tree -C -L 4 $TEST_ROOT | highlight-hermetic-input
  find "$TEST_ROOT" -type f -exec sha256sum {} \; | sort | highlight-hermetic-input
  grep "$TEST_ROOT/merged-store/nix/store" /proc/self/mounts
  find "$TEST_ROOT" -name db.sqlite | while read DB; do
    echo
    echo -e "\x1b[1;35m$DB\x1b[0m"
    sqlite3 "$DB" "select path from ValidPaths;"
    echo
  done
  nix-store --store "$storeA" --query --requisites "$TEST_ROOT/store/k2frsyvpb38mzypcxnvv3qfbz2vxwjbk-hermetic-input-3.drv"
  nix-store --store "$storeB" --query --requisites "$TEST_ROOT/store/k2frsyvpb38mzypcxnvv3qfbz2vxwjbk-hermetic-input-3.drv"
  nix-store --store "$storeB" --realise "$TEST_ROOT/store/k2frsyvpb38mzypcxnvv3qfbz2vxwjbk-hermetic-input-3.drv"
  umount "$TEST_ROOT/merged-store/nix/store"
  rm -r $TEST_ROOT/workdir
  exit 9
}
trap cleanupOverlay EXIT

#exit 9

toRealPath () {
  storeDir=$1; shift
  storePath=$1; shift
  echo $storeDir$(echo $storePath | sed "s^$NIX_STORE_DIR^^")
}

echo -e '\x1b[1;37mCheck status\x1b[0m'

echo -e '\x1b[1;37mChecking for path in lower layer\x1b[0m'
stat $(toRealPath "$storeA/nix/store" "$path")

echo -e '\x1b[1;37mChecking for path in upper layer (should fail)\x1b[0m'
expect 1 stat $(toRealPath "$storeBTop" "$path")

echo -e '\x1b[1;37mChecking for path in overlay store matching lower layer\x1b[0m'
diff $(toRealPath "$storeA/nix/store" "$path") $(toRealPath "$TEST_ROOT/merged-store/nix/store" "$path")

echo -e '\x1b[1;37mChecking requisites query agreement\x1b[0m'
[[ \
  $(nix-store --store $storeA --query --requisites $drvPath) \
  == \
  $(nix-store --store $storeB --query --requisites $drvPath) \
  ]]

echo -e '\x1b[1;37mChecking referrers query agreement\x1b[0m'
busyboxStore=$(nix store --store $storeA add-path $busybox)
[[ \
  $(nix-store --store $storeA --query --referrers $busyboxStore) \
  == \
  $(nix-store --store $storeB --query --referrers $busyboxStore) \
  ]]

echo -e '\x1b[1;37mChecking derivers query agreement\x1b[0m'
[[ \
  $(nix-store --store $storeA --query --deriver $path) \
  == \
  $(nix-store --store $storeB --query --deriver $path) \
  ]]

echo -e '\x1b[1;37mChecking outputs query agreement\x1b[0m'
[[ \
  $(nix-store --store $storeA --query --outputs $drvPath) \
  == \
  $(nix-store --store $storeB --query --outputs $drvPath) \
  ]]

echo -e '\x1b[1;37mVerifying path in lower layer\x1b[0m'
nix-store --verify-path --store "$storeA" "$path"

echo -e '\x1b[1;37mVerifying path in merged-store\x1b[0m'
nix-store --verify-path --store "$storeB" "$path"

hashPart=$(echo $path | sed "s^$NIX_STORE_DIR/^^" | sed 's/-.*//')

echo -e '\x1b[1;37mLower store can find from hash part\x1b[0m'
[[ $(nix store --store $storeA path-from-hash-part $hashPart) == $path ]]

echo -e '\x1b[1;37mmerged store can find from hash part\x1b[0m'
[[ $(nix store --store $storeB path-from-hash-part $hashPart) == $path ]]

echo -e '\x1b[1;37mDo a redundant add\x1b[0m'

echo -e '\x1b[1;37mupper layer should not have it\x1b[0m'
expect 1 stat $(toRealPath "$storeBTop/nix/store" "$path")

path=$(nix-store --store "$storeB" --add dummy)

echo -e '\x1b[1;37mlower store should have it from before\x1b[0m'
stat $(toRealPath "$storeA/nix/store" "$path")

echo -e '\x1b[1;37mupper layer should still not have it (no redundant copy)\x1b[0m'
expect 1 stat $(toRealPath "$storeB/nix/store" "$path")

echo -e '\x1b[1;37mDo a build in overlay store\x1b[0m'

path=$(nix-build ./hermetic.nix --arg busybox $busybox --arg seed 2 --store "$storeB" --no-out-link)

echo -e '\x1b[1;37mChecking for path in lower layer (should fail)\x1b[0m'
expect 1 stat $(toRealPath "$storeA/nix/store" "$path")

echo -e '\x1b[1;37mChecking for path in upper layer\x1b[0m'
stat $(toRealPath "$storeBTop" "$path")

echo -e '\x1b[1;37mVerifying path in overlay store\x1b[0m'
nix-store --verify-path --store "$storeB" "$path"
