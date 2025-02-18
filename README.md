# rfsoc-streamer

## Description
The rfsoc-streamer package is a group of G3Pipeline modules for receiving packets from
the RFSoCs being used by the CCAT Observatory, packaging the data
into G3Frames, and writing those frames into g3 files. The rfsoc-streamer
builds heavily upon the model of the [smurf-streamer](https://github.com/simonsobs/smurf-streamer/tree/master)
that is being used for the equivalent task for the Simons Observatory.
The g3 file format, frame concept, and G3Pipeline framework all come from
the SPT-3G collaboration and build on the public subset of their code
that is maintained by the CMB-S4 collaboration ([GitHub](https://github.com/CMB-S4/spt3g_software/) and [docs](https://cmb-s4.github.io/spt3g_software/)). 
It also makes use of tools for working with g3 files from the 
[so3g](https://github.com/simonsobs/so3g) package, also from the Simons Observatory.

The core code for the rfsoc-streamer is written in C++ and lives in the
`src/` and `include/` directories. There are two main modules: the 
RfsocTransmitter and the RfsocBuilder. The transmitter is passed a source
IP address. It receives UDP packets from that source IP address, stores 
them in an RfsocSample object that comprises a timestamp and an RfsocPacket struct, and passes this RfsocSample object to the builder via
the G3Pipeline with the AsyncDatum function in the G3EventBuilder class.
The RfsocPacket struct is designed to store the data
according to the CCAT RFSoC datagram structure defined [here](https://github.com/TheJabur/primecam_readout/blob/develop/docs/rfsoc_datagram.csv).

The builder, then, collects incoming packets into a queue, flushes that
queue every `agg_duration_` seconds (default: 3 seconds) by calling the
FrameFromSamples() function, and generates a Scan G3Frame storing all of the 
data from the RfsocSample within FrameFromSamples(). The output frame
has many keys outlined below, but the main data structure is a 
G3SuperTimestream (from so3g) that stores the timestamps, channel names,
and I and Q data timestreams for all of the active detectors in the 
packet over that `agg_duration_` length of time. This object has 
built-in compression with configurable options that can be found in the 
so3g docs. 

| Scan Frame Key | Meaning |
| -------------- | ------- |
| "time" | (G3Time) Timestamp when the frame is generated |
| "data" | (G3SuperTimestream) Data object with attributes "names", "times", and "data" containing the channel names, list of timestamps, and data points for each timestamp. The I and Q channels for each resonator are saved separately. Begins with "r0000_I", "r0000_Q", "r0001_I", etc. |
| "channel_count" | (G3Int) The number of active RFSoC tones, as reported in the packets |
| "num_samples" | (G3Int) The number of packets aggregated into this frame | 
| "utc_from_ptp" | (G3Time) UTC timestamp of the first packet in the frame |
| "double_utc_from_ptp" | (Only present when debug_ is True) (G3Double) The non-G3Time version of the PTP timestamp for debugging conversion |
| "start_packet_count" | (Only present when debug_ is True) (G3Int) The packet number of the first packet that is aggregated into this frame. Currently, the RFSoC does not reset its packet counter until re-initialized, so this number counts the number of packets since then. | 
| "last_packet_count" | (Only present when debug_ is True) (G3Int) The packet number of the last packet that is aggregated into this frame. Currently, the RFSoC does not reset its packet counter until re-initialized, so this number counts the number of packets since then. | 
| "packet_flag_1" | (G3Int) The first packet flag byte in the first packet in the stream. Currently unused! | 
| "packet_flag_2" | (G3Int) The second packet flag byte in the first packet in the stream. Currently unused! | 
| "frame_number" | The number of the frame in the file - added by SessionManager |
| "session_id" | The session ID of the stream (see below) - added by SessionManager |
| "ccatstream_id" | The stream ID of the stream (see below) - added by SessionManager |
| "ccatstream_version" | The version number of the SessionManager code - added by SessionManager |

All of this C++ code has been wrapped into Python using tools in Boost Python. Once properly built, it can 
be imported as `ccatrfsoccore`. There are also Makefiles provided in `src/` and `scripts/` for building
the core C++ code and C++ scripts for streaming data (`stream_min_example.cxx`) and printing out the 
contents of g3 files (`inspect_g3.cxx`). Instructions for building from source are provided below.

For general operation, however, the code is intended to be used alongside the pure Python library,
`ccatrfsoc`, that lives in the `python/` directory. There are two additional modules here: the 
G3Rotator in CCATFileWriter and the SessionManager. The former handles writing the output frames
to g3 files. After a fixed amount of time, one file is closed off and a new one begun to avoid 
any one file becoming too large. The latter handles how frames pass through the system, wrapping up
the current g3 file when a stream ends and beginning a new g3 file when a stream begins.
Both of these are similar to the smurf-streamer versions but with small changes to accommodate
differences between the streaming flow and packets of the SMuRF and the RFSoC.

## Standard operation
The fastest way to get started with the rfsoc-streamer and the default operation method for CCAT
is to use the Docker images that are automatically built in this repo with GitHub Actions. These
containers handle all of the installation and can be easily configured with a docker-compose.yml
file and the stream_config.yaml file (example in `scripts/`) to run many instances of the 
rfsoc-streamer. Nominally, there should be a single instance of the streamer running for each
RFSoC drone activate on the network so that data from different drones (each of which handles
a separate RF network) ends up in separate files.

An example docker-compose.yml file for these would look like:
```
services:
  rfsoc-streamer:
    image: ghcr.io/ccatobs/rfsoc-streamer:v0.1
    user: ocs:ocs
    container_name: rfsoc-streamer-b2d1
    network_mode: host
    volumes:
      - ${STREAM_CONFIG_DIR}:/config
      - <path_to_data_directory_on_host_computer>:/data
    entrypoint: python3 -u /usr/local/src/rfsoc-streamer/scripts/stream.py
    environment:
      - BOARD=2
      - DRONE=1
```
Here, the image tag after rfsoc-streamer should be updated to whichever version
you would like to use. Versions can be seen on GitHub under the [packages link](https://github.com/orgs/ccatobs/packages?repo_name=rfsoc-streamer) on the
right hand side of the screen in the repo home page. The user should replace 
<path_to_data_directory_on_host_computer> with the path where they want their data
to show up on the host computer. The user should also replace the numbers following 
BOARD and DRONE with the appropriate values to point to the correct RFSoC infomation
in the stream_config.yaml file. The container name should be updated accordingly.
The $STREAM_CONFIG_DIR variable should be set in .bashrc with:
```
export STREAM_CONFIG_DIR=<path-to-stream_config.yaml>
```

The entrypoint command shows that the container will immediately begin running
the stream.py script upon being started. This script loads stream_config.yaml, 
initializes the G3Pipeline and all of the modules that will be run, and runs it.
As soon as packets begin appearing at the source IP address, files will begin
to be written. It is best to not have data streaming when the streamer container
is brought up and to stop data streaming before bringing down the container.
If data is still streaming when the container ends, the last frame will likely be
corrupted. 

The stream_config.yaml file should follow the pattern given in `scripts/`. The `g3_dir`
key sets the high-level path to the g3 files. Within the /data directory, which is mapped
in the docker compose file to somewhere in your computer's file system, the normal
structure of the output filetree might look like the following example:
```
|-- g3_dir/
|   |-- <session_id>
|   |   |-- rfsoc01drone1
|   |   |   |-- r01d0_<10-digit ctime>_000.g3
|   |   |   |-- r01d0_<10-digit ctime>_001.g3
|   |   |-- rfsoc01drone2
|   |   |   |-- r01d1_<10-digit ctime>_000.g3
|   |   |   |-- r01d1_<10-digit ctime>_001.g3
|   |-- <session_id>
|   |   |-- rfsoc02drone1
|   |   |   |-- r02d1_<10-digit ctime>_000.g3
|   |   |   |-- r02d1_<10-digit ctime>_001.g3
|   |   |   |-- r02d1_<10-digit ctime>_002.g3
```
Within the `g3_dir`, files are organized by the `session_id`. This is the first five
digits of a ten-digit Linux timestamp, so it corresponds roughly to a single day. The 
next level down is the `stream_id` set in the stream_config.yaml file - it should have the
format `rfsoc##drone#` or it will not be parsed correctly for the g3 file names. All of the files
made by a given drone on a given RFSoC will end up in these directories until `session_id` changes.

The naming convention of the g3 files is `r##d#_<10-digit ctime>_<sequence number>.g3`. The numbers
following r and d are parsed from the `stream_id` so that each file is tagged with the same RFSoC and
drone number. The ten-digit ctime is the time when a given stream began - different streams will have
different ctimes. The sequence number is from the file rotation. When a file accumulates data for
longer than the `file_rotation_time` variable set in stream_config.yaml, that file will be closed
and a new g3 file will begin with the sequence number incremented by one.

In the example file tree given above, there was a stream run on one day (corresponding to the
first session_id) that took data on rfsoc01, drones 1 and 2 for long enough to generate two files.
On another day (corresponding to the second session_id), another stream was taken with only drone 1
on rfsoc02, and it ran long enough to generate three files.

## Installation outside of Docker
It remains possible to install and use the code outside of the Docker containers for testing
and debugging. In this section, instructions are provided for setting up a working 
environment for a Ubuntu 22.04 LTS instance running Python3.10 and running it either 
in pure C++ or Python. The code has not been 
tested extensively on other versions, and paths may need to be changed to work on other 
Linux instances. We do not support Windows or Mac outside of Docker.

### Building base environment
```
# on an instance running ubuntu22
# confirmed /usr/bin/python3 is running 3.10

# set up ssh keys or other authentication method to enable git clone from github

sudo apt-get update
sudo apt install gcc g++
sudo apt install make
sudo apt install python3-dev
sudo apt-get install libboost-all-dev
sudo apt install python3.10-venv 
sudo apt install libopenblas-openmp-dev libflac-dev bzip2 libbz2-dev 
sudo apt install libgsl-dev
#
python3 -m venv streamer_test
source streamer_test/bin/activate
pip install --upgrade pip
#
mkdir ~/git
cd ~/git
git clone git@github.com:simonsobs/so3g.git
git clone git@github.com:CMB-S4/spt3g_software.git
git clone git@github.com:ccatobs/rfsoc-streamer.git
#
cd ~/git/so3g
pip install -r requirements.txt    # This also installs cmake
mkdir ~/so3g
#
cd ~/git/spt3g_software
mkdir -p build
cd build

cmake \
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

make
make install
#
cd ~/git/so3g
mkdir -p build
cd build

cmake \
  -DCMAKE_PREFIX_PATH=${HOME}/git/spt3g_software/build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
  -DPYTHON_INSTALL_DEST="${HOME}/so3g" \
  -DCMAKE_INSTALL_PREFIX="${HOME}/so3g" \
  ..

make -j 4
make install
#
cd ~/git/so3g
mkdir -p build
cd build

cmake \
  -DCMAKE_PREFIX_PATH=${HOME}/git/spt3g_software/build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
  -DPYTHON_INSTALL_DEST="${HOME}/so3g" \
  -DCMAKE_INSTALL_PREFIX="${HOME}/so3g" \
  ..

make -j 4
make install
#
```
### Building and running rfsoc-streamer

Various environment variables are set in ~/.bashrc to point to so3g, spt3g, rfsoc_streamer, etc.

For the Python version to find everything with using CMakeLists.txt, the following environment variables
must be set:
```
export SPT3G_SOFTWARE_PATH=${HOME}/git/spt3g_software
export SO3G_DIR=${HOME}/git/so3g
export SPT3G_SOFTWARE_BUILD_PATH=${HOME}/git/spt3g_software/build
export SO3G_BUILD_PATH=${HOME}/so3g/lib/python3.10/site-packages
export RFSOC_DIR=${HOME}/git/rfsoc-streamer
export LD_LIBRARY_PATH=$SPT3G_SOFTWARE_BUILD_PATH/lib:$LD_LIBRARY_PATH
export PYTHONPATH="${RFSOC_DIR}/lib:${RFSOC_DIR}/python:$SPT3G_SOFTWARE_BUILD_PATH:$SO3G_BUILD_PATH:$PYTHONPATH"
export PATH="${HOME}/so3g/bin:${PATH}"
export STREAM_CONFIG_DIR=${HOME}/git/rfsoc-streamer/test
```
These default values should work if you followed the steps for setting up the environment above.

If using outside the Docker container, the path to the numpy include files (`include_directories(/opt/venv/lib/python3.10/site-packages/numpy/core/include)`)
also needs to be commented out or explictly pointed to the numpy include files in the venv.

#### Python-wrapped version

```
source ~/streamer_test/bin/activate    # activate the python virtual environment

cd ~/git/rfsoc-streamer
cmake .    # this will create multiple cmake-related files and a Makefile
make       # or "VERBOSE=1 make" if you want more information printed out during make

python scripts/stream.py

# run this for a little while, then terminate with Ctrl-C
# this should produce one more g3 files under ~/data/g3, with timestamp information distinguishing files

# if you want to either write to a different directory, 
# or receive packets from a different IP address,
# or change the file_dur (file duration before rotating),
# modify the code in scripts/stream.py and scripts/stream_config.yaml and re-run the python code
```

#### Pure C++ version

```
cd ~/git/rfsoc-streamer
cd src
make

cd ../scripts
make

./stream_min_example

# run this for a little while, then terminate with Ctrl-C
# If you begin streaming on an IP address with no packets, this will 
# hang on Ctrl-C - use Ctrl-Z to move to background, then kill the process.
# should produce a g3 file in ~/data/test.g3

# if you want to either write to a different filename, 
# or receive packets from a different IP address,
# modify the code in stream_min_example.cxx and remake the executable

make inspect_g3             # this will make inspect_g3
./inspect_g3 <a_g3_file>.g3 # this will read a_g3_file.g3 and print a summary of its contents to stdout
```
