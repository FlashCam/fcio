import cppyy
import numpy as np
import sysconfig

try:
    from importlib.resources import files as import_files
except ImportError:
    from importlib_resources import files as import_files

package_base = import_files("fcio")
fcio_header_path = (package_base / "include" / "fcio.h")
fcio_lib_path = (package_base / f"fcio_c{sysconfig.get_config_vars('SO')[0]}")

cppyy.include(str(fcio_header_path))
cppyy.load_library(str(fcio_lib_path))


class Event:
  def __init__(self, raw_event_struct, config):
    self.buffer = raw_event_struct
    self.config = config
    self.nadcs = self.config.nadcs
    self.nsamples = self.config.nsamples
    self.ntriggers = self.config.ntriggers

    self._baseline = np.ndarray(buffer=self.buffer.traces,
                                dtype=np.uint16,
                                shape=(self.nadcs, ),
                                offset=0,
                                strides=((self.nsamples+2) * 2,))
    self._baseline.setflags(write=False)

    self._integrator = np.ndarray(buffer=self.buffer.traces,
                                  dtype=np.uint16,
                                  shape=(self.nadcs, ),
                                  offset=2,
                                  strides=((self.nsamples+2) * 2,))
    self._integrator.setflags(write=False)

    self._traces = np.ndarray(buffer=self.buffer.traces,
                              dtype=np.uint16,
                              shape=(self.nadcs, self.nsamples),
                              offset=4,
                              strides=((self.nsamples+2) * 2, 2))
    self._traces.setflags(write=False)

    self._triggertraces = np.ndarray(buffer=self.buffer.traces,
                                     dtype=np.uint16,
                                     shape=(self.config.ntriggers, self.nsamples),
                                     offset=((self.nsamples+2) * self.nadcs) * 2 + 4,
                                     strides=((self.nsamples+2) * 2, 2))
    self._triggertraces.setflags(write=False)

  @property
  def numtraces(self):
    return self.buffer.num_traces

  @property
  def tracelist(self):
    """
    Returns the list of triggered adcs for the current event
    return np.ndarray(shape=(self.nadcs), dtype=np.int, buffer=tracelist_view)
    """
    return np.ndarray(shape=(self.numtraces), dtype=np.uint16, buffer=self.buffer.trace_list)

  @property
  def traces(self):
    """
    Returns an numpy array with a view set to the data fields in the traces array of the FCIOData struct.
    """
    return self._traces[:self.numtraces]

  @property
  def triggertraces(self):
    """
    Returns an numpy array with a view set to the triggersum fields in the traces array of the FCIOData struct.
    """
    if self.numtraces != self.nadcs:
      raise Exception("Trying to access the trigger traces while reading a FCIOSparseEvent, which don't contain trigger traces.")
    return self._triggertraces

  @property
  def baseline(self):
    return self._baseline[:self.numtraces] / self.config.blprecision

  @property
  def daqenergy(self):
    return (self._integrator[:self.numtraces] - self._baseline[:self.numtraces]) * (self.config.sumlength / self.config.blprecision)

  @property
  def pulser(self):
    return self.buffer.pulser

  @property
  def eventtype(self):
    return self.buffer.type

  @property
  def last_sample_period_ns(self):
    return 1 / (self.timestamp_maxticks + 1) * 1e9

  @property
  def runtime_ns(self):
    sample_period = 1 / (self.timestamp_maxticks + 1) * 1e9
    event_ns = np.int64(self.timestamp_ticks * sample_period)
    return np.int64(self.timestamp_pps * 1e9) + event_ns

  @property
  def runtime_imprecise(self):
    return self.timestamp_pps + self.timestamp_ticks / (self.timestamp_maxticks + 1)

  @property
  def eventtime_ns(self):
    return np.int64(self.buffer.timeoffset[2] * 1e9) + self.runtime_ns

  @property
  def eventtime_imprecise(self):
    return self.buffer.timeoffset[2] + self.runtime_imprecise

  @property
  def eventnumber(self):
    return self.buffer.timestamp[0]

  @property
  def timestamp_pps(self):
    return self.buffer.timestamp[1]

  @property
  def timestamp_ticks(self):
    return self.buffer.timestamp[2]

  @property
  def timestamp_maxticks(self):
    return self.buffer.timestamp[3]

  """
    fcio_event 'timeoffset' fields
  """

  @property
  def timeoffset_mu_sec(self):
    return self.buffer.timeoffset[0]

  @property
  def timeoffset_mu_usec(self):
    return self.buffer.timeoffset[1]

  @property
  def timeoffset_master_sec(self):
    return self.buffer.timeoffset[2]

  @property
  def timeoffset_dt_mu_usec(self):
    return self.buffer.timeoffset[3]

  @property
  def timeoffset_abs_mu_usec(self):
    return self.buffer.timeoffset[4]

  @property
  def timeoffset_start_sec(self):
    return self.buffer.timeoffset[5]

  @property
  def timeoffset_start_usec(self):
    return self.buffer.timeoffset[6]

  """
    fcio_event 'deadregion' fields
  """

  @property
  def deadregion_start_pps(self):
    return self.buffer.deadregion[0]

  @property
  def deadregion_start_ticks(self):
    return self.buffer.deadregion[1]

  @property
  def deadregion_stop_pps(self):
    return self.buffer.deadregion[2]

  @property
  def deadregion_stop_ticks(self):
    return self.buffer.deadregion[3]

  @property
  def deadregion_maxticks(self):
    return self.buffer.deadregion[4]

  @property
  def deadtime_imprecise(self):
    return (self.deadregion_stop_pps-self.deadregion_start_pps) + (self.deadregion_stop_ticks-self.deadregion_start_ticks)/(self.deadregion_maxticks+1)

  @property
  def deadtime_ns(self):
    sample_period = 1 / (self.deadregion_maxticks + 1) * 1e9
    deadtime_ns = (self.deadregion_stop_ticks-self.deadregion_start_ticks) * sample_period
    return np.int64(self.deadregion_stop_pps-self.deadregion_start_pps) + np.int64(deadtime_ns)


