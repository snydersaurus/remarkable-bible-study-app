#!/usr/bin/env bash
# Deploy the built AppLoad bundle to a developer-mode Paper Pro Move.
set -euo pipefail

RM_HOST="${RM_HOST:-10.11.99.1}"
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT="$HERE/appload-native/out"
DEST="/home/root/xovi/exthome/appload/word-study"

[ -f "$OUT/backend/entry" ] || {
    echo "nothing built — run ./build.sh first" >&2
    exit 1
}

echo "==> checking AppLoad on $RM_HOST"
ssh "root@$RM_HOST" '[ -d /home/root/xovi/exthome/appload ]'

echo "==> copying backend"
ssh "root@$RM_HOST" "mkdir -p $DEST/backend"
scp "$OUT/backend/entry" "root@$RM_HOST:$DEST/backend/entry.new"
ssh "root@$RM_HOST" "chmod +x $DEST/backend/entry.new && mv -f $DEST/backend/entry.new $DEST/backend/entry"

echo "==> checking the device can load the backend"
LOADED="$(ssh "root@$RM_HOST" "$DEST/backend/entry /tmp/appload-probe-word-study.sock 2>&1" || true)"
case "$LOADED" in
    *"not found"*|*"cannot open shared object"*)
        echo "$LOADED" >&2
        echo "The backend was built against an incompatible SDK." >&2
        exit 1
        ;;
esac

echo "==> copying offline Bible data"
ssh "root@$RM_HOST" "mkdir -p $DEST/data"
for file in "$OUT"/data/*; do
    name="$(basename "$file")"
    scp "$file" "root@$RM_HOST:$DEST/data/$name.new"
    ssh "root@$RM_HOST" "mv -f $DEST/data/$name.new $DEST/data/$name"
done

echo "==> copying frontend bundle"
for file in manifest.json resources.rcc icon.png; do
    scp "$OUT/$file" "root@$RM_HOST:$DEST/$file.new"
    ssh "root@$RM_HOST" "mv -f $DEST/$file.new $DEST/$file"
done

echo "==> restarting xochitl so AppLoad rescans the app"
ssh "root@$RM_HOST" 'systemctl restart xochitl'

cat <<MSG

Installed The Word at $DEST on $RM_HOST.
After xochitl returns, open the AppLoad sidebar and tap The Word.
MSG
