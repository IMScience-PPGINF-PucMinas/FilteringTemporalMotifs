import auxiliarymethods.auxiliary_methods as aux
import auxiliarymethods.datasets as dp
import kernel_baselines as kb
from auxiliarymethods.kernel_evaluation import kernel_svm_evaluation
import timeit
import os
import numpy as np

# Download dataset.


def calculateGraphLet(name):
    all_matrices = []
    gm = kb.compute_graphlet_dense(name, True, False)
    gm = aux.normalize_gram_matrix(gm)
    all_matrices.append(gm)
    classes = dp.get_dataset(name)
    # kernel_svm_evaluation(all_matrices, classes,
    #                         num_repetitions=10, all_std=False)
    print("GL: ")
    print(kernel_svm_evaluation(all_matrices, classes,
                            num_repetitions=10, all_std=False))


def calculateW1(name: str):
    all_matrices = []
    for i in range(1, 6):
        gm = kb.compute_wl_1_dense(name, i, True, False)
        gm = aux.normalize_gram_matrix(gm)
        all_matrices.append(gm)
        # print(gm.shape)
    classes = dp.get_dataset(name)
    # kernel_svm_evaluation(all_matrices, classes,
    #                             num_repetitions=10, all_std=False)
    print("WL: ")
    print(kernel_svm_evaluation(all_matrices, classes,
                                num_repetitions=10, all_std=False))


for name in os.listdir("./datasets"):
    print(name)
    timeGL = timeit.timeit('calculateGraphLet(name)', 'from __main__ import calculateGraphLet, name', number= 1)
    timeW1 = timeit.timeit('calculateW1(name)', 'from __main__ import calculateW1, name', number= 1)
    print(name + "\nTime GL: " + str(timeGL) + "    Time W1: " + str(timeW1) + "\n")