class Config:
  def __init__(self, raw_event_struct):
    self.buffer = raw_event_struct

  @property
  def nsamples(self):
    return self.buffer.eventsamples

  @property
  def nadcs(self):
    return self.buffer.adcs

  @property
  def telid(self):
    return self.buffer.telid

  @property
  def ntriggers(self):
    return self.buffer.triggers

  @property
  def adcbits(self):
    return self.buffer.adcbits

  @property
  def sumlength(self):
    return self.buffer.sumlength

  @property
  def blprecision(self):
    return self.buffer.blprecision

  @property
  def mastercards(self):
    return self.buffer.mastercards

  @property
  def triggercards(self):
    return self.buffer.triggercards

  @property
  def adccards(self):
    return self.buffer.adccards

  @property
  def gps(self):
    return self.buffer.gps


class CardStatus:
  def __init__(self, raw_card_status_struct):
    self.buffer = raw_card_status_struct

  @property
  def reqid(self):
    return self.buffer.reqid

  @property
  def status(self):
    return self.buffer.status

  @property
  def eventno(self):
    return self.buffer.eventno

  @property
  def pps(self):
    return self.buffer.pps

  @property
  def ticks(self):
    return self.buffer.ticks

  @property
  def maxticks(self):
    return self.buffer.maxticks

  @property
  def numenv(self):
    return self.buffer.numenv

  @property
  def numctilinks(self):
    return self.buffer.numctilinks

  @property
  def numlinks(self):
    return self.buffer.numlinks

  @property
  def dumm(self):
    return self.buffer.dumm

  @property
  def totalerrors(self):
    return self.buffer.totalerrors

  @property
  def othererrors(self):
    return np.ndarray(shape=(5), dtype=np.uint16, offset=0, buffer=self.buffer.othererrors)

  @property
  def environment(self):
    return np.ndarray(shape=(self.numenv), dtype=np.uint16, offset=0, buffer=self.buffer.environment)

  @property
  def ctierrors(self):
    return np.ndarray(shape=(self.numctilinks), dtype=np.uint16, offset=0, buffer=self.buffer.ctierrors)

  @property
  def linkerrors(self):
    return np.ndarray(shape=(self.numlinks), dtype=np.uint16, offset=0, buffer=self.buffer.linkerrors)

  @property
  def enverrors(self):
    return np.ndarray(shape=(self.numlinks), dtype=np.uint16, offset=0, buffer=self.buffer.enverrors)


class Status:
  def __init__(self, raw_event_struct):
    self.buffer = raw_event_struct

  @property
  def status(self):
    return self.buffer.status

  @property
  def statustime(self):
    return self.buffer.statustime

  @property
  def cards(self):
    return self.buffer.cards

  @property
  def size(self):
    return self.buffer.size

  @property
  def data(self):
    for i in range(self.cards):
      yield CardStatus(self.buffer.data[i])


class FCIO:
  def __init__(self, filename : str, timeout : int = 0, buffersize : int = 0, debug : int = 0):

    self.filename = str(filename)
    self.timeout = int(timeout)
    self.buffersize = int(buffersize)
    self.debug = int(debug)
    self.buffer = None
    cppyy.gbl.FCIODebug(self.debug)

    self.FCIOEvent = cppyy.gbl.FCIOEvent
    self.FCIOSparseEvent = cppyy.gbl.FCIOSparseEvent
    self.FCIOConfig = cppyy.gbl.FCIOConfig
    self.FCIOStatus = cppyy.gbl.FCIOStatus

    self.open()

  def __enter__(self):
    self.open()
    return self

  def __exit__(self, exc_type, exc_val, exc_tb):
    self.close()

  def __del__(self):
    self.close()

  def open(self):
    if self.buffer:
      self.close()
    self.buffer = cppyy.gbl.FCIOOpen(self.filename, self.timeout, self.buffersize)
    tag = 1
    while tag > 0:
      tag = self.get_record()
      if tag == self.FCIOConfig:
        break
    self.status = Status(self.buffer.status)

  def close(self):
    if self.buffer:
      cppyy.gbl.FCIOClose(self.buffer)
      self.buffer = None

  def get_record(self):
    if self.buffer:
      tag = cppyy.gbl.FCIOGetRecord(self.buffer)
      if tag == self.FCIOConfig:
        self.config = Config(self.buffer.config)
        self.event = Event(self.buffer.event, self.config)
      return tag
    else:
      raise Exception(f"File {self.filename} not opened.")

  @property
  def records(self):
    tag = 1
    while tag > 0:
      tag = self.get_record()
      yield tag

  @property
  def events(self):
    tag = 1
    while tag > 0:
      tag = self.get_record()
      if tag == self.FCIOEvent or tag == self.FCIOSparseEvent:
        yield self.event

  @property
  def statuses(self):
    tag = 1
    while tag > 0:
      tag = self.get_record()
      if tag == self.FCIOStatus:
        yield self.status
