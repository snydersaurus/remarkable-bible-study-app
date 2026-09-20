# reMarkable Paper Pro Move platform notes

The Word is a native QML AppLoad app for the Paper Pro Move. These constraints
are shared by the neighboring baseball and football apps.

## Device and SDK

- Paper Pro Move: `chiappa`, 954×1696, aarch64.
- The current device line is 5.7.x with Qt 6.8.2.
- Build with the matching or older SDK. A newer SDK can produce a binary that
  silently fails to launch because its Qt ABI is too new.
- The SDK runs inside Docker on Apple Silicon under x86 emulation. The first
  build can take several minutes; later builds use the cached image.

The build and deploy scripts check that the backend reaches its own AppLoad
socket failure on the tablet. A loader error at that point means the SDK does
not match the device.

## AppLoad bundle

The installed bundle lives under:

```text
/home/root/xovi/exthome/appload/word-study/
  manifest.json
  icon.png
  resources.rcc
  backend/entry
  data/
```

The QML frontend runs inside xochitl. The backend is a separate headless Qt
process, and the two exchange one compact JSON state message plus small input
messages over the AppLoad socket.

AppLoad reads the manifest, icon, and frontend bundle when xochitl starts.
Frontend changes therefore require:

```bash
ssh root@10.11.99.1 'systemctl restart xochitl'
```

The deploy script copies binaries to temporary names and moves them into place
so a backend that AppLoad has left running is not overwritten in place.

## E-ink layout

- Use black and white with one restrained accent color.
- Avoid animation, gradients, and dense fills.
- Keep touch targets large and text readable.
- Derive the canvas from the panel aspect ratio rather than hardcoding only
  the full-size Paper Pro dimensions.
- Keep the frontend pure presentation; offline data and persistence belong in
  the backend.

## Release workflow

For development:

```bash
IMAGE=rmpp-sdk-5.7 ./build.sh
RM_HOST=10.11.99.1 ./deploy.sh
```

For distribution:

```bash
./package.sh
```

This writes `dist/word-study.zip` with the AppLoad bundle, offline data, and a
standalone installer. The tablet must be awake and in developer mode with SSH,
xovi, and AppLoad available.
