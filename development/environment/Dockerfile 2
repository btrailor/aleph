# Aleph Development Environment
# Based on Ubuntu 20.04 for better compatibility with old toolchains
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
# Note: gcc-multilib and g++-multilib are not available on ARM64
RUN apt-get update && apt-get install -y \
    curl \
    flex \
    bison \
    libgmp-dev \
    libmpfr-dev \
    libmpc-dev \
    autoconf \
    build-essential \
    libncurses5-dev \
    texinfo \
    git \
    wget \
    bzip2 \
    unzip \
    gcc \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /aleph

# Set up environment
ENV PATH="/opt/avr32-tools/bin:/opt/uClinux/bfin-elf/bin:${PATH}"

# Note: Actual toolchain builds will be done when the container runs
# to avoid making the image too large

CMD ["/bin/bash"]
