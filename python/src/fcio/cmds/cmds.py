import sys

from fcio import FCIO


def print_status():
  filename = sys.argv[1]
  with FCIO(filename) as io:
    for s in io.statuses:
      print(s.statustime[0], s.statustime[1], s.statustime[2], s.statustime[3], s.status)
  

def print_event():
  filename = sys.argv[1]
  with FCIO(filename) as io:
    for e in io.events:
      print(e.runtime_imprecise)

def plot_events():
  import matplotlib.pyplot as plt

  if len(sys.argv) < 2:
    print("fcio-plot-events <filename> <start,stop>")
    print("  optional: <start,stop> : either only number of events or first event and number of events.")
    sys.exit(1)
  

  elif (len(sys.argv) == 3):
    start_event = 0
    nevents = int(sys.argv[2])

  elif (len(sys.argv) == 4):
    start_event = int(sys.argv[2])
    nevents = int(sys.argv[3])
  else:
    start_event = 0
    nevents = -1

  filename = sys.argv[1]

  with FCIO(filename) as io:
    for i, e in enumerate(io.events):
      if i < start_event:
        continue
      plt.plot(e.traces.T - e.baseline)

      if i == (start_event + nevents - 1):
        print(f"Read {i - start_event} events.")
        break
  plt.show()

def plot_peak_histogram():
  """
  
  """
  import matplotlib.pyplot as plt
  import numpy as np
  
  if len(sys.argv) < 2:
    print("fcio-plot-energy-histogram <filename> <start,stop>")
    print("  uses the daqenergy / integrator value, with it's caveats.")
    print("  optional: <start,stop> : either only number of events or first event and number of events.")
    sys.exit(1)
  

  if (len(sys.argv) == 3):
    start_event = 0
    nevents = int(sys.argv[2])

  elif (len(sys.argv) == 4):
    start_event = int(sys.argv[2])
    nevents = int(sys.argv[3])
  else:
    start_event = 0
    nevents = -1

  filename = sys.argv[1]

  with FCIO(filename) as io:
    amplitudes = []
    for i, e in enumerate(io.events):
      if i < start_event:
        continue
      amplitudes.append(np.max(e.traces, axis=1) - e.baseline)
      if i == (start_event + nevents - 1):
        print(f"Read {i - start_event} events.")
        break

  amplitudes = np.array(amplitudes).T
  for ch in amplitudes:
    hist, edges = np.histogram(ch, bins=1000)
    bincentres = [(edges[i]+edges[i+1])/2. for i in range(len(edges)-1)]
    plt.step(bincentres,hist,where='mid',linestyle='-')

  plt.show()