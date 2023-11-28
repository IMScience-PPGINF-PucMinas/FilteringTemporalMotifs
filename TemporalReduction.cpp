#include "TemporalWedgeKernel.h"
#include <unordered_map>
#include "GraphKernelCommonFunctions.h"
#include <queue>
#include <random>

using namespace std;

int computeTemporalReduction(TemporalGraph &tg, int delta)
{
    int i = 0;
    for (auto &node : tg.nodes)
    {
        if (node.timed_labels.back().second == 1)
        {
            Time a = node.timed_labels.back().first;
            for (auto adjEdgeIt = node.adjlist.begin(); adjEdgeIt != node.adjlist.end(); ++adjEdgeIt)
            {
                const auto &adjEdge = *adjEdgeIt;
                auto last_timed_label_v = tg.nodes[adjEdge.v_id].timed_labels.back();
                Time t = adjEdge.t;
                Time b = last_timed_label_v.first;
                if (last_timed_label_v.second == 1)
                {
                    // if ((a < b && b < t) || (b < a && a < t) || (b < t && t < a))
                    if ((a < b && b < t) || (b < a && a < t))
                    {
                        node.adjlist.erase(adjEdgeIt);
                        --adjEdgeIt;
                        i += 1;
                    }
                    // else if ((((t < a && a < b) || (t < b && b < a)) && (((a - t) > delta) || ((b - t) > delta))))
                    // {
                    //     node.adjlist.erase(adjEdgeIt);
                    //     --adjEdgeIt;
                    //     i += 1;
                    // }
                }
            }
        }
    }
    // print graph
    // for (TGNode node : tg.nodes)
    // {
    //     cout << "node: " << node.id << endl;
    //     for (auto adjEdge : node.adjlist)
    //     {
    //         cout << "adjNode: " << adjEdge.v_id << " a: " << adjEdge.t << endl;
    //     }
    //     cout << endl;
    // }
    return i;
}