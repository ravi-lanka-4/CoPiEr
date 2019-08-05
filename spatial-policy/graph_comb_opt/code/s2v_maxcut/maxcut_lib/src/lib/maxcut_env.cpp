#include "maxcut_env.h"
#include "graph.h"
#include <cassert>
#include <random>

MaxcutEnv::MaxcutEnv(double _norm) : IEnv(_norm)
{

}

void MaxcutEnv::s0(std::shared_ptr<Graph> _g)
{
    graph = _g;
    cut_set.clear();
    action_list.clear();
    cutWeight = 0;
    state_seq.clear();
    act_seq.clear();
    reward_seq.clear();
    sum_rewards.clear();
    graph->resetSupFlag();
}

double MaxcutEnv::step(int a)
{
    assert(graph);
    assert(cut_set.count(a) == 0);
    assert(a >= 0 && a < graph->num_nodes);

    state_seq.push_back(action_list);
    act_seq.push_back(a);

    cut_set.insert(a);
    action_list.push_back(a);

    double old_cutWeight = cutWeight;
    if (!graph->sup){
      /*
       * Cutset   Reward
       *   1      W_{ij}
       *   0     -W_{ij}
       */

      for (auto& neigh : graph->adj_list[a]){
          if (cut_set.count(neigh.first) == 0)
              cutWeight += neigh.second;
          else
              cutWeight -= neigh.second;
      }
    }
    else{
      /*
       * Cutset   OptSet    Reward
       *   1        1       W_{ij}
       *   0        1      -W_{ij}
       *   1        0         0
       *   0        0         0
       */

      for (auto& neigh : graph->adj_list[a]){
          std::set< std::pair<int, int> >::const_iterator opt_itr = (graph->opt_list).find( std::make_pair(a, neigh.first) );
          bool isOpt = (opt_itr != (graph->opt_list).end());
          if (isOpt){
            if (cut_set.count(neigh.first) == 0)
                cutWeight += neigh.second;
            else
                cutWeight -= neigh.second;
          }
      }
    }
      
    double r_t = getReward(old_cutWeight, a);
    reward_seq.push_back(r_t);
    sum_rewards.push_back(r_t);  

    return r_t;
}

int MaxcutEnv::randomAction()
{
    assert(graph);
    avail_list.clear();

    for (int i = 0; i < graph->num_nodes; ++i)
        if (cut_set.count(i) == 0)
            avail_list.push_back(i);
    
    assert(avail_list.size());
    int idx = rand() % avail_list.size();
    return avail_list[idx];
}

int MaxcutEnv::expertAction()
{
    assert(graph);
    assert((graph->opt_list).size());
    avail_list.clear();

    for (int i = 0; i < graph->num_nodes; ++i)
       if (cut_set.count(i) == 0)
          avail_list.push_back(i);

    /* find an edge between cut_set and avail_list belonging to opt_list */
    std::vector<int> cavail;
    std::set<int>::iterator it;
    unsigned int j=0;
    for (it = cut_set.begin(); it != cut_set.end(); it++){
       int v1 = *it;
       for (j = 0; j < avail_list.size(); j++){
          int v2 = avail_list[j];
          std::set< std::pair<int, int> >::const_iterator opt_itr = \
                (graph->opt_list).find( std::make_pair(v1, v2) );
          bool isOpt = (opt_itr != (graph->opt_list).end());
          if (!isOpt)
            cavail.push_back(v2);
       }
    }
   
    if(cavail.size()){
      int idx = rand() % cavail.size();
      return cavail[idx];
    }
    else{
      assert(avail_list.size());
      int idx = rand() % avail_list.size();
      return avail_list[idx];
    }
}

bool MaxcutEnv::isTerminal()
{
    assert(graph);
    return ((int)cut_set.size() + 1 >= graph->num_nodes);
}

double MaxcutEnv::getReward(double old_cutWeight, int a)
{
    return (cutWeight - old_cutWeight) / norm;
}
