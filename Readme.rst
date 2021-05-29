Open monitor
============
A completely open-source, privacy-aware, low-latency IP camera for DIY enthusiasts.

Ideal as:

- Baby monitor
- Local security camera

Open monitor is an extremely lightweight, webRTC-based monitoring solution. On top of that, when no one is watching the feed, Open Monitor is idling and is not acquiring nor processing any frames. For example, the load on an active feed is less than 5% on a single core on a Raspberry pi 2B. All credits for this go to `GStreamer <https://gstreamer.freedesktop.org/>`_ and its awesome open source community!

While the project is setup to be generic and easily portable, our current focus is on fully supporting all Raspberry pi's connected to raspberry pi camera's.

Configuration
-------------
Open monitor is configurable through a configuration file in lua format. By default it checks for a configuration file in `/etc/open-monitor.conf`, but this can be overriden by using the `--config` command-line option.

The following options can be configured:

- *port*: The port on which to offer the HTML video page and the associated webRTC stream.
- *pipelines*: The GStreamer pipeline to use for generating the video and/or audio webRTC stream. Currently only one pipeline can be defined. The result of the configured pipeline must be compatible with GStreamer's `webrtcbin <https://gstreamer.freedesktop.org/documentation/webrtc/index.html>`_. Make sure all pipeline elements from your configured pipeline are available in the GStreamer installation on your target. The default config shows a test src for checking whether Open Monitor works on your system. Check the `example_configs` directory for preconfigured configurations for common setups.

Installation
------------
Build Open Monitor from source
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Install required upstream packages::

    sudo apt-get update && sudo apt-get install --yes build-essential meson ninja-build

2. Configure the Open Monitor build::

    meson setup --buildtype release -D systemd=enabled build .

3. Build Open Monitor::

    ninja -C build

4. Install Open Monitor::

    sudo ninja -C build install

Enable Open Monitor
~~~~~~~~~~~~~~~~~~~

1. Start Open Monitor::

    sudo systemctl start open_monitor.service

2. Browse to `http://<your host>:57778`. After a few seconds, a video stream with your camera images should appear on the seamingly empty web page. For running complex pipelines or on slower targets, you may need to wait longer.

3. If step 2 is not succesful, debug the problem using::

    sudo journalctl -eu open_monitor.service

4. If step 2 is succesful, enable Open Monitor to start at boot::

    sudo systemstl enable open_monitor.service

5. Happy monitoring!


Raspbian buster + Raspberry Pi camera installation
--------------------------------------------------
Open monitor uses the :code:`rpicamsrc` GStreamer plugin for acquiring data from the Raspberry Pi camera. However, most distributions - including Raspbian Buster - ship GStreamer without this plugin. If your distribution does ship with this plugin, we recommend to use the upstream version from your distribution and go directly to the open monitor build step. You can check whether your upstream distribution ships with this plugin by installing the gstreamer 1.x package and running::

    gst-inspect-1.0 rpicamsrc

Build Gstreamer (with rpicamsrc plugin) from source
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1. Install the required upstream packages::

    sudo apt-get update && sudo apt-get install --yes libssl-dev nasm libgnutls28-dev libsrtp2-dev meson ninja-build build-essential

2. Clone the `gst-build` repository::

    git clone https://gitlab.freedesktop.org/gstreamer/gst-build.git

3. Step into the `gst-build` directory::

    cd gst-build

4. Checkout the version of GStreamer you want to build. It is recommended to use the latest stable version, e.g. `1.18.4`::

    git checkout 1.18.4

5. Use meson to configure what GStreamer features you want to build. For a minimal GStreamer build, use::

   meson -Dauto_features=disabled -Dugly=disabled -Dbad=enabled -Dgood=enabled -Dbase=enabled -Dgstreamer:tools=enabled -Dgst-plugins-good:rpicamsrc=enabled -Dgst-plugins-good:udp=enabled -Dgst-plugins-good:rtp=enabled -Dgst-plugins-good:rtpmanager=enabled -Dgst-plugins-bad:videoparsers=enabled -Dgst-plugins-bad:webrtc=enabled  -Dgst-plugins-base:typefind=enabled -Dlibnice=enabled -Dlibnice:gstreamer=enabled -Dintrospection=disabled -Dorc=enabled -Dgst-plugins-bad:dtls=enabled -Dgst-plugins-bad:srtp=enabled build

6. Build GStreamer::

    ninja -C build

7. Install GStreamer::

    sudo ninja -C build install

Build Open Monitor from source
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Follow the guide in the  `installation` chapter.

Enable Open Monitor
~~~~~~~~~~~~~~~~~~~
Follow the guide in the `installation` chapter.
