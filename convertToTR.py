from grakel.graph import Graph
from sklearn.utils import Bunch
import numpy as np
import sys
import networkx as nx
import time
import os

def read_data(
        name,
        with_classes=True,
        as_graphs=False,
        is_symmetric=False):
    indicator_path = "./datasets/"+ str(name) + "/"+str(name)+"/raw/"+str(name)+"_graph_indicator.txt"
    edges_path = "./datasets/"+ str(name) + "/" + str(name) + "/raw/" + str(name) + "_A.txt"
    node_labels_path = "./datasets/"+ str(name) + "/" + str(name) + "/raw/" + str(name) + "_node_labels.txt"
    edge_attributes_path = \
        "./datasets/"+ str(name) + "/" + str(name) + "/raw/" + str(name) + "_edge_attributes.txt"
    graph_classes_path = \
        "./datasets/"+ str(name) + "/" + str(name) + "/raw/" + str(name) + "_graph_labels.txt"

    # node graph correspondence
    ngc = dict()
    # edge line correspondence
    elc = dict()
    # dictionary that keeps sets of edges
    Graphs = dict()
    # dictionary of labels for nodes
    node_labels = dict()
    # dictionary of labels for edges
    edge_labels = dict()

    # Associate graphs nodes with indexes
    with open(indicator_path, "r") as f:
        for (i, line) in enumerate(f, 1):
            ngc[i] = int(line[:-1])
            if int(line[:-1]) not in Graphs:
                Graphs[int(line[:-1])] = set()
            if int(line[:-1]) not in node_labels:
                node_labels[int(line[:-1])] = dict()
            if int(line[:-1]) not in edge_labels:
                edge_labels[int(line[:-1])] = dict()

    # Extract graph edges
    with open(edges_path, "r") as f:
        for (i, line) in enumerate(f, 1):
            edge = line[:-1].replace(' ', '').split(",")
            elc[i] = (int(edge[0]), int(edge[1]))
            Graphs[ngc[int(edge[0])]].add((int(edge[0]), int(edge[1])))
            if is_symmetric:
                Graphs[ngc[int(edge[1])]].add((int(edge[1]), int(edge[0])))

    # Extract node labels
    with open(node_labels_path, "r") as f:
        for (i, line) in enumerate(f, 1):
            labels = line[:-1].replace(' ', '').split(",")
            if len(labels) > 3:
                node_labels[ngc[i]][i] = (int(labels[2]), int(labels[3]))
            else:
                node_labels[ngc[i]][i] = (0,0)

    # Extract edge attributes
    with open(edge_attributes_path, "r") as f:
        for (i, line) in enumerate(f, 1):
            label = float(line[:-1])
            edge_labels[ngc[elc[i][0]]][elc[i]] = label
            if is_symmetric:
                edge_labels[ngc[elc[i][1]]][(elc[i][1], elc[i][0])] = label


    Gs = list()
    if as_graphs:
        for i in range(1, len(Graphs)+1):
            Gs.append(Graph(Graphs[i], node_labels[i], edge_labels[i]))
    else:
        for i in range(1, len(Graphs)+1):
            Gs.append([Graphs[i], node_labels[i], edge_labels[i]])

    if with_classes:
        classes = []
        with open(graph_classes_path, "r") as f:
            for line in f:
                classes.append(int(line[:-1]))

        classes = np.array(classes, dtype=np.int)
        return Bunch(data=Gs, target=classes)
    else:
        return Bunch(data=Gs)




def remove_cycles(G, node, visited, parent):
    visited.add(node)
    for neighbor in G.neighbors(node):
        if neighbor not in visited:
            remove_cycles(G, neighbor, visited, node)
        elif neighbor != parent:
            G.remove_edge(node, neighbor)

def modified_DFS(oldGraph):
    G = oldGraph
    visited = set()
    for node in G.nodes():
        if node not in visited:
            remove_cycles(G, node, visited, None)
    return G





for dataset in os.listdir("./datasets"):
    edges_path = f"./{str(dataset)}_A.txt"
    edge_attributes_path = f"./{str(dataset)}_edge_attributes.txt"
    allDataset = []
    data_att = read_data(dataset)
    data = data_att.data
    target = data_att.target
    edges = open(edges_path, "a")
    edge_attributes = open(edge_attributes_path, "a")
    execution_time = 0
    for graph in data:
        #graph[0] #edges
        #graph[1] #node_labels
        #graph[2] # edge_labels
        G = nx.DiGraph()
        G.add_edges_from(graph[0])
        G.add_nodes_from(graph[1])
        start_time = time.time()
        TR = nx.transitive_reduction(G)
        end_time = time.time()
        execution_time += (end_time - start_time) * 1000
        # TR = modified_DFS(G)
        edgesSorted=sorted(TR.edges(data=True), key=lambda t: t[0])
        for edge in edgesSorted:
            edges.write(f'{edge[0]}, {edge[1]}\n')
            edge_attributes.write(f'{graph[2][(edge[0],edge[1])]}\n')


    print(f"Execution time of {dataset}: {execution_time}")
    edges.close()
    edge_attributes.close()