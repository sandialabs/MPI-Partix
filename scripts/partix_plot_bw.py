#(c) Sandia National Labs

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sbrn
import pandas as pd
from matplotlib.colors import LogNorm
import sys

sbrn.set_context('paper')
_fontsize = 13
sbrn.set_style("whitegrid")
sbrn.set_style("ticks", {"xtick.major.size": 12, "ytick.major.size": 12})

dataSet = pd.read_table(sys.argv[1], skiprows = 0, header=0, delimiter=",")
print(dataSet.head())
ax = sbrn.lineplot(x="partsize", y="bw(MB)",  hue="partitions",  data=dataSet)

#ax.set_xscale("log")
#ax.set_yscale("log")
ax.set_xlabel('Partition size (bytes)',fontsize=_fontsize);
ax.set_ylabel('Bandwidth(MB/s)',fontsize=_fontsize);

ax.legend(fontsize=_fontsize)
ax.set_title("OMPI Part Comm BW",fontsize=_fontsize)

plt.tight_layout()
plt.show()

