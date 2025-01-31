# rfsoc-streamer

## Building base environment
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
pip install -r requirements.txt
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

make
make install
#
```
## Building and running rfsoc-streamer

Various environment variables are set in ~/.bashrc to point to so3g, spt3g, rfsoc_streamer, etc.

### Python-wrapped version

```
source ~/streamer_test/bin/activate    # activate the python virtual environment

cd ~/git/rfsoc-streamer
cmake .    # this will create multiple cmake-related files and a Makefile
make       # or "VERBOSE=1 make" if you want more information printed out during make

python test/stream.py

# run this for a little while, then terminate with Ctrl-C
# this should produce one more g3 files under ~/data/g3, with timestamp information distinguishing files

# if you want to either write to a different directory, 
# or receive packets from a different IP address,
# or change the file_dur (file duration before rotating),
# modify the code in test/stream.py and re-run the python code
```

### Pure C++ version

```
cd ~/git/rfsoc-streamer
cd src
make

cd ../test
make

./stream_min_example

# run this for a little while, then terminate with Ctrl-C
# should produce a g3 file in ~/data/test.g3

# if you want to either write to a different filename, 
# or receive packets from a different IP address,
# modify the code in stream_min_example.cxx and remake the executable

make inspect_g3           # this will make inspect_g3
./inspect_g3 a_g3_file.g3 # this will read a_g3_file.g3 and print a summary of its contents to stdout
```
