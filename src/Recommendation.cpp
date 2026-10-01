#include "Recommendation.h"

bool RecommendationGraph::isSimilar(const Song& a, const Song& b) {
    if (a.title == b.title) return false; // same song
    if (a.artist == b.artist) return true; // same artist
    if (a.genre == b.genre) return true;   // same genre
    return false;
}

void RecommendationGraph::buildGraph(const vector<Song>& allSongs) {
    // Clear existing
    adjList.clear();

    // Create edges for similar songs
    for (size_t i = 0; i < allSongs.size(); i++) {
        for (size_t j = i + 1; j < allSongs.size(); j++) {
            if (isSimilar(allSongs[i], allSongs[j])) {
                adjList[allSongs[i].title].push_back(allSongs[j].title);
                adjList[allSongs[j].title].push_back(allSongs[i].title);
            }
        }
    }
}

void RecommendationGraph::recommendSimilar(string seedTitle) {
    if (adjList.find(seedTitle) == adjList.end()) {
        cout << "No recommendations found for '" << seedTitle << "'.\n";
        return;
    }

    cout << "\n=== Recommended Based on '" << seedTitle << "' ===\n";
    
    // BFS traversal
    unordered_set<string> visited;
    queue<string> q;
    
    q.push(seedTitle);
    visited.insert(seedTitle);
    
    int count = 0;
    while (!q.empty() && count < 5) { // Recommend up to 5 songs
        string current = q.front();
        q.pop();
        
        if (current != seedTitle) {
            cout << " - " << current << "\n";
            count++;
        }
        
        for (const string& neighbor : adjList[current]) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }
    cout << "==========================================\n";
}
