############################################################################################
#LIBRARIES #################################################################################
############################################################################################
# Data management: regular expressions and value counting
import re
from collections import Counter

############################################################################################
# EXTRA FUNCTIONS ##########################################################################
############################################################################################
def detectPredominantNumber(segment):
    count = Counter(segment)
    return count.most_common(1)[0][0]

def reorderLabels(labels,partition):
    n = len(labels)
    part = n // partition
    predominantNumbers = []
    for i in range(partition):
        start, end = i * part, (i + 1) * part
        predominantNumbers.append(detectPredominantNumber(labels[start:end]))
    mapping = {predominantNumbers[i]: i for i in range(partition)}
    reorderedLabels = [mapping[num] for num in labels]
    return np.array(reorderedLabels)

def stringToNumpyArray(s):
    labels_list = list(map(int, re.findall(r'\d+', s)))
    return np.array(labels_list)

############################################################################################
# EXECUTION ################################################################################
############################################################################################
#Example: 4 partitions and the following labels
labelsStr = "[3 3 3 2 0 2 3 0 3 3 3 1 1 1 1 0 0 0 0 0 3 3 3 3 3 3]"
labelsArray = stringToNumpyArray(labelsStr)
reorderedLabels = reorderLabels(labelsArray,4)
print(reorderedLabels)