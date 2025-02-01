FROM ubuntu:22.04

# Installing needed packages
RUN apt-get update && apt-get install -y gcc \
    g++ make python3-dev libboost-all-dev \
    python3.10-venv libopenblas-openmp-dev \
    libflac-dev bzip2 libbz2-dev libgsl-dev

WORKDIR /usr/local/src

ENV SO3G_DIR /usr/local/src/so3g
ENV SPT3G_DIR /usr/local/src/spt3g_software

# Clone all repos
RUN git clone https://github.com/CMB-S4/spt3g_software.git
RUN git clone https://github.com/simonsobs/so3g.git
RUN git clone https://github.com/ccatobs/rfsoc-streamer.git

# Install spt3g
WORKDIR /usr/local/src/spt3g_software/build
# FIX PATHS!
RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="gcc" \
    -DCMAKE_CXX_COMPILER="g++" \
    -DCMAKE_C_FLAGS="-O3 -g -fPIC" \
    -DCMAKE_CXX_FLAGS="-O3 -g -fPIC -std=c++11" \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
    -DPython_EXECUTABLE:FILEPATH=$(which python3) \
    -DPYTHON_MODULE_DIR="${HOME}/so3g/lib/python3.10/site-packages" \
    -DCMAKE_INSTALL_PREFIX="${HOME}/so3g" \
    .. 
RUN make
RUN make install

# Install so3g
WORKDIR /usr/local/src/so3g/build
RUN pip install -r requirements.txt
# FIX PATHS!
RUN cmake \
    -DCMAKE_PREFIX_PATH=${HOME}/git/spt3g_software/build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
    -DPYTHON_INSTALL_DEST="${HOME}/so3g" \
    -DCMAKE_INSTALL_PREFIX="${HOME}/so3g" \
    ..
RUN make
RUN make install

# Install rfsoc-streamer
WORKDIR /usr/local/src/rfsoc-streamer
RUN CMAKE .
RUN MAKE
# Modify PYTHONPATH?

WORKDIR /usr/local/src/rfsoc-streamer
ENTRYPOINT ??