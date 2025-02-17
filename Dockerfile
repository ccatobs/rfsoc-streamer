FROM ubuntu:22.04

# Installing needed packages
RUN apt-get update && apt-get install -y gcc \
    g++ make git python3-dev python3-pip \
    python3.10-venv libboost-all-dev \
    libopenblas-openmp-dev libflac-dev \
    bzip2 libbz2-dev libgsl-dev

# Workaround for build issue with so3g needing specifically the 'python' executable
RUN ln -s /usr/bin/python3.10 /usr/bin/python

# Make sure files belong to ocs user and group
RUN groupadd -g 9000 ocs && \
    useradd -m -l -u 9000 -g 9000 ocs

# Setting up virtual environment
RUN python3 -m venv /opt/venv/
ENV PATH="/opt/venv/bin:$PATH"
RUN python3 -m pip install -U pip
# Installing cmake here instead of with apt-get
# to avoid version issues later with so3g's 
# requirements.txt file installing cmake with pip
RUN python3 -m pip install cmake

# Guaranteeing that there will be a common build directory for socs/spt3g
WORKDIR /usr/local/so3g
WORKDIR /usr/local/src

ENV SO3G_DIR=/usr/local/src/so3g
ENV SPT3G_DIR=/usr/local/src/spt3g_software
ENV SPT3G_SOFTWARE_PATH=/usr/local/src/spt3g_software
ENV RFSOC_DIR=/usr/local/src/rfsoc-streamer
ENV LD_LIBRARY_PATH=/usr/local/so3g/lib:/usr/local/so3g/so3g
ENV STREAM_CONFIG_DIR=/config
ENV SPT3G_SOFTWARE_BUILD_PATH=${SPT3G_DIR}/build
ENV SO3G_BUILD_PATH=/usr/local/so3g/lib

# Clone all repos
RUN git clone https://github.com/CMB-S4/spt3g_software.git
RUN git clone https://github.com/simonsobs/so3g.git
#RUN git clone https://github.com/ccatobs/rfsoc-streamer.git
COPY . /usr/local/src/rfsoc-streamer

# Install spt3g
WORKDIR /usr/local/src/spt3g_software/build

RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="gcc" \
    -DCMAKE_CXX_COMPILER="g++" \
    -DCMAKE_C_FLAGS="-O3 -g -fPIC" \
    -DCMAKE_CXX_FLAGS="-O3 -g -fPIC -std=c++11" \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
    -DPython_EXECUTABLE:FILEPATH=$(which python3) \
    -DCMAKE_INSTALL_PREFIX="/usr/local/so3g" \
    ..

RUN make -j ${nproc}
RUN make install

# Install so3g
WORKDIR /usr/local/src/so3g
RUN pip install -r requirements.txt
WORKDIR /usr/local/src/so3g/build

RUN cmake \
    -DCMAKE_PREFIX_PATH='/usr/local/src/spt3g_software/build' \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
    -DPYTHON_INSTALL_DEST="/usr/local/so3g" \
    -DCMAKE_INSTALL_PREFIX="/usr/local/so3g" \
    ..

RUN make -j ${nproc}
RUN make install

# Install rfsoc-streamer
WORKDIR /usr/local/src/rfsoc-streamer
RUN cmake .
RUN make

# There is no PYTHONPATH to start, so no need to append it to the end here
ENV PYTHONPATH="${RFSOC_DIR}/lib:${RFSOC_DIR}/python:$SPT3G_SOFTWARE_BUILD_PATH:$SO3G_BUILD_PATH"
ENV PATH="/usr/local/so3g/bin:${PATH}"

RUN pip install dumb-init

# The command to run stream.py will be in the docker-compose.yml file
# for more granular user control
ENTRYPOINT ["dumb-init", /bin/bash]
