from spt3g import core
import ccatrfsoccore # The wrapped C++ library
import ccatrfsoc     # The pure Python library
import yaml
import os
import argparse

def main() -> None:
    if 'STREAM_CONFIG_DIR' in os.environ:
        cfg_dir = os.environ['STREAM_CONFIG_DIR']
    else:
        raise ValueError("STREAM_CONFIG_DIR must be set in environment")

    with open(os.path.join(cfg_dir, 'stream_config.yaml')) as f:
        cfg = yaml.safe_load(f)

    g3_dir = cfg['g3_dir']
    file_length = int(cfg['file_rotation_time'])

    # Setting option to pass in board and drone numbers
    # When running with Docker, these will usually be set in the env
    parser = argparse.ArgumentParser()
    parser.add_argument('--board', type=int, default=None,
                        help='The board number for this streamer instance')
    parser.add_argument('--drone', type=int, default=None,
                         help='The drone number for this streamer instance')
    args = parser.parse_args()

    if 'BOARD' in os.environ:
        board_num = int(os.environ['BOARD'])
    elif args.board is not None:
        board_num = args.board
    else:
        raise ValueError("BOARD must be set in env or passed as the 'board' arg")

    if 'DRONE' in os.environ:
        drone_num = int(os.environ['DRONE'])
    elif args.drone is not None:
        drone_num = args.drone
    else:
        raise ValueError("DRONE must be set in env or passed as the 'drone' arg")

    # The Stream ID tells you which board and drone the data is coming from
    stream_id = cfg['rfsocs'][f'BOARD[{board_num}]'][f'DRONE[{drone_num}]']['stream_id']
    # Check that the format is rfsoc??_drone? for later parsing in filenames
    # Probably a cleaner way to do this...
    if len(stream_id) != 14 or stream_id[0:5] != 'rfsoc' or stream_id[7:13] != '_drone':
        raise ValueError("stream_id improperly formatted. Should be like rfsoc##_drone#")

    source_ip = cfg['rfsocs'][f'BOARD[{board_num}]'][f'DRONE[{drone_num}]']['drone_stream_ip']

    print(f"Stream id: {stream_id}")
    print(f"Streaming IP address: {source_ip}")

    # Initializing the Python parts of the G3Pipeline
    # Should eventually add the ability to set the variables
    # that are passed by default to the C++ RfsocBuilder constructor (debug, etc.)
    builder = ccatrfsoccore.RfsocBuilder()
    transmitter = ccatrfsoccore.RfsocTransmitter(builder, source_ip)
    transmitter.Start()

    pipe = core.G3Pipeline()
    pipe.Add(builder)
    pipe.Add(ccatrfsoc.SessionManager.SessionManager, stream_id=stream_id)
    pipe.Add(ccatrfsoc.G3Rotator, data_path=g3_dir, file_dur=file_length)
    pipe.Run(profile=False)

if __name__ == '__main__':
    main()
