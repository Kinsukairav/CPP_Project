#ifndef MUSICDATABASE_H
#define MUSICDATABASE_H

#include "Models.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

// BST Node for displaying library alphabetically
struct BSTNode {
    Song song;
    BSTNode* left;
    BSTNode* right;

    BSTNode(Song s) : song(s), left(nullptr), right(nullptr) {}
};

class MusicDatabase {
private:
    // Hash map for O(1) lookups: Title -> Song object
    unordered_map<string, Song> database;
    
    // BST Root for O(N) alphabetical in-order traversal
    BSTNode* bstRoot;

    // Vector for O(log N) autocomplete search (sorted by title)
    vector<Song> sortedSongList;

    // Helper functions for BST
    BSTNode* insertBST(BSTNode* root, Song s);
    void inOrderTraversal(BSTNode* root) const;
    void destroyBST(BSTNode* root);

    // Keep sorted list updated
    void updateSortedList();
    static string normalizeTitle(const string& title);

public:
    MusicDatabase();
    ~MusicDatabase();

    // Add song to the database
    void addSongToDatabase(string title, string artist, double duration, string genre, int rating = 5);
    void addSongToDatabase(const Song& s);

    // Retrieve song by title O(1)
    bool getSong(string title, Song& outSong) const;

    // Check if song exists
    bool songExists(string title) const;

    // Display all songs alphabetically using BST
    void displayLibraryAlphabetical() const;

    // Search/Autocomplete using Binary Search
    void autocompleteSearch(string query) const;

    // Exact Title Search using Binary Search O(log N)
    bool binarySearchExactTitle(string title, Song& outSong) const;

    // Get all songs (for recommendations or other uses)
    vector<Song> getAllSongs() const;

    // Seed data
    void seedDatabase();
};

#endif // MUSICDATABASE_H
