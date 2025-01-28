from spt3g import core
import ccatrfsoccore # The wrapped C++ library
import ccatrfsoc     # The pure Python library
import sys
import os

# Could eventually load config file if desired
# This would be a convenient way to set the g3 directory,
# the IP address of the particular drone, and more.

g3_dir = '/home/streamer/data/g3/'
source_ip = '192.168.3.58'
# board ID? Drone ID?

# Stream ID?
stream_id = 'test'

def main() -> None:
# Initializing the Python parts of the G3Pipeline
    builder = ccatrfsoccore.RfsocBuilder(options)
    transmitter = ccatrfsoccore.RfsocTransmitter(builder, options)
    transmitter.Start()

    pipe = core.G3Pipeline()
    pipe.Add(builder)
    pipe.Add(ccatrfsoc.SessionManager.SessionManager, stream_id=stream_id)
    pipe.Add(ccatrfsoc.G3Rotator, g3_dir, file_dur=10*60)
    pipe.Run(profile=True)

if __name__ == '__main__':
    main()
