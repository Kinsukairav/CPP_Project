#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <iostream>

using namespace std;

// Structure to represent a Song
struct Song {
    string title;
    string artist;
    double duration;
    string genre;
    int playCount;
    int rating; // 1-10 rating for DP Knapsack

    Song() : title(""), artist(""), duration(0.0), genre(""), playCount(0), rating(5) {}
    
    Song(string t, string a, double d, string g, int r = 5) 
        : title(t), artist(a), duration(d), genre(g), playCount(0), rating(r) {}

    // Equality operator for comparison
    bool operator==(const Song& other) const {
        return title == other.title && artist == other.artist;
    }
};

// Structure for a Node in the Doubly Linked List (Playlist)
struct Node {
    Song song;
    Node* prev;
    Node* next;
    
    Node(Song s) {
        song = s;
        prev = nullptr;
        next = nullptr;
    }
};

#endif // MODELS_H
