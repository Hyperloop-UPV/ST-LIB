FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG CUBECLT_VERSION=1.16.0
ARG CUBECLT_INSTALLER=cubeclt_${CUBECLT_VERSION}_installer.sh

RUN echo "deb http://security.ubuntu.com/ubuntu focal-security main universe" > /etc/apt/sources.list.d/ubuntu-focal-sources.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        git git-lfs curl \
        python3 python3-pip python3-venv \
        cmake ninja-build \
        g++ build-essential \
        libncurses5 libusb-1.0-0 \
        gdb && \
    git lfs install && \
    rm -rf /var/lib/apt/lists/*

# Install cubeclt (STM32CubeCLT)
WORKDIR /tmp

RUN git clone --depth 1 https://github.com/HyperloopUPV-H8/cubeclt.git && \
    cd cubeclt && \
    git lfs pull && \
    chmod +x ${CUBECLT_INSTALLER} && \
    echo | LICENSE_ALREADY_ACCEPTED=1 ./${CUBECLT_INSTALLER} && \
    cd / && \
    rm -rf /tmp/cubeclt

ENV PATH="$PATH:/opt/st/stm32cubeclt_${CUBECLT_VERSION}/GNU-tools-for-STM32/bin:/opt/st/stm32cubeclt_${CUBECLT_VERSION}/CMake/bin:/opt/st/stm32cubeclt_${CUBECLT_VERSION}/Ninja/bin"

WORKDIR /workspace