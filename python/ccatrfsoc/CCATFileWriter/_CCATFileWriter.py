from spt3g import core
import os
import time
from ccatrfsoc.SessionManager import FlowControl

class G3Rotator:
    def __init__(self, data_path, file_dur, debug=0):
        """
        Borrowed heavily from equivalent file in smurf-streamer for SO.
        Modified to work with rfsoc-streamer
        
        From smurf-streamer docstring:
        The G3Rotator is a spt3g module like the G3Writer, but differs in that
        it automatically determines the file-path based off of the incoming
        data stream, and automatically rotates files once a certain duration
        has been reached. It is a lot like the G3MultiFileWriter except more
        specialized to work directly with the so frame stream.

        On a new session with a given session_id (which will always be the int
        timestamp when the session was started, files will be named::

            path = <data_path>/str(sess_id)[:5]/<sess_id>_<seq>.g3

        Because session id is the start time of the session, str(sess_id)[:5]
        is the first 5 ctime digits, which increment in intervals of ~1 day.
        <seq> starts at zero and increments each time the file is rotated. This
        way all files are easily found given the session-id.

        Attributes:
        -----------
        data_path: str
            Base directory where data will be written
        cur_path: str
            Path to the current file being written
        seq: int
            Index of the current file being written. This will increment each
            time a new file is created for an existing streaming session. It
            will be reset to 0 when a new streaming session starts.
        file_start_time: float
            Start time of the current file
        file_dur: float
            Duration of file before rotating in seconds
        cur_session_id: int
            Session id of the current data stream. 0 if there is no data
            streaming.
        debug: bool
            If True, will print frames and frame write time every time a frame
            is written
        disable: bool
            If true, will skip writing frames to disk.
        """
        self.data_path = data_path
        self._writer = None
        self.seq = 0
        self.cur_session_id = 0
        self.file_start_time = 0
        self.file_dur = file_dur
        self.cur_path = ''
        self.debug = debug
        self.disable = False

        # Was code for pysmurf Publisher here - could add if we want something like this for CCAT

    def close_writer(self):
        """
        Closes the current G3Writer if open, and sets the cur_path,
        file_start_time, and cur_session_id to the default
        """
        if self._writer is not None:
            self.file_start_time = 0
            self.cur_session_id = 0
            self._writer(core.G3Frame(core.G3FrameType.EndProcessing))
            # Was a Publisher message here
            self.cur_path = ''
        self._writer = None

    def new_writer(self, frame, seq):
        """
        Determines the filepath and creates a new G3Writer based on a
        G3Frame and seq index.

        Args
        -----
        frame: G3Frame
            frame for which to start the new writer. This will be used to
            dtermine the stream and session id.
        seq: int
            Seq index of the file in the given streaming session
        """
        session_id = frame['session_id']
        stream_id = frame['ccatstream_id']
        board_num = stream_id[5:7]
        drone_num = stream_id[-1]
        subdir = os.path.join(self.data_path, str(session_id)[:5], stream_id)
        fname = f"r{board_num}d{drone_num}_{session_id}_{seq:03}.g3"
        fpath = os.path.join(subdir, fname)

        if not os.path.exists(subdir):
            os.makedirs(subdir)

        self._writer = core.G3Writer(fpath)
        self.cur_path = fpath
        self.file_start_time = time.time()
        self.cur_session_id = session_id

        return self._writer

    def new_file_condition(self):
        """
        Returns true if `file_dur` sec have passed since the file_start_time.
        """
        return time.time() > self.file_start_time + self.file_dur

    def get_writer(self, frame):
        """
        Gets appropriate G3 writer for a given frame based on the following
        rules. Will automatically rotate files if `new_file_condition` has
        been met, and will close out files if an FlowControl.End frame is
        received.
        """
        if 'ccatstream_flowcontrol' in frame:
            if frame['ccatstream_flowcontrol'] == FlowControl.END.value:
                self.close_writer()
            return None

        if 'session_id' not in frame:
            return None
        sess_id = frame['session_id']

        if self.cur_session_id != sess_id:
            self.close_writer()
            self.seq = 0

            return self.new_writer(frame, self.seq)
        elif self.new_file_condition():
            self.close_writer()
            self.seq += 1
            return self.new_writer(frame, self.seq)
        else:
            return self._writer

    def __call__(self, frame):
        if self.disable:
            if self.debug:
                print(frame)
            return [frame]

        writer = self.get_writer(frame)
        if writer is not None:
            start = time.time()
            writer(frame)
            stop = time.time()
            if self.debug:
                print(f"Wrote frame in {stop - start} sec")
                print(frame)
        return [frame]