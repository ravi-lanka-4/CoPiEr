#include "graph.h"
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <random>

Graph::Graph() : num_nodes(0), num_edges(0), labeled(0), sup(0)
{
    edge_list.clear();
    adj_list.clear();
    opt_list.clear();
}

Graph::Graph(const int _num_nodes, const int _num_edges, const int* edges_from, const int* edges_to)
        : num_nodes(_num_nodes), num_edges(_num_edges)
{
    labeled = false;
    edge_list.resize(num_edges);
    adj_list.resize(num_nodes);
    for (int i = 0; i < num_nodes; ++i)
        adj_list[i].clear();

    for (int i = 0; i < num_edges; ++i)
    {
        int x = edges_from[i], y = edges_to[i];
        adj_list[x].push_back(y);
        adj_list[y].push_back(x);
        edge_list[i] = std::make_pair(edges_from[i], edges_to[i]);
    }
}

/* Constructor for initializing the graph with optimal solution */
Graph::Graph(const int _num_nodes, const int _num_edges, const int* edges_from, const int* edges_to, 
              const int num_opt, const int* opt_nodes, const double prob)
        : num_nodes(_num_nodes), num_edges(_num_edges)
{
    assert((prob >= 0) and (prob <= 1));
    this->prob = prob;
    labeled = true;

    edge_list.resize(num_edges);
    adj_list.resize(num_nodes);

    /* Lists that save the optimal solution */
    opt_list.clear();
    opt_list.resize(num_nodes);

    for (int i = 0; i < num_nodes; ++i){
        adj_list[i].clear();
        opt_list[i] = 0;
    }

    for (int i = 0; i < num_opt; ++i){
        opt_list[opt_nodes[i]] = 1;
    }

    for (int i = 0; i < num_edges; ++i)
    {
        int x = edges_from[i], y = edges_to[i];
        adj_list[x].push_back(y);
        adj_list[y].push_back(x);
        edge_list[i] = std::make_pair(edges_from[i], edges_to[i]);
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
