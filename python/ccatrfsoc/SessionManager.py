from spt3g import core
import signal
import time
from enum import Enum

CCATSTREAM_VERSION = 1

class FlowControl(Enum):
    """Flow control enumeration."""
    ALIVE = 0
    START = 1
    END = 2
    CLEANSE = 3

class SessionManager:
    """
    Class for handling a single streaming session
    and how frames flow through it. Also triggers logic
    for frames at the beginning and end of files.

    Modeled entirely after the SessionManager.py in smurf-streamer
    except with changes to accommodate how the RFSoC for CCAT
    communicates the beginning and end of files as well as
    Wiring frames.
    """

    def __init__(self, stream_id=''):
        self.stream_id = stream_id
        self.session_id = None
        self.end_session_flag = False
        self.frame_num = 0
        self.status = {}

    def signal_handler(self,sig,frame):
        """
        Handler to exit appropriately on SIGINT for edge
        case where data stream never started so no
        frames are being sent.
        This lets the FileWriter know that it should end.
        """
        self.session_id = None
        self.end_session_flag = False
        self.frame_num = 0
        return self.flowcontrol_frame(FlowControl.END)

    def flowcontrol_frame(self, fc):
        """
        Creates flow control frame.

        Args:
            fc (int):
                flow control type
        """
        frame = core.G3Frame(core.G3FrameType.none)
        frame['ccatstream_flowcontrol'] = fc.value
        return frame

    def tag_frame(self, frame):
        frame['ccatstream_version'] = CCATSTREAM_VERSION
        frame['ccatstream_id'] = self.stream_id
        frame['frame_num'] = self.frame_num
        self.frame_num += 1
        if self.session_id is not None:
            frame['session_id'] = self.session_id
        if 'time' not in frame:
            frame['time'] = core.G3Time.Now()

        return frame

    def start_session(self):
        self.session_id = int(time.time())
        frame = core.G3Frame(core.G3FrameType.Observation)
        frame['stream_placement'] = 'start'
        self.tag_frame(frame)
        return frame

    def __call__(self, frame):
        # Always default to passing the frame out to the file rotator
        # unless flow control logic changes this
        out = [frame]

        #######################################
        # On None frames
        #######################################
        if frame.type == core.G3FrameType.none:

            # Add sigint handler to close nicely on Ctrl+C
            signal.signal(signal.SIGINT, self.signal_handler)

            # In absence of wiring frames or registers from RFSoC, 
            # trigger end of session on first None frame following data frames.
            if self.session_id is not None:
                # Returns [previous, end]
                out = []

                end_session_frame = core.G3Frame(core.G3FrameType.Observation)
                end_session_frame['stream_placement'] = 'end'
                self.tag_frame(end_session_frame)

                out.append(end_session_frame)
                out.append(self.flowcontrol_frame(FlowControl.END))

                # Smurf Session Manager has cleanse frames here - related to G3NetworkSender,
                # which we aren't using

                self.session_id = None
                self.end_session_flag = False
                self.frame_num = 0
                return out

        #######################################
        # On Scan frames
        #######################################
        elif frame.type == core.G3FrameType.Scan:

            # In absence of wiring frames or registers from RFSoC, 
            # trigger new session on first data frame after None frames
            if self.session_id is None:
                # Returns [start, session, status] - but no status/wiring yet! Just return data frame
                session_frame = self.start_session()
                out = [
                    self.flowcontrol_frame(FlowControl.START),
                    session_frame,
                    frame
                ]

                return out

            self.tag_frame(frame)
            return out
