#include "graph.h"
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <random>

Graph::Graph() : num_nodes(0), num_edges(0), labeled(0), sup(0)
{
    adj_list.clear();
    opt_list.clear();
}

Graph::Graph(const int _num_nodes, const int _num_edges, const int* edges_from, const int* edges_to, const double* weights)
        : num_nodes(_num_nodes), num_edges(_num_edges)
{
    labeled = false;
    adj_list.resize(num_nodes);
    for (int i = 0; i < num_nodes; ++i)
        adj_list[i].clear();
        
    for (int i = 0; i < num_edges; ++i)
    {
        int x = edges_from[i], y = edges_to[i];
        double w = weights[i];

        adj_list[x].push_back( std::make_pair(y, w) );
        adj_list[y].push_back( std::make_pair(x, w) );
    }
}

/* Constructor for initializing the graph with optimal solution */
Graph::Graph(const int _num_nodes, const int _num_edges, const int* edges_from, const int* edges_to, const double* weights,
              const int num_opt, const int* opt_nodes_v1, const int* opt_nodes_v2, const int num_opt_nodes, const double prob)
        : num_nodes(_num_nodes), num_edges(_num_edges)
{
    assert((prob >= 0) and (prob <= 1));
    this->prob = prob;
    labeled = true;

    /* Lists that save the optimal solution */
    opt_list.clear();
    for (int i = 0; i < num_opt_nodes; i++){
        int v1 = opt_nodes_v1[i];
        int v2 = opt_nodes_v2[i];
        opt_list.insert( std::make_pair(v1, v2) );
    }

    adj_list.resize(num_nodes);
    for (int i = 0; i < num_nodes; ++i){
        adj_list[i].clear();
    }

    for (int i = 0; i < num_edges; ++i)
    {
        int x = edges_from[i], y = edges_to[i];
        double w = weights[i];

        adj_list[x].push_back( std::make_pair(y, w) );
        adj_list[y].push_back( std::make_pair(x, w) );
    }
}

void Graph::resetSupFlag(){
    if (labeled){
      int rand_var = std::rand()/(RAND_MAX+1u);
      if (rand_var < prob){
        this->sup = true;
      }
      else
        this->sup = false;
    }
    else{
      /* Always unsupervised */
      this->sup = false;
    }
}

GSet::GSet()
{
    graph_pool.clear();
}

void GSet::InsertGraph(int gid, std::shared_ptr<Graph> graph)
{
    assert(graph_pool.count(gid) == 0);
    graph_pool[gid] = graph;
}

std::shared_ptr<Graph> GSet::Get(int gid)
{
    assert(graph_pool.count(gid));
    return graph_pool[gid];
}

std::shared_ptr<Graph> GSet::Sample()
{
    assert(graph_pool.size());
    int gid = rand() % graph_pool.size();
    assert(graph_pool[gid]);
    return graph_pool[gid];
}

GSet GSetTrain, GSetTest;
