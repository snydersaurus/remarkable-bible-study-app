#!/usr/bin/env bash
# Cross-compile and assemble the AppLoad bundle.
#
#   ./build.sh ~/Downloads/remarkable-production-image-...-chiappa-...-toolchain.sh
#
# The SDK image is intentionally pinned by IMAGE. Build against an SDK no
# newer than the Qt version on the tablet.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
IMAGE="${IMAGE:-rmpp-sdk-5.7}"
SDK_SH="${1:-}"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    [ -n "$SDK_SH" ] || {
        echo "no '$IMAGE' image yet — pass the SDK installer:"
        echo "  ./build.sh ~/Downloads/remarkable-...-toolchain.sh"
        exit 1
    }
    if [ "$(cd "$(dirname "$SDK_SH")" && pwd)/$(basename "$SDK_SH")" \
         != "$HERE/docker/rmpp-sdk.sh" ]; then
        cp "$SDK_SH" "$HERE/docker/rmpp-sdk.sh"
    fi
    docker build --platform linux/amd64 -t "$IMAGE" "$HERE/docker"
fi

docker run --rm --platform linux/amd64 -v "$HERE":/src "$IMAGE" bash -lc '
  set -euo pipefail
  source /opt/rmpp-sdk/environment-setup-*-remarkable-linux
  TOOLCHAIN="${OE_CMAKE_TOOLCHAIN_FILE:-${OECORE_NATIVE_SYSROOT}/usr/share/cmake/OetoolchainConfig.cmake}"
  if [ ! -f "$TOOLCHAIN" ]; then
    TOOLCHAIN="${OE_CMAKE_TOOLCHAIN_FILE:-${OECORE_NATIVE_SYSROOT}/usr/share/cmake/OEToolchainConfig.cmake}"
  fi
  cmake -S /src -B /src/build-rmpp -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
      -DQT_HOST_PATH="${OECORE_NATIVE_SYSROOT}/usr"
  cmake --build /src/build-rmpp
  "${OECORE_NATIVE_SYSROOT}/usr/bin/qmllint" /src/ui/*.qml || true

  cd /src
  rm -rf build-qml && mkdir -p build-qml/ui
  cp ui/Main.qml ui/Board.qml ui/WordToken.qml build-qml/ui/
  cp application.qrc build-qml/
  (cd build-qml && "${OECORE_NATIVE_SYSROOT}/usr/libexec/rcc" \
      --binary --no-compress -o resources.rcc application.qrc)

  rm -rf appload-native/out && mkdir -p appload-native/out/backend
  cp appload-native/manifest.json appload-native/icon.png appload-native/out/
  cp build-qml/resources.rcc appload-native/out/
  cp build-rmpp/word_study_backend appload-native/out/backend/entry
  cp -R data appload-native/out/data
  chmod +x appload-native/out/backend/entry
'

echo "Built: $HERE/appload-native/out"
echo "Install with AppLoad after restarting xochitl so it rescans the manifest."
