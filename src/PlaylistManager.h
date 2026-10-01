#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include "Models.h"
#include "MusicDatabase.h"
#include <stack>
#include <queue>
#include <iostream>
#include <vector>

using namespace std;

// Custom comparator for Max-Heap
struct ComparePlayCount {
    bool operator()(const Song& a, const Song& b) const {
        return a.playCount < b.playCount; // Max heap based on playCount
    }
};

class PlaylistManager {
private:
    Node* head;
    Node* tail;
    Node* current; // The currently playing song

    // Stack for history (Undo/Previous)
    stack<Song> playHistory;

    // Queue for "Up Next" (Priority queue override essentially)
    queue<Song> upNextQueue;

    // Pointer to global database
    MusicDatabase* db;

    // Helper for Merge Sort
    Node* split(Node* head);
    Node* merge(Node* first, Node* second);
    Node* mergeSortRecursive(Node* head);

public:
    PlaylistManager(MusicDatabase* database);
    ~PlaylistManager();

    // Core DLL operations
    void addSong(string title);
    void removeSong(string title);
    void playNext();
    void playPrevious();
    void displayCurrentSong() const;
    void displayPlaylist() const;

    // Queue operations
    void queueUpNext(string title);
    void viewQueue() const;

    // History operations
    void displayHistory() const;
    void clearHistory();

    // Sorting (Merge Sort)
    void sortByDuration();

    // DP: 0-1 Knapsack to generate exact duration playlist
    void generateExactDurationPlaylist(double targetMinutes);
    
    // Track Most Played (Heap)
    void showMostPlayed();
};

#endif // PLAYLISTMANAGER_H
