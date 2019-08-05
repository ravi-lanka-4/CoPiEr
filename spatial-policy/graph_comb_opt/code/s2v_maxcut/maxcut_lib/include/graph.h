#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <set>
#include <vector>
#include <memory>

class Graph
{
public:
    Graph();

    Graph(const int _num_nodes, const int _num_edges, const int* edges_from, const int* edges_to, const double* weights);
    Graph(const int _num_nodes, const int _num_edges, const int* edges_from, const int* edges_to, const double* weights, \
            const int num_opt, const int* opt_nodes_v1, const int* opt_nodes_v2, const int num_opt_nodes, const double prob);
    int num_nodes;
    int num_edges;

    std::vector< std::vector< std::pair<int, double> > > adj_list;
    std::set< std::pair<int, int> > opt_list;

    void resetSupFlag();
    bool labeled;           /* labeled or unlabeled data point */
    bool sup;               /* always unsup if unlabeled else sup or unsup with a probability */
    double prob;
};

class GSet
{
public:
    GSet();

    void InsertGraph(int gid, std::shared_ptr<Graph> graph);
    std::shared_ptr<Graph> Sample();
    std::shared_ptr<Graph> Get(int gid);
    std::map<int, std::shared_ptr<Graph> > graph_pool;
};

extern GSet GSetTrain;
extern GSet GSetTest;

#endif
