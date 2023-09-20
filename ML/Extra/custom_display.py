############################################################################################
#LIBRARIES #################################################################################
############################################################################################
# Numeric data management libraries
import pandas as pd
import numpy as np

# Data plotting
import matplotlib.pyplot as plt

# Data management: regular expressions
import re

############################################################################################
# EXTRA FUNCTIONS ##########################################################################
############################################################################################
def stringToNumpyArray(s):
    labels_list = list(map(int, re.findall(r'\d+', s)))
    return np.array(labels_list)

############################################################################################
# WORKING ENVIRONMENT DEFINITION ###########################################################
############################################################################################
# Graphic representation characteristics: sizes and style
np.set_printoptions(threshold=np.inf)

plt.style.use('seaborn-dark')

# File dump to DataFrame
filename ="Scenarios-APStats-Scaled.csv"
dataframe = pd.read_csv(filename)

# Data structure definition
X = np.array(dataframe[["avgthroughput", "avgrxpackets", "snr",
                               "avgtotaljitter", "avgtotaldelay", "avgtxpackets"]])
Y = np.array(dataframe['scenario'])

############################################################################################
# LABEL DEFINITION #########################################################################
############################################################################################
labels = stringToNumpyArray("1 3 0 1 0 2 2 0 0 3 ... 1 1 1 3")
print(labels)

############################################################################################
# RESULT DISPLAY ###########################################################################
############################################################################################
# Label based clustering
plt.rcParams['figure.figsize'] = (17, 17)
figure1 = plt.figure()
axes1 = figure1.add_subplot(111, projection='3d')
colors1 =['blue' ,'red' ,'orange' ,'green' ,'black']
colors2 = ['black' ,'black' ,'black' ,'black']
asignment1 = []
for row in labels:
    asignment1.append(colors1[row])
axes1.scatter(X[:, 3], X[:, 2], X[:, 1], c=asignment1)
axes1.set_xlabel('avgtotaljitter', fontsize=22)
axes1.set_ylabel('snr', fontsize=22)
axes1.set_zlabel('avgrxpackets', fontsize=22)
plt.title('Clustering Result')
plt.show()


# Bidimensional plots
combinations = [
    {
        "x": dataframe["avgtxpackets"].values,
        "y": dataframe["avgtxpackets"].values - dataframe["avgrxpackets"].values,
        "title": 'avgtxpackets - avglostpackets'
    },
    {
        "x": dataframe["avgtxpackets"].values - dataframe["avgrxpackets"].values,
        "y": dataframe["snr"].values,
        "title": 'avglostpackets - snr'
    },
    {
        "x": dataframe["avgthroughput"].values,
        "y": dataframe["snr"].values,
        "title": 'avgthroughput - snr'
    },
    {
        "x": dataframe["avgtotaldelay"].values,
        "y": dataframe["avgtotaljitter"].values,
        "title": 'avgtotaldelay - avgtotaljitter'
    }
]

plt.rcParams['figure.figsize'] = (7, 7)

for combo in combinations:
    plt.scatter(combo["x"], combo["y"], c=asignment1, s=10)
    plt.title(combo["title"], fontsize=18)
    plt.show()