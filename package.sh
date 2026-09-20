#!/usr/bin/env bash
# Package the already-built AppLoad bundle as a standalone zip.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
OUT="$HERE/dist"
APPID="word-study"

[ -f "$HERE/appload-native/out/backend/entry" ] || {
    echo "nothing built — run ./build.sh first" >&2
    exit 1
}

rm -rf "$OUT"
mkdir -p "$OUT/$APPID"
cp -R "$HERE/appload-native/out/." "$OUT/$APPID/"

cat > "$OUT/$APPID/install.sh" <<'INSTALLER'
#!/usr/bin/env bash
set -euo pipefail

RM_HOST="${RM_HOST:-10.11.99.1}"
HERE="$(cd "$(dirname "$0")" && pwd)"
DEST="/home/root/xovi/exthome/appload/word-study"

ssh "root@${RM_HOST}" "mkdir -p ${DEST}/backend"
scp "$HERE/backend/entry" "root@${RM_HOST}:${DEST}/backend/entry.new"
ssh "root@${RM_HOST}" "chmod +x ${DEST}/backend/entry.new && mv -f ${DEST}/backend/entry.new ${DEST}/backend/entry"
for file in manifest.json resources.rcc icon.png; do
    scp "$HERE/$file" "root@${RM_HOST}:${DEST}/$file.new"
    ssh "root@${RM_HOST}" "mv -f ${DEST}/$file.new ${DEST}/$file"
done
ssh "root@${RM_HOST}" "mkdir -p ${DEST}/data"
for file in "$HERE"/data/*; do
    name="$(basename "$file")"
    scp "$file" "root@${RM_HOST}:${DEST}/data/$name.new"
    ssh "root@${RM_HOST}" "mv -f ${DEST}/data/$name.new ${DEST}/data/$name"
done

cat <<MSG

Installed The Word. Restart xochitl once so AppLoad rescans the manifest,
then open AppLoad and tap The Word.
MSG
INSTALLER
chmod +x "$OUT/$APPID/install.sh"

(cd "$OUT" && zip -qr "$APPID.zip" "$APPID")
rm -rf "$OUT/$APPID"
echo "Wrote $OUT/$APPID.zip"
