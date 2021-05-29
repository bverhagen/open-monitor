FROM bverhagen/exec-helper:debian-testing

RUN apt-get update && \
    apt-get install --yes \
        # Build tools
        build-essential \
        meson \
        ninja-build \
        clang \
        pkg-config \
        git \
        # Libraries
        libgstreamer1.0-dev \
        libgstreamer-plugins-base1.0-dev \
        libgstreamer-plugins-bad1.0-dev \
        libsoup2.4-dev \
        libjson-glib-dev \
        libglibmm-2.4-dev \
        libboost-dev \
        # Development tools
        gdb \
        clangd \
        clang-tidy \
        gstreamer1.0-tools \
        gstreamer1.0-plugins-rtp \
        gstreamer1.0-plugins-bad \
        gstreamer1.0-plugins-ugly \
        gstreamer1.0-nice \
    && apt-get clean --yes && rm -rf /var/lib/apt/lists/*
