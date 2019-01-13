FROM debian:stretch

RUN apt-get update && \
    apt-get -y --force-yes install \
    automake \
    autotools-dev \
    build-essential \
    check \
    checkinstall \
    cmake \
    ffmpeg \
    git \
    libavcodec-dev \
    libavdevice-dev \
    libexif-dev \
    libgdk-pixbuf2.0-dev \
    libgtk2.0-dev \
    libopenal-dev \
    libopus-dev \
    libqrencode-dev \
    libqt5opengl5-dev \
    libqt5svg5-dev \
    libsodium-dev \
    libsqlcipher-dev \
    libtool \
    libvpx-dev \
    libxss-dev \
    pkg-config \
    qrencode \
    qt5-default \
    qttools5-dev \
    qttools5-dev-tools \
    yasm

RUN git clone https://github.com/toktok/c-toxcore.git /toxcore
WORKDIR /toxcore
RUN git checkout v0.2.9 && \
        cmake . && \
        cmake --build . && \
        make install && \
        echo '/usr/local/lib/' >> /etc/ld.so.conf.d/locallib.conf && \
        ldconfig

COPY . /qtox
WORKDIR /qtox
RUN cmake . && cmake --build .
