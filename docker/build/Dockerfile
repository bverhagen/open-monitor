FROM bverhagen/exec-helper:debian-testing

RUN apt-get update && \
    apt-get install --yes \
        # Build tools
        build-essential \
        meson \
        ninja-build \
        pkg-config \
        # Cross-build tools
        multistrap \
        g++-arm-linux-gnueabihf \
        # Libraries
        libgstreamer1.0-dev \
        libgstreamer-plugins-base1.0-dev \
        libgstreamer-plugins-bad1.0-dev \
        libsoup2.4-dev \
        libjson-glib-dev \
    && apt-get clean --yes && rm -rf /var/lib/apt/lists/*

# Prepare Raspbian sysroot
COPY /arm-linux-gnueabihf-pkg-config /usr/bin/
ARG SYSROOT=/opt/sysroot/raspbian
RUN mkdir --parents ${SYSROOT}/etc/apt
RUN apt-key --keyring ${SYSROOT}/etc/apt/trusted.gpg adv --keyserver keyserver.ubuntu.com --recv-keys 9165938D90FDDD2E

# Populate Raspbian sysroot
COPY multistrap_raspbian /opt/sysroot/multistrap_raspbian
RUN multistrap --arch armhf --dir ${SYSROOT} --file /opt/sysroot/multistrap_raspbian
RUN rm ${SYSROOT}/var/cache/apt/archives/libc6-dev_*.deb
RUN for DEB in $(find ${SYSROOT}/var/cache/apt/archives -not -path ${SYSROOT}/var/cache/apt/archives/partial -type f); do echo "Extracting ${DEB}..."; dpkg -x ${DEB} ${SYSROOT}; done   # Unpack all downloaded debian packages
