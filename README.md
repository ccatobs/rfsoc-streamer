```
rfsoc-streamer

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
cd ~/git/rfsoc-streamer
cd src
make

cd ../test
make

export LD_LIBRARY_PATH=/home/ubuntu/so3g/lib:/home/ubuntu/so3g/so3g
./stream_min_example   # this will eventually segfault, but not before creating an empty test.g3 in the data directory

make cppexample           # this will make cppexample
./cppexample a_g3_file.g3 data/test2.g3   # this will read a_g3_file.g3 and write the contents to data/test2.g3
```
