#include "MusicDatabase.h"

MusicDatabase::MusicDatabase() : bstRoot(nullptr) {}

string MusicDatabase::normalizeTitle(const string& title) {
    string normalized = title;
    transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    });
    return normalized;
}

MusicDatabase::~MusicDatabase() {
    destroyBST(bstRoot);
}

void MusicDatabase::destroyBST(BSTNode* root) {
    if (root) {
        destroyBST(root->left);
        destroyBST(root->right);
        delete root;
    }
}

void MusicDatabase::addSongToDatabase(string title, string artist, double duration, string genre, int rating) {
    Song s(title, artist, duration, genre, rating);
    addSongToDatabase(s);
}

void MusicDatabase::addSongToDatabase(const Song& s) {
    string key = normalizeTitle(s.title);
    if (database.find(key) == database.end()) {
        database[key] = s;
        bstRoot = insertBST(bstRoot, s);
        updateSortedList();
    }
}

BSTNode* MusicDatabase::insertBST(BSTNode* root, Song s) {
    if (root == nullptr) {
        return new BSTNode(s);
    }
    if (s.title < root->song.title) {
        root->left = insertBST(root->left, s);
    } else if (s.title > root->song.title) {
        root->right = insertBST(root->right, s);
    }
    return root;
}

bool MusicDatabase::getSong(string title, Song& outSong) const {
    auto it = database.find(normalizeTitle(title));
    if (it != database.end()) {
        outSong = it->second;
        return true;
    }
    return false;
}

bool MusicDatabase::songExists(string title) const {
    return database.find(normalizeTitle(title)) != database.end();
}

void MusicDatabase::inOrderTraversal(BSTNode* root) const {
    if (root != nullptr) {
        inOrderTraversal(root->left);
        cout << " - " << root->song.title << " by " << root->song.artist 
             << " [" << root->song.genre << "] (" << root->song.duration << " mins)\n";
        inOrderTraversal(root->right);
    }
}

void MusicDatabase::displayLibraryAlphabetical() const {
    cout << "\n=== Global Music Library (A-Z) ===\n";
    if (bstRoot == nullptr) {
        cout << "Database is empty.\n";
    } else {
        inOrderTraversal(bstRoot);
    }
    cout << "==================================\n";
}

void MusicDatabase::updateSortedList() {
    sortedSongList.clear();
    for (auto const& pair : database) {
        sortedSongList.push_back(pair.second);
    }
    // Sort vector alphabetically for binary search
    sort(sortedSongList.begin(), sortedSongList.end(), [](const Song& a, const Song& b) {
        return a.title < b.title;
    });
}

// Case insensitive substring match helper
bool containsIgnoreCase(const string& str, const string& query) {
    auto it = search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return tolower(ch1) == tolower(ch2); }
    );
    return (it != str.end());
}

bool MusicDatabase::binarySearchExactTitle(string title, Song& outSong) const {
    if (sortedSongList.empty()) return false;
    int left = 0;
    int right = (int)sortedSongList.size() - 1;
    
    // Case insensitive comparison for search
    string searchTitle = title;
    transform(searchTitle.begin(), searchTitle.end(), searchTitle.begin(), ::tolower);

    while (left <= right) {
        int mid = left + (right - left) / 2;
        string midTitle = sortedSongList[mid].title;
        transform(midTitle.begin(), midTitle.end(), midTitle.begin(), ::tolower);

        if (midTitle == searchTitle) {
            outSong = sortedSongList[mid];
            return true;
        }
        if (midTitle < searchTitle) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return false;
}

void MusicDatabase::autocompleteSearch(string query) const {
    cout << "\nSearch Results for '" << query << "':\n";
    
    vector<Song> results;
    string lowerQuery = query;
    transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Use lower_bound (Binary Search) to find the first element that is >= query
    auto it = lower_bound(sortedSongList.begin(), sortedSongList.end(), lowerQuery, 
        [](const Song& s, const string& q) {
            string lowerTitle = s.title;
            transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
            return lowerTitle < q;
        }
    );

    // Collect prefix matches using iterator
    while (it != sortedSongList.end()) {
        string lowerTitle = it->title;
        transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        if (lowerTitle.find(lowerQuery) == 0) {
            results.push_back(*it);
        } else {
            break; // Since it's sorted, once it doesn't match prefix, we can stop
        }
        it++;
    }

    // Fallback for substring or artist matches (O(N)) if no prefix matched
    if (results.empty()) {
        for(const auto& s : sortedSongList) {
            if(containsIgnoreCase(s.title, query) || containsIgnoreCase(s.artist, query)) {
                results.push_back(s);
            }
        }
    }

    if (results.empty()) {
        cout << " No matches found.\n";
    } else {
        for (const auto& s : results) {
            cout << " -> " << s.title << " by " << s.artist << "\n";
        }
    }
}

vector<Song> MusicDatabase::getAllSongs() const {
    vector<Song> all;
    for (const auto& pair : database) {
        all.push_back(pair.second);
    }
    return all;
}

void MusicDatabase::seedDatabase() {
    addSongToDatabase("Bohemian Rhapsody", "Queen", 5.92, "Rock", 10);
    addSongToDatabase("Stairway to Heaven", "Led Zeppelin", 8.03, "Rock", 9);
    addSongToDatabase("Hotel California", "Eagles", 6.50, "Rock", 9);
    addSongToDatabase("Smells Like Teen Spirit", "Nirvana", 5.02, "Grunge", 8);
    addSongToDatabase("Billie Jean", "Michael Jackson", 4.90, "Pop", 10);
    addSongToDatabase("Shape of You", "Ed Sheeran", 3.90, "Pop", 7);
    addSongToDatabase("Blinding Lights", "The Weeknd", 3.33, "Synthpop", 8);
    addSongToDatabase("Rolling in the Deep", "Adele", 3.80, "Soul", 9);
    addSongToDatabase("Thriller", "Michael Jackson", 5.95, "Pop", 10);
    addSongToDatabase("Sweet Child O' Mine", "Guns N' Roses", 5.93, "Rock", 8);
    addSongToDatabase("Lose Yourself", "Eminem", 5.43, "Hip Hop", 9);
    addSongToDatabase("Sicko Mode", "Travis Scott", 5.20, "Hip Hop", 7);
    addSongToDatabase("Closer", "The Chainsmokers", 4.08, "EDM", 6);
    addSongToDatabase("Wake Me Up", "Avicii", 4.12, "EDM", 8);
}
