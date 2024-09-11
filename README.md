```
rfsoc-streamer

# on an instance running ubuntu22
# confirmed /usr/bin/python3 is running 3.10

sudo apt-get update
sudo apt install python3.10-venv
#
python3 -m venv streamer_test1
source streamer_test1/bin/activate
pip install --upgrade pip
#
mkdir ~/git
cd ~/git
git clone git@github.com:simonsobs/so3g.git
#
cd so3g
pip install -r requirements.txt
#
pip install so3g
#
cd ~/git
git clone git@github.com:ccatobs/rfsoc-streamer.git
git clone git@github.com:CMB-S4/spt3g_software.git
#
sudo apt install gcc g++
sudo apt install make
sudo apt-get install libboost-all-dev
#

make                               # this will make stream_min_example
./stream_min_example   # this will eventually segfault, but not before creating an empty test.g3 in the data directory

make cppexample           # this will make cppexample
./cppexample a_g3_file.g3 data/test2.g3   # this will read a_g3_file.g3 and write the contents to data/test2.g3
```
