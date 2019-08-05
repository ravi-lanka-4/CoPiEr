#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <vector>
#include <memory>

class Graph
{
public:
    Graph();

    Graph(const int _num_nodes, const int _num_edges, const int* edges_from, const int* edges_to);
    Graph(const int _num_nodes, const int _num_edges, const int* edges_from, \
            const int* edges_to, const int num_opt, const int* opt_nodes, const double prob);
    int num_nodes;
    int num_edges;

    std::vector< std::vector< int > > adj_list;
    std::vector< std::pair<int, int> > edge_list;
    std::vector< int > opt_list;

    void resetSupFlag();
    bool labeled; /* labeled or unlabeled data point */
    bool sup; /* always unsup if unlabeled else sup or unsup with a probability */
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
