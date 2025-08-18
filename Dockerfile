FROM ubuntu:24.04

# Installing needed packages
RUN apt-get update && apt-get install -y gcc \
    g++ make git python3-dev python3-pip \
    python3.12-venv \
    libopenblas-openmp-dev libflac-dev \
    bzip2 libbz2-dev libgsl-dev

# Make sure files belong to ocs user and group
RUN groupadd -g 9000 ocs && \
    useradd -m -l -u 9000 -g 9000 ocs

# Setting up virtual environment
RUN python3 -m venv /opt/venv/
ENV PATH="/opt/venv/bin:$PATH"
RUN python3 -m pip install -U pip
# Installing cmake here instead of with apt-get --
# did so previously to avoid version issues later with so3g's 
# requirements.txt file installing cmake with pip --
# not sure if there are still issues
RUN python3 -m pip install cmake

# Install subset of so3g requirements.txt
# needed for rfsoc-streamer (which subset?)
RUN python3 -m pip install numpy
RUN python3 -m pip install scipy
RUN python3 -m pip install PyYAML
RUN python3 -m pip install astropy
RUN python3 -m pip install healpy
# Install pybind11 with version required by spt3g
RUN python3 -m pip install pybind11==2.13.6

# Guaranteeing that there will be a common build directory for spt3g
WORKDIR /usr/local/src

ENV SPT3G_DIR=/usr/local/src/spt3g_software
ENV SPT3G_SOFTWARE_PATH=/usr/local/src/spt3g_software
ENV RFSOC_DIR=/usr/local/src/rfsoc-streamer
ENV STREAM_CONFIG_DIR=/config
ENV SPT3G_SOFTWARE_BUILD_PATH=${SPT3G_DIR}/build

# Clone all repos
RUN git clone https://github.com/CMB-S4/spt3g_software.git
#RUN git clone https://github.com/ccatobs/rfsoc-streamer.git

# Install spt3g
WORKDIR /usr/local/src/spt3g_software/build
RUN git checkout e2b0ed8ae72f6a3f9b9e0127c19536e2213ff63d
RUN test "$(git rev-parse HEAD)" = "e2b0ed8ae72f6a3f9b9e0127c19536e2213ff63d"

#
RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="gcc" \
    -DCMAKE_CXX_COMPILER="g++" \
    -DCMAKE_C_FLAGS="-O3 -g -fPIC" \
    -DCMAKE_CXX_FLAGS="-O3 -g -fPIC -std=c++17" \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
    -DPython_EXECUTABLE:FILEPATH=$(which python3) \
    -DCMAKE_INSTALL_PREFIX="/usr/local/spt3g" \
    ..

RUN make -j 4
RUN make install

# Install rfsoc-streamer
WORKDIR /usr/local/src
COPY . /usr/local/src/rfsoc-streamer
WORKDIR /usr/local/src/rfsoc-streamer
RUN cmake .
RUN make
COPY scripts/stream_config.yaml /config/stream_config.yaml

WORKDIR /usr/local/src/rfsoc-streamer/scripts
RUN make inspect_g3
WORKDIR /usr/local/src/rfsoc-streamer

# There is no PYTHONPATH to start, so no need to append it to the end here
ENV PYTHONPATH="${RFSOC_DIR}/lib:${RFSOC_DIR}/python:$SPT3G_SOFTWARE_BUILD_PATH"
ENV LD_LIBRARY_PATH="${SPT3G_SOFTWARE_BUILD_PATH}/lib"

RUN python3 -m pip install dumb-init

# The command to run stream.py will be in the docker-compose.yml file
# for more granular user control
ENTRYPOINT ["dumb-init" , "/bin/bash"]




