# -*- coding: utf-8 -*-
# Mode: Python; indent-tabs-mode: nil; python-indent-offset: 4; tab-width: 4
# Copyright (c) 2023 Universidad del País Vasco / Euskal Herriko Unibertsitatea (UPV/EHU)
# Copyright (c) 2022, scikit-learn developers
# Distributed under the BSD 3-Clause License.
# See https://scikit-learn.org/stable/licence.html for more information.

############################################################################################
# LIBRARIES ################################################################################
############################################################################################
# Numeric data management libraries
import pandas as pd
import numpy as np
import seaborn as sb

# Data plotting
import matplotlib.pyplot as plt

# Generic SK-Learn resources
from sklearn.preprocessing import MinMaxScaler

# Birch
from sklearn.cluster import Birch

# Data management: regular expressions and value counting
import re
from collections import Counter

############################################################################################
# EXTRA FUNCTIONS ##########################################################################
############################################################################################
def detect_predominant_number(segment):
    positive_segment = [num for num in segment if num >= 0]
    count = Counter(positive_segment)
    return count.most_common(1)[0][0] if positive_segment else None

def reorder_labels(labels,partition):
    n = len(labels)
    part = n // partition
    predominant_numbers = []
    for i in range(partition):
        start, end = i * part, (i + 1) * part
        predominant_numbers.append(detect_predominant_number(labels[start:end]))
    mapping = {predominant_numbers[i]: i for i in range(partition) if predominant_numbers[i] is not None}
    reordered_labels = [mapping.get(num, num) if num >= 0 else num for num in labels]
    return np.array(reordered_labels)

def stringToNumpyArray(s):
    labels_list = list(map(int, re.findall(r'\d+', s)))
    return np.array(labels_list)
############################################################################################
# WORKING ENVIRONMENT DEFINITION ###########################################################
############################################################################################
# Graphic representation characteristics: sizes and style
np.set_printoptions(threshold=np.inf)

plt.rcParams['figure.figsize'] = (16, 9)
plt.style.use('seaborn-dark')

# File dump to DataFrame
filename ="Scenarios-STAStats.csv"
dataframe = pd.read_csv(filename)
print(dataframe.head())                               # Show first 5 lines

# Get data statistics
print(dataframe.describe())                         # Generate descriptive statistics
print(dataframe.groupby('scenario').size())         # Get sample number per environment

# Sample distribution histogram
dataframe.drop(['scenario'] ,axis=1).hist()         # Scenario axis histogram
plt.show()

# Variable crossing
sb.pairplot(dataframe.dropna(), hue='scenario', palette="dark",
            vars=["snr" ,"avgrxpackets" ,"avgthroughput"])
plt.show()

############################################################################################
# PREPROCESSING ############################################################################
############################################################################################
# Mapping for non numeric parameters
mapping_strings = {'A' :1, 'B': 2, 'C': 3, 'D': 4, 'E': 5, 'STA': 1, 'AP': 2}
mapped_dataframe = dataframe.replace(mapping_strings)
mapped_dataframe['address' ] = mapped_dataframe['address'].str.replace(r'192\.168\.\d+\.', '', regex=True)

# Data scaling
min_max_scaler = MinMaxScaler()
array_scaled = min_max_scaler.fit_transform(mapped_dataframe)
dataframe_scaled = pd.DataFrame(array_scaled, columns=mapped_dataframe.columns)

# Scaled file generation
df = pd.DataFrame(dataframe_scaled)
df.to_csv(r"Scenarios-STAStats-scaled.csv")

# Data structure definition
X = np.array(dataframe_scaled[["avgthroughput", "avgrxpackets", "snr",
                               "avgtotaljitter", "avgtotaldelay", "avgtxpackets"]])
Y = np.array(mapped_dataframe['scenario'])
print(X.shape)  # X axis check
print(Y.shape)  # Y axis check

# 3D plotting of samples
plt.rcParams['figure.figsize'] = (17, 17)
figure1= plt.figure()
axes1 = figure1.add_subplot(111, projection='3d')
axes1.set_xlabel('avgtotaljitter', fontsize=22)
axes1.set_ylabel('snr', fontsize=22)
axes1.set_zlabel('avgrxpackets', fontsize=22)
colors1 =['black' ,'black' , 'black', 'black', 'black']
asignment1 =[]

# Assing colors to each sample
for row in Y:
    asignment1.append(colors1[row])
axes1.scatter(X[:, 3], X[:, 2], X[:, 1], c=asignment1)

# Plot display
plt.show()

# Label based display
figure1= plt.figure()
axes1 = figure1.add_subplot(111, projection='3d')
colors1 = ['green' ,'blue' ,'red' ,'orange' ,'green']
asignment1 =[]

scenario_ids = mapped_dataframe['scenario'].astype(int).tolist()
for row in scenario_ids:
    asignment1.append(colors1[row])
axes1.scatter(X[:, 3], X[:, 2], X[:, 1], c=asignment1)
axes1.set_xlabel('avgtotaljitter', fontsize=22)
axes1.set_ylabel('snr', fontsize=22)
axes1.set_zlabel('avgrxpackets', fontsize=22)
plt.title('Labeled samples')
plt.show()

############################################################################################
# BIRCH ####################################################################################
############################################################################################
# Clustering settings
birch = Birch(n_clusters=3, branching_factor=4, threshold=0.5).fit(X)

# Cluster prediction
labels = birch.predict(X)
print(labels)
labels = reorder_labels(labels,4)
print(labels)

############################################################################################
# RESULT DISPLAY ###########################################################################
############################################################################################
# Label based clustering
plt.rcParams['figure.figsize'] = (17, 17)
figure1 = plt.figure()
axes1 = figure1.add_subplot(111, projection='3d')
colors1 =['blue' ,'red' ,'orange' ,'green' ,'blue']
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

# Show how much samples are in each cluster
copy = pd.DataFrame()
dataframe_scaled.reset_index(inplace=True)
print(dataframe_scaled)
copy['index' ] =dataframe_scaled['index'].values
copy['scenario' ] =dataframe_scaled['scenario'].values
copy['label'] = labels

colors1 =['blue' ,'red' ,'orange' ,'green']

tam_clusters = pd.DataFrame()
tam_clusters['color' ] =colors1
tam_clusters['quantity' ] =copy.groupby('label').size()
print(tam_clusters)

############################################################################################
# RESULT ANALYSIS ##########################################################################
############################################################################################
# Check scenario based cluster hypothesis
scenario_ids = [0, 1, 2, 3]

for i in scenario_ids:
    group_referrer_index = copy['label'] == i
    group_referrals = copy[group_referrer_index]
    group_diversity = pd.DataFrame()
    group_diversity['scenario'] = scenario_ids
    group_diversity['quantity'] = group_referrals.groupby('scenario').size().reset_index(drop=True)
    group_diversity.fillna(0, inplace=True)
    group_diversity['quantity'] = group_diversity['quantity'].astype(int)
    print(f"Assignment of label {i} per scenario")
    print(group_diversity)
    print("\n")



