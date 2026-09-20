# The reMarkable SDK is a Yocto toolchain that runs on x86_64 Linux. Docker
# keeps the host clean and lets Apple Silicon use the SDK under emulation.
FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        git \
        file \
        python3 \
        rsync \
        bzip2 \
        xz-utils \
        ca-certificates \
        libc6-dev \
    && rm -rf /var/lib/apt/lists/*

ARG SDK_INSTALLER=rmpp-sdk.sh
COPY ${SDK_INSTALLER} /tmp/sdk.sh

RUN chmod +x /tmp/sdk.sh \
    && /tmp/sdk.sh -y -d /opt/rmpp-sdk \
    && rm -f /tmp/sdk.sh

WORKDIR /src
