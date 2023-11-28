#include "TemporalWedgeKernel.h"
#include <unordered_map>
#include <Eigen/Sparse>
#include "GraphKernelCommonFunctions.h"
#include "util/Timer.h"
#include "TemporalReduction.h"
#include "util/DataSetLoader.h"
#include <queue>
#include <random>

using namespace std;
using namespace Eigen;
using SpMatrix = Eigen::SparseMatrix<double>;
using GramMatrix = SpMatrix;

std::vector<uint> estimateWedges(TemporalGraph &tg, uint delta, uint num_samples, bool use_delta)
{
    std::vector<uint> result(32, 0);

    vector<uint> degrees;
    std::vector<double> distribution;
    for (auto &n : tg.nodes)
    {
        double x = n.adjlist.size();
        degrees.push_back(x);
        if (x == 0)
            distribution.push_back(0);
        else
        {
            auto d = 0.5 * x * (x - 1);
            distribution.push_back(d);
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d(distribution.begin(), distribution.end());

    for (int i = 0; i < num_samples;)
    {

        NodeId v = d(gen);
        NodeId u = rand() % tg.nodes[v].adjlist.size();
        NodeId w = rand() % tg.nodes[v].adjlist.size();

        if (u == w)
            continue;

        TemporalEdge e = tg.nodes[v].adjlist[u];
        TemporalEdge f = tg.nodes[v].adjlist[w];

        if (e.v_id == f.v_id)
            continue;

        if (use_delta && ((e.t > f.t && e.t - f.t > delta) || (f.t > e.t && f.t - e.t > delta)))
            continue;

        Label ul = tg.nodes[e.v_id].getLabel(e.t);
        Label vl1 = tg.nodes[e.u_id].getLabel(e.t + 1);
        Label vl2 = tg.nodes[f.u_id].getLabel(f.t);
        Label wl = tg.nodes[f.v_id].getLabel(f.t + 1);

        Label o = e.t < f.t;

        int index = o + 2 * ul + 4 * vl1 + 8 * vl2 + 16 * wl;
        result[index]++;
        ++i;
    }
    return result;
}

bool isASafeMotif(TGNodes nodes, NodeId u, NodeId v)
{
    TGNode node_v = nodes[v];
    TGNode node_u = nodes[node_v.adjlist[u].v_id];

    auto nodeTimed_labelsBack = node_v.timed_labels.back();
    auto last_timed_label_v = node_u.timed_labels.back();

    if (nodeTimed_labelsBack.second == 1 && last_timed_label_v.second == 1)
    {
        Time a = nodeTimed_labelsBack.first;
        Time t = node_v.adjlist[u].t;
        Time b = last_timed_label_v.first;
        if ((a < b && b < t) || (b < a && a < t) || (b < t && t < a))
            return true;
    }
    return false;
}

std::vector<uint> countWedges(TemporalGraph &tg, uint delta)
{
    std::vector<uint> result(32, 0);

    for (int i = 0; i < tg.num_nodes; ++i)
    {
        NodeId v = i;
        for (int u = 0; u < tg.nodes[v].adjlist.size(); ++u)
        {
            // if (isASafeMotif(tg.nodes, u, v))
            //     continue;
            for (int w = 0; w < tg.nodes[v].adjlist.size(); ++w)
            {
                // if (isASafeMotif(tg.nodes, w, v))
                //     continue;

                TemporalEdge e = tg.nodes[v].adjlist[u];
                TemporalEdge f = tg.nodes[v].adjlist[w];

                if (e.v_id == f.v_id)
                    continue;
                if ((e.t > f.t && e.t - f.t > delta) || (f.t > e.t && f.t - e.t > delta))
                    continue;

                Label ul = tg.nodes[e.v_id].getLabel(e.t);
                Label vl1 = tg.nodes[e.u_id].getLabel(e.t + 1);
                Label vl2 = tg.nodes[f.u_id].getLabel(f.t);
                Label wl = tg.nodes[f.v_id].getLabel(f.t + 1);

                Label o = e.t < f.t;

                int index = o + 2 * ul + 4 * vl1 + 8 * vl2 + 16 * wl;
                result[index]++;
            }
        }
    }
    return result;
}

void computeTemporalGraphletKernelApproxGramMatrix(TemporalGraphStreams &data, std::string const &datasetname,
                                                   uint num_samples, bool use_delta)
{

    using S = Eigen::Triplet<unsigned int>;
    Timer timer;

    vector<TemporalGraph> data_tg;
    for (auto &tgs : data)
    {
        auto tg = tgs.toTemporalGraph();
        data_tg.push_back(tg);
    }

    long num_graphs = (long)data.size();
    vector<S> nonzero_components;

    // compute motifs
    vector<uint> deltas = {10, 100, 1000, 10000};

    for (int d = 0; d < deltas.size(); ++d)
    {

        timer.startTimer();

        auto delta = deltas[d];

        int num_graph = 0;
        int index = 0;
        for (auto &tgs : data_tg)
        {
            vector<uint> counts = estimateWedges(tgs, delta, num_samples, use_delta);

            index = 0;
            for (auto count : counts)
            {
                nonzero_components.emplace_back(num_graph, index++, count);
            }

            num_graph++;
        }

        // Compute Gram matrix.
        GramMatrix feature_vectors(num_graphs, index);
        feature_vectors.setFromTriplets(nonzero_components.begin(), nonzero_components.end());

        GramMatrix gram_matrix(num_graphs, num_graphs);
        gram_matrix = feature_vectors * feature_vectors.transpose();

        Eigen::MatrixXd dense_gram_matrix(gram_matrix);

        Eigen::MatrixXd nm;
        CommonFunctions::normalizeGramMatrix(dense_gram_matrix, nm);

        auto full_name = datasetname + "__tkga" + to_string(num_samples) + "_" + to_string(d) + ".gram";
        CommonFunctions::writeGramMatrixToFile(nm, data, full_name);
        timer.stopTimer(full_name + ".time");
    }
}

void computeTemporalGraphletKernelWedgesGramMatrix(TemporalGraphStreams &data, std::string const &datasetname)
{

    using S = Eigen::Triplet<unsigned int>;
    Timer timer;
    int edgesInit = 0;

    vector<TemporalGraph> data_tg, data_tg_copy;
    for (auto &tgs : data)
    {
        auto tg = tgs.toTemporalGraph();
        data_tg.push_back(tg);
        data_tg_copy.push_back(tg);
        edgesInit += tg.num_edges;
    }
    cout << "qtd edges: " << edgesInit << endl;
    long num_graphs = (long)data.size();
    vector<S> nonzero_components;
    TemporalGraphs setg;
    // compute motifs
    vector<uint> deltas = {10, 100, 1000, 10000};

    for (int d = 0; d < deltas.size(); ++d)
    {
        timer.startTimer();
        auto delta = deltas[d];
        int num_graph = 0;
        int index = 0;
        int edgesCount = edgesInit;

        for (auto &tgs : data_tg)
        {
            edgesCount -= computeTemporalReduction(tgs, delta);
            if (d == 0)
                setg.push_back(tgs);
            vector<uint> counts = countWedges(tgs, delta);
            index = 0;
            for (auto count : counts)
            {
                nonzero_components.emplace_back(num_graph, index++, count);
            }
            num_graph++;
        }
        data_tg = data_tg_copy;
        // Compute Gram matrix.
        GramMatrix feature_vectors(num_graphs, index);
        feature_vectors.setFromTriplets(nonzero_components.begin(), nonzero_components.end());

        GramMatrix gram_matrix(num_graphs, num_graphs);
        gram_matrix = feature_vectors * feature_vectors.transpose();

        Eigen::MatrixXd dense_gram_matrix(gram_matrix);

        Eigen::MatrixXd nm;
        CommonFunctions::normalizeGramMatrix(dense_gram_matrix, nm);

        auto full_name = datasetname + "__tkgw_" + to_string(d) + ".gram";
        CommonFunctions::writeGramMatrixToFile(nm, data, full_name);
        timer.stopTimer(full_name + ".time");
        cout << "Tem " << edgesCount << " edges" << endl;
    }

    DataManager dsl;
    dsl.saveGraphs(setg, datasetname + "LG");
}