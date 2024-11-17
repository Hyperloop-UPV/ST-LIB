FROM --platform=linux/amd64 mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

# Install cubeclt
RUN curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash && apt-get install git-lfs && \
    mkdir /temp && cd /temp && git clone https://github.com/HyperloopUPV-H8/cubeclt.git && \
    cd cubeclt && git lfs pull && chmod +x cubeclt_1.16.0_installer.sh && \
    echo | sudo LICENSE_ALREADY_ACCEPTED=1 ./cubeclt_1.16.0_installer.sh

USER vscode