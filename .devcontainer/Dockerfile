FROM ubuntu:24.04

ENV PATH="$PATH:/opt/st/stm32cubeclt_1.16.0/GNU-tools-for-STM32/bin:/opt/st/stm32cubeclt_1.16.0/CMake/bin"

# Install git, git-lfs, pip and colorama, and create workspace directory
RUN apt-get update && apt-get -y install git git-lfs python3-colorama cmake g++ build-essential && \
    git lfs install

# Install cubeclt
RUN mkdir /temp && cd /temp && git clone https://github.com/HyperloopUPV-H8/cubeclt.git && \
    cd cubeclt && git lfs pull && chmod +x cubeclt_1.16.0_installer.sh && \
    echo | LICENSE_ALREADY_ACCEPTED=1 ./cubeclt_1.16.0_installer.sh
