#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include "Models.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <iostream>

using namespace std;

class RecommendationGraph {
private:
    // Adjacency list: Song Title -> List of Similar Song Titles
    unordered_map<string, vector<string>> adjList;

    // Helper to calculate similarity weight
    bool isSimilar(const Song& a, const Song& b);

public:
    // Build graph from the global database
    void buildGraph(const vector<Song>& allSongs);

    // BFS to recommend songs based on a seed song
    void recommendSimilar(string seedTitle);
};

#endif // RECOMMENDATION_H
