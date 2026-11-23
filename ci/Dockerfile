FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
ARG CUBECLT_VERSION=1.21.0
ARG CUBECLT_INSTALLER=cubeclt_${CUBECLT_VERSION}_installer.sh

# 1. Install System Dependencies (Revised for Programmer CLI)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git git-lfs \
    python3 python3-pip python3-venv curl \
    libncurses5 libusb-1.0-0 stlink-tools openocd \
    libglib2.0-0 libusb-0.1-4 libsm6 libice6 \
    tio cu lsb-release \
    && rm -rf /var/lib/apt/lists/*

RUN curl -LsSf https://astral.sh/uv/install.sh | sh && \
    ln -sf /root/.local/bin/uv /usr/local/bin/uv

WORKDIR /tmp

RUN git clone --depth 1 https://github.com/Hyperloop-UPV/cubeclt.git && \
    cd cubeclt && \
    git lfs pull --include="${CUBECLT_INSTALLER}" && \
    chmod +x ${CUBECLT_INSTALLER} && \
    ./${CUBECLT_INSTALLER} --target /tmp/extract --noexec && \
    # --- Part A: STM32CubeCLT ---
    mkdir -p /opt/st/STM32CubeCLT_${CUBECLT_VERSION} && \
    tar -xzf /tmp/extract/st-stm32cubeclt_*.tar.gz -C /opt/st/STM32CubeCLT_${CUBECLT_VERSION} --strip-components=1 && \
    cp /tmp/extract/version.txt /opt/st/STM32CubeCLT_${CUBECLT_VERSION}/version.txt && \
    # --- Part B: ST-LINK Server ---
    export LICENSE_ALREADY_ACCEPTED=1 && \
    SERVER_INSTALL=$(ls /tmp/extract/st-stlink-server.*.install.sh) && \
    sh "$SERVER_INSTALL" --target /tmp/server_extract --noexec && \
    cp /tmp/server_extract/stlink-server /usr/bin/stlink-server && \
    chmod +x /usr/bin/stlink-server && \
    # --- Part C: FIX THE PATH MISMATCH ---
    # Create the directory structure the VS Code extension expects
    mkdir -p /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer && \
    ln -s /opt/st/STM32CubeCLT_${CUBECLT_VERSION}/STM32CubeProgrammer/bin /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin && \
    # Cleanup
    cd / && rm -rf /tmp/cubeclt /tmp/extract /tmp/server_extract

ENV STM32_CLT_ROOT=/opt/st/STM32CubeCLT_${CUBECLT_VERSION}
ENV HYPER_STM32CLT_ROOT=/opt/st/STM32CubeCLT_${CUBECLT_VERSION}
ENV PATH="/root/.local/bin:${STM32_CLT_ROOT}/GNU-tools-for-STM32/bin:${STM32_CLT_ROOT}/STM32CubeProgrammer/bin:${STM32_CLT_ROOT}/STLink-gdb-server/bin:${STM32_CLT_ROOT}/CMake/bin:${STM32_CLT_ROOT}/Ninja/bin:${PATH}"

WORKDIR /workspaces
