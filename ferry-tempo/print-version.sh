#!/usr/bin/env sh

# git tag and git rev-parse are sadly not porcelain...
# We'll have to live with the possiblity of breakage.

OUT=$(git tag -l --points-at HEAD)
if [[ -z "$OUT" ]]; then
  OUT="$OUT"$(git rev-parse HEAD)
fi

OUT="$OUT-"$(git symbolic-ref --short -q HEAD || echo "detached")
OUT="$OUT"$(git diff-index --quiet HEAD -- || echo "-dirty")

if [[ "$1" != "--raw" ]]; then
	echo "-DVERSION=\"\\\"$OUT\\\"\""
else
	echo "$OUT"
fi
