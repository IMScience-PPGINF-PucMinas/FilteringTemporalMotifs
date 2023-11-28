# Filtering safe temporal motifs in dynamic graphs for dissemination purposes

Our work is based on "A Temporal Graphlet Kernel For Classifying Dissemination in Temporal Graphs".

The source code for their paper is available in: https://gitlab.com/tgpublic/tgraphlet.

After the compilation you need to add our TemporalReduction.cpp/.h algorithm into the root folder of the project. The computeTemporalReduction function filter the temporal graph provided, removing the safe motifs. The commented lines filter the "unsafe motifs" for study analysis.

You are free to call the funcion in any temporal graphlet kernel provided by the previous work.
Example is provided in TemporalWedgeKernel.cpp.

## Classification Task

For the classification task we used the TUDataset benchmark available in: https://github.com/chrsmrrs/tudataset

- The transitive reduction algorithm used was the one povided by sklearn lib. In "convertToTR.py" is the code used to read the temporal graph and replace the edges and edges labels according to the Transitive reduction removal.

- For the classification task we used 2 different kernels, graphlet and Weisfeiler–Lehman kernels. The example is provided in "classification.py" file.

## Data Sets

See [Benchmark Data Sets for Graph Kernels](https://graphlearning.io/) for data sets.

## Cite

@misc{10.48550/arxiv.2209.07332,
doi = {10.48550/ARXIV.2209.07332},
url = {https://arxiv.org/abs/2209.07332},
author = {Oettershagen, Lutz and Kriege, Nils M. and Jordan, Claude and Mutzel, Petra},
title = {A Temporal Graphlet Kernel for Classifying Dissemination in Evolving Networks},
publisher = {arXiv},
year = {2022}
}
