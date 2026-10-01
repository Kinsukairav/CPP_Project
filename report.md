# 🎵 Music Playlist Manager — Project Report

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [DSA Concepts Used (Summary Table)](#2-dsa-concepts-used-summary-table)
3. [File-by-File Detailed Explanation](#3-file-by-file-detailed-explanation)
   - [Models.h](#31-modelsh)
   - [MusicDatabase.h / MusicDatabase.cpp](#32-musicdatabaseh--musicdatabasecpp)
   - [PlaylistManager.h / PlaylistManager.cpp](#33-playlistmanagerh--playlistmanagercpp)
   - [Recommendation.h / Recommendation.cpp](#34-recommendationh--recommendationcpp)
   - [main.cpp](#35-maincpp)
4. [How to Compile & Run](#4-how-to-compile--run)
5. [Sample Run Walkthrough](#5-sample-run-walkthrough)
6. [Viva Questions & Answers](#6-viva-questions--answers)

---

## 1. Project Overview

**Project Name:** Music Playlist Manager  
**Language:** C++ (C++17)  
**Purpose:** A console-based music playlist manager that demonstrates fundamental **Data Structures and Algorithms** through a real-world use case.

The application lets a user:
- Browse a pre-seeded music library
- Search for songs (autocomplete / binary search)
- Build and manage a playlist (add, remove, navigate forward/backward)
- Queue songs to play next
- View listening history
- Sort the playlist by duration
- Generate an optimal playlist for a target duration
- Get song recommendations based on similarity

### Architecture Diagram

```
┌─────────────┐        ┌───────────────────┐        ┌─────────────────────┐
│  main.cpp   │───────>│  MusicDatabase    │<───────│  RecommendationGraph│
│  (UI/Menu)  │        │  (Hash Map + BST  │        │  (Adjacency List +  │
│             │───┐    │   + Sorted Vector) │        │   BFS Traversal)    │
└─────────────┘   │    └───────────────────┘        └─────────────────────┘
                  │
                  │    ┌───────────────────┐
                  └───>│  PlaylistManager  │
                       │  (Doubly Linked   │
                       │   List + Stack +  │
                       │   Queue + Heap +  │
                       │   Merge Sort + DP)│
                       └───────────────────┘
```

---

## 2. DSA Concepts Used (Summary Table)

| # | DSA Concept | Where Used | Purpose | Time Complexity |
|---|-------------|-----------|---------|-----------------|
| 1 | **Hash Map** (`unordered_map`) | `MusicDatabase` — `database` | O(1) song lookup by title | O(1) avg |
| 2 | **Binary Search Tree (BST)** | `MusicDatabase` — `BSTNode` tree | Alphabetical in-order display of library | O(N) traversal, O(log N) insert (avg) |
| 3 | **Binary Search** | `MusicDatabase` — `binarySearchExactTitle`, `autocompleteSearch` (`lower_bound`) | Fast exact title search & prefix autocomplete on sorted vector | O(log N) |
| 4 | **Doubly Linked List (DLL)** | `PlaylistManager` — `head`/`tail`/`current` | Playlist storage; forward & backward traversal | O(1) insert/delete at ends, O(N) search |
| 5 | **Stack** | `PlaylistManager` — `playHistory` | LIFO listening history for "Play Previous" (undo) | O(1) push/pop |
| 6 | **Queue** | `PlaylistManager` — `upNextQueue` | FIFO "Up Next" queue for priority song playback | O(1) enqueue/dequeue |
| 7 | **Merge Sort** | `PlaylistManager` — `sortByDuration` | Sort playlist DLL by song duration (stable, O(N log N)) | O(N log N) |
| 8 | **Max Heap (Priority Queue)** | `PlaylistManager` — `showMostPlayed` | Display top-played songs in descending order of play count | O(N log N) build |
| 9 | **Dynamic Programming (0/1 Knapsack)** | `PlaylistManager` — `generateExactDurationPlaylist` | Select best-rated songs that fit a target duration budget | O(N x W) |
| 10 | **Graph (Adjacency List)** | `RecommendationGraph` — `adjList` | Model song-to-song similarity relationships | O(N^2) build |
| 11 | **BFS (Breadth-First Search)** | `RecommendationGraph` — `recommendSimilar` | Traverse graph to find related songs | O(V + E) |

---

## 3. File-by-File Detailed Explanation

---

### 3.1 `Models.h`

> **Role:** Defines the core data structures (`Song` and `Node`) used across the entire project.

#### `struct Song`

```cpp
struct Song {
    string title;
    string artist;
    double duration;   // in minutes
    string genre;
    int playCount;     // how many times played this session
    int rating;        // 1-10 rating, used by DP Knapsack

    Song();                                          // Default constructor
    Song(string t, string a, double d, string g, int r = 5); // Parameterized
    bool operator==(const Song& other) const;        // Compare by title+artist
};
```

| Member | Purpose |
|--------|---------|
| `title` | Song name — acts as the primary identifier |
| `artist` | Artist/band name |
| `duration` | Length in minutes (e.g., 5.92) |
| `genre` | Category like "Rock", "Pop", "EDM" — used by the recommendation graph |
| `playCount` | Incremented each time the song is played; used by the Max Heap |
| `rating` | Integer 1-10; acts as the "value" in the DP Knapsack algorithm |
| `operator==` | Lets us compare two `Song` objects — they're equal if title AND artist match |

**Default constructor** sets everything to zero/empty with `rating = 5`.  
**Parameterized constructor** uses an initializer list for efficient construction; `playCount` always starts at 0.

#### `struct Node`

```cpp
struct Node {
    Song song;
    Node* prev;
    Node* next;
    Node(Song s);
};
```

This is the building block for the **Doubly Linked List** used by `PlaylistManager`. Each node holds:
- A `Song` value
- A `prev` pointer (to the previous node)
- A `next` pointer (to the next node)

Both pointers start as `nullptr`.

**Viva Q:** *Why doubly linked list and not singly?*  
**A:** We need **bidirectional traversal** — "Play Next" goes forward, "Play Previous" goes backward. A singly linked list only supports forward traversal efficiently.

---

### 3.2 `MusicDatabase.h` / `MusicDatabase.cpp`

> **Role:** Manages the global music library. Provides O(1) lookups (Hash Map), O(N) alphabetical display (BST), and O(log N) search (Binary Search on sorted vector).

#### Data Members

```cpp
class MusicDatabase {
private:
    unordered_map<string, Song> database;     // Hash Map: normalized_title -> Song
    BSTNode* bstRoot;                         // BST root for alphabetical traversal
    vector<Song> sortedSongList;              // Sorted vector for binary search
};
```

Three data structures are maintained **in parallel** to demonstrate different DSA trade-offs:

| Data Structure | Access Pattern | Why This One? |
|---|---|---|
| `unordered_map` (Hash Map) | Exact title lookup -> O(1) | Fastest way to check "does this song exist?" |
| BST (`BSTNode*` tree) | In-order traversal -> alphabetical | Demonstrates tree traversal; prints A-Z naturally |
| `vector<Song>` (sorted) | Binary search -> O(log N) | Enables autocomplete with `lower_bound` |

#### `struct BSTNode`

```cpp
struct BSTNode {
    Song song;
    BSTNode* left;
    BSTNode* right;
    BSTNode(Song s) : song(s), left(nullptr), right(nullptr) {}
};
```

Standard BST node. Left child holds songs that come **before** alphabetically; right child holds songs that come **after**.

#### Key Functions

##### `normalizeTitle(const string& title)` — *static, private*
```cpp
string MusicDatabase::normalizeTitle(const string& title) {
    string normalized = title;
    transform(normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char ch) { return static_cast<char>(tolower(ch)); });
    return normalized;
}
```
- Converts a title to **all lowercase** for case-insensitive comparisons.
- Uses `std::transform` with a lambda that calls `tolower` on each character.
- **Why:** "Bohemian Rhapsody" and "bohemian rhapsody" should match the same song.

##### `addSongToDatabase(const Song& s)` — *public*
```cpp
void MusicDatabase::addSongToDatabase(const Song& s) {
    string key = normalizeTitle(s.title);
    if (database.find(key) == database.end()) {
        database[key] = s;                  // Insert into Hash Map
        bstRoot = insertBST(bstRoot, s);    // Insert into BST
        updateSortedList();                 // Rebuild sorted vector
    }
}
```
1. Normalizes the title to get the hash key.
2. Checks for duplicates using `find()` — O(1).
3. If new, inserts into all three data structures.

**Overloaded version:** `addSongToDatabase(string title, string artist, double duration, string genre, int rating)` — constructs a `Song` object first, then calls the above.

##### `insertBST(BSTNode* root, Song s)` — *private, recursive*
```cpp
BSTNode* MusicDatabase::insertBST(BSTNode* root, Song s) {
    if (root == nullptr) return new BSTNode(s);
    if (s.title < root->song.title)
        root->left = insertBST(root->left, s);
    else if (s.title > root->song.title)
        root->right = insertBST(root->right, s);
    return root;
}
```
- **Base case:** Empty subtree -> create a new node.
- **Recursive case:** Compare titles lexicographically; go left if smaller, right if larger.
- Returns the (potentially updated) root.
- **Time:** O(log N) average, O(N) worst case (skewed tree).

##### `getSong(string title, Song& outSong)` — *public*
```cpp
bool MusicDatabase::getSong(string title, Song& outSong) const {
    auto it = database.find(normalizeTitle(title));
    if (it != database.end()) {
        outSong = it->second;
        return true;
    }
    return false;
}
```
- Looks up the song in the **hash map** — O(1) average.
- Uses an **output parameter** (`outSong`) to return the found song.
- Returns `true`/`false` to indicate success/failure.

##### `inOrderTraversal(BSTNode* root)` — *private, recursive*
```cpp
void MusicDatabase::inOrderTraversal(BSTNode* root) const {
    if (root != nullptr) {
        inOrderTraversal(root->left);   // Visit left subtree
        cout << " - " << root->song.title << " by " << root->song.artist
             << " [" << root->song.genre << "] (" << root->song.duration << " mins)\n";
        inOrderTraversal(root->right);  // Visit right subtree
    }
}
```
- **In-order traversal** of BST: Left -> Root -> Right.
- Because the BST is ordered by title, this prints songs **alphabetically (A-Z)**.
- **Time:** O(N) — visits every node exactly once.

##### `displayLibraryAlphabetical()` — *public*
Calls `inOrderTraversal(bstRoot)` with header/footer formatting.

##### `updateSortedList()` — *private*
```cpp
void MusicDatabase::updateSortedList() {
    sortedSongList.clear();
    for (auto const& pair : database) {
        sortedSongList.push_back(pair.second);
    }
    sort(sortedSongList.begin(), sortedSongList.end(),
        [](const Song& a, const Song& b) { return a.title < b.title; });
}
```
- Rebuilds the sorted vector from the hash map.
- Sorts alphabetically by title using `std::sort` — O(N log N).
- Called every time a new song is added.

##### `binarySearchExactTitle(string title, Song& outSong)` — *public*
```cpp
bool MusicDatabase::binarySearchExactTitle(string title, Song& outSong) const {
    if (sortedSongList.empty()) return false;   // Guard against empty list
    int left = 0;
    int right = (int)sortedSongList.size() - 1;

    string searchTitle = title;
    transform(searchTitle.begin(), searchTitle.end(), searchTitle.begin(), ::tolower);

    while (left <= right) {
        int mid = left + (right - left) / 2;    // Avoids integer overflow
        string midTitle = sortedSongList[mid].title;
        transform(midTitle.begin(), midTitle.end(), midTitle.begin(), ::tolower);

        if (midTitle == searchTitle)     { outSong = sortedSongList[mid]; return true; }
        if (midTitle < searchTitle)      left = mid + 1;
        else                             right = mid - 1;
    }
    return false;
}
```
- Classic **binary search** on the sorted vector.
- `mid = left + (right - left) / 2` prevents integer overflow (vs. `(left+right)/2`).
- Case-insensitive comparison using `tolower`.
- **Time:** O(log N).

##### `autocompleteSearch(string query)` — *public*
```cpp
void MusicDatabase::autocompleteSearch(string query) const {
    // Step 1: Use lower_bound (binary search) to find first title >= query
    auto it = lower_bound(sortedSongList.begin(), sortedSongList.end(), lowerQuery,
        [](const Song& s, const string& q) {
            string lowerTitle = s.title;
            transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
            return lowerTitle < q;
        });

    // Step 2: Collect prefix matches
    while (it != sortedSongList.end()) {
        if (lowerTitle.find(lowerQuery) == 0) results.push_back(*it);
        else break;
        it++;
    }

    // Step 3: Fallback — linear scan for substring/artist matches
    if (results.empty()) { ... }
}
```
- Uses `std::lower_bound` (which internally uses **binary search**) to jump to the first matching position — O(log N).
- Then collects consecutive prefix matches — typically a small number.
- Falls back to O(N) substring search if no prefix match is found.

**Helper — `containsIgnoreCase`:** A free function that does case-insensitive substring matching using `std::search` with a custom comparator.

##### `destroyBST(BSTNode* root)` — *private, recursive (destructor helper)*
```cpp
void MusicDatabase::destroyBST(BSTNode* root) {
    if (root) {
        destroyBST(root->left);
        destroyBST(root->right);
        delete root;
    }
}
```
- **Post-order traversal** to delete all BST nodes and free memory.
- Called from the destructor `~MusicDatabase()`.

##### `seedDatabase()` — *public*
Pre-populates the database with 14 well-known songs across Rock, Pop, Hip Hop, EDM, Grunge, Synthpop, and Soul genres with realistic durations and ratings.

---

### 3.3 `PlaylistManager.h` / `PlaylistManager.cpp`

> **Role:** The heart of the application. Manages the user's playlist using a Doubly Linked List, with Stack, Queue, Merge Sort, Max Heap, and DP Knapsack features.

#### Data Members

```cpp
class PlaylistManager {
private:
    Node* head;             // First song in DLL
    Node* tail;             // Last song in DLL
    Node* current;          // Currently playing song
    stack<Song> playHistory; // LIFO history for "undo"
    queue<Song> upNextQueue; // FIFO "play next" queue
    MusicDatabase* db;      // Pointer to global database
};
```

#### Custom Comparator for Max Heap

```cpp
struct ComparePlayCount {
    bool operator()(const Song& a, const Song& b) const {
        return a.playCount < b.playCount;  // Reversed: makes it a MAX heap
    }
};
```
- `priority_queue` in C++ is a max-heap by default, but with a custom comparator.
- `a.playCount < b.playCount` means songs with **higher** playCount get higher priority.

#### Core DLL Functions

##### `addSong(string title)`
```cpp
void PlaylistManager::addSong(string title) {
    Song s;
    if (!db->getSong(title, s)) { cout << "Error..."; return; }

    Node* newNode = new Node(s);
    if (head == nullptr) {
        head = tail = current = newNode;  // First song
    } else {
        tail->next = newNode;             // Link at end
        newNode->prev = tail;
        tail = newNode;
    }
}
```
1. Looks up the song in the global database (O(1) via hash map).
2. Creates a new `Node`.
3. If the list is empty, the new node becomes `head`, `tail`, and `current`.
4. Otherwise, appends to the end of the DLL.
- **Time:** O(1) for insertion (appending at tail).

##### `removeSong(string title)`
```cpp
void PlaylistManager::removeSong(string title) {
    Node* temp = head;
    while (temp != nullptr) {
        if (temp->song.title == title) {
            // Handle 3 cases: head, tail, middle
            if (temp == head) { head = temp->next; if (head) head->prev = nullptr; }
            else if (temp == tail) { tail = temp->prev; if (tail) tail->next = nullptr; }
            else { temp->prev->next = temp->next; temp->next->prev = temp->prev; }

            if (current == temp) current = temp->next ? temp->next : temp->prev;
            delete temp;
            return;
        }
        temp = temp->next;
    }
}
```
- Linear search through DLL to find the song — O(N).
- Handles three deletion cases:
  - **Head node:** Update `head` pointer.
  - **Tail node:** Update `tail` pointer.
  - **Middle node:** Re-link prev and next around the deleted node.
- If the deleted node was the `current` song, moves `current` to next (or prev if at end).

##### `playNext()`
```cpp
void PlaylistManager::playNext() {
    // Priority 1: Check the "Up Next" queue
    if (!upNextQueue.empty()) {
        Song nextQueued = upNextQueue.front();
        upNextQueue.pop();
        // Push current to history, insert queued node after current, advance
        ...
        return;
    }
    // Priority 2: Normal forward traversal
    if (current->next != nullptr) {
        playHistory.push(current->song);  // Push to history stack
        current = current->next;
        current->song.playCount++;
    }
}
```
- **Queue takes priority:** If there are songs in the "Up Next" queue, dequeue (FIFO) and play that song instead.
- **Normal flow:** Push current song onto the **history stack**, then advance `current` to `current->next`.
- Increments `playCount` for the Max Heap feature.

##### `playPrevious()`
```cpp
void PlaylistManager::playPrevious() {
    if (playHistory.empty()) { cout << "No previous songs..."; return; }
    if (current != nullptr && current->prev != nullptr) {
        current = current->prev;
        playHistory.pop();
        current->song.playCount++;
    }
}
```
- Uses the **stack** (LIFO) to undo — pops the last song from history.
- Moves `current` backward in the DLL.

**Viva Q:** *Why use a stack for history?*  
**A:** "Play Previous" is an **undo** operation. Undo always reverses the **most recent** action — that's exactly LIFO (Last In, First Out), which a stack provides.

##### `displayPlaylist()`
Traverses the DLL from `head` to `tail`, printing each song. Marks the `current` song with ` -> `.

##### `displayCurrentSong()`
Simply prints the `current` node's song details.

#### Queue Operations

##### `queueUpNext(string title)`
```cpp
void PlaylistManager::queueUpNext(string title) {
    Song s;
    if (!db->getSong(title, s)) { ... return; }
    upNextQueue.push(s);  // FIFO enqueue
}
```
- Looks up the song and pushes it into the `queue<Song>`.
- FIFO: First song queued will be the first played when "Play Next" is pressed.

##### `viewQueue()`
Copies the queue to a temporary, then pops and prints each element (non-destructive display).

#### History Operations

##### `displayHistory()`
Copies the stack to a temporary, then pops and prints (most recent first).

##### `clearHistory()`
Pops all elements from the stack.

#### Merge Sort (Sort Playlist by Duration)

##### `split(Node* h)` — *private*
```cpp
Node* PlaylistManager::split(Node* h) {
    Node* fast = h;
    Node* slow = h;
    while (fast->next && fast->next->next) {
        fast = fast->next->next;  // Moves 2 steps
        slow = slow->next;        // Moves 1 step
    }
    Node* temp = slow->next;
    slow->next = nullptr;         // Cut the list in half
    return temp;                  // Return head of second half
}
```
- **Tortoise and Hare** algorithm to find the middle of the linked list.
- `fast` moves 2 nodes at a time; `slow` moves 1.
- When `fast` reaches the end, `slow` is at the midpoint.
- Splits the list into two halves by setting `slow->next = nullptr`.

##### `merge(Node* first, Node* second)` — *private, recursive*
```cpp
Node* PlaylistManager::merge(Node* first, Node* second) {
    if (!first) return second;
    if (!second) return first;

    if (first->song.duration < second->song.duration) {
        first->next = merge(first->next, second);
        first->next->prev = first;
        first->prev = nullptr;
        return first;
    } else {
        second->next = merge(first, second->next);
        second->next->prev = second;
        second->prev = nullptr;
        return second;
    }
}
```
- Merges two sorted DLLs into one sorted DLL.
- Compares by `duration` — shorter songs come first.
- Recursively picks the smaller-duration node and links it.
- Properly maintains `prev` pointers for the doubly-linked structure.

##### `mergeSortRecursive(Node* node)` — *private*
```cpp
Node* PlaylistManager::mergeSortRecursive(Node* node) {
    if (!node || !node->next) return node;  // Base case: 0 or 1 element
    Node* second = split(node);
    node = mergeSortRecursive(node);
    second = mergeSortRecursive(second);
    return merge(node, second);
}
```
- **Divide and Conquer:**
  1. **Divide:** Split the list into two halves.
  2. **Conquer:** Recursively sort each half.
  3. **Combine:** Merge the two sorted halves.
- **Time:** O(N log N). **Space:** O(log N) stack frames (recursive calls).

##### `sortByDuration()` — *public*
```cpp
void PlaylistManager::sortByDuration() {
    if (!head || !head->next) return;
    head = mergeSortRecursive(head);
    // Fix tail pointer
    Node* temp = head;
    while (temp->next) temp = temp->next;
    tail = temp;
    current = head;
}
```
- Calls merge sort on the DLL.
- After sorting, the `tail` pointer is stale, so we walk to the end to fix it.
- Resets `current` to `head`.

**Viva Q:** *Why Merge Sort and not Quick Sort for a linked list?*  
**A:** Merge Sort is preferred for linked lists because:
1. It doesn't require random access (arrays are better for Quick Sort's partitioning).
2. The split operation is O(N) using the slow/fast pointer technique.
3. No extra space needed for merging (we re-link existing nodes).

#### Max Heap — Most Played Songs

##### `showMostPlayed()` — *public*
```cpp
void PlaylistManager::showMostPlayed() {
    priority_queue<Song, vector<Song>, ComparePlayCount> maxHeap;

    Node* temp = head;
    while (temp) {
        if (temp->song.playCount > 0)
            maxHeap.push(temp->song);
        temp = temp->next;
    }

    int count = 1;
    while (!maxHeap.empty() && count <= 5) {
        Song s = maxHeap.top();
        maxHeap.pop();
        cout << count++ << ". " << s.title << " (Played " << s.playCount << " times)\n";
    }
}
```
- Creates a **max-heap** (`priority_queue`) ordered by `playCount`.
- Inserts all played songs from the DLL into the heap.
- Pops the top 5 to show the most-played songs.
- **Time:** O(N log N) to build the heap, O(K log N) to extract K elements.

**Viva Q:** *Why use a heap instead of just sorting?*  
**A:** When we only need the top K elements (here K=5), a heap is conceptually the right tool. In practice, for small N both work, but a heap demonstrates the priority queue data structure which is the core concept here.

#### Dynamic Programming — 0/1 Knapsack

##### `generateExactDurationPlaylist(double targetMinutes)` — *public*
```cpp
void PlaylistManager::generateExactDurationPlaylist(double targetMinutes) {
    // 1. Collect songs from DLL into a vector
    vector<Song> songs;
    Node* temp = head;
    while (temp) { songs.push_back(temp->song); temp = temp->next; }

    int n = songs.size();
    int target = (int)(targetMinutes * 100);  // Convert to integers (centiseconds)

    // 2. Build DP table
    //    dp[i][w] = max total rating using first i songs within w capacity
    vector<vector<int>> dp(n + 1, vector<int>(target + 1, 0));

    for (int i = 1; i <= n; i++) {
        int weight = (int)(songs[i-1].duration * 100);  // Song duration as "weight"
        int value = songs[i-1].rating;                   // Song rating as "value"

        for (int w = 1; w <= target; w++) {
            if (weight <= w)
                dp[i][w] = max(dp[i-1][w], dp[i-1][w - weight] + value);
            else
                dp[i][w] = dp[i-1][w];
        }
    }

    // 3. Backtrack to find selected songs
    vector<Song> selected;
    int res = dp[n][target];
    int w = target;
    for (int i = n; i > 0 && res > 0; i--) {
        if (res == dp[i-1][w]) continue;   // Song i was NOT included
        else {
            selected.push_back(songs[i-1]); // Song i WAS included
            res -= songs[i-1].rating;
            w -= (int)(songs[i-1].duration * 100);
        }
    }
}
```

**Analogy to classic Knapsack:**

| Knapsack Term | Music App Term |
|---------------|----------------|
| Item | Song |
| Weight | Duration (in centiseconds) |
| Value | Rating (1-10) |
| Capacity | Target playlist duration |
| Goal | Maximize total rating within the time budget |

**How the DP table works:**
- `dp[i][w]` = the **maximum rating** achievable using songs 1..i with a time budget of w centiseconds.
- For each song, we choose: **include it** (if it fits) or **skip it**.
- `dp[i][w] = max(dp[i-1][w], dp[i-1][w - weight] + value)`

**Backtracking:** Starting from `dp[n][target]`, we trace which songs were included by checking if `dp[i][w] != dp[i-1][w]`.

**Why multiply by 100?** The DP table needs integer indices. Durations like 5.92 become 592 as integers.

**Time:** O(N x W) where N = number of songs, W = target in centiseconds.  
**Space:** O(N x W) for the 2D DP table.

---

### 3.4 `Recommendation.h` / `Recommendation.cpp`

> **Role:** Builds a similarity graph between songs and uses BFS to find recommendations.

#### Data Members

```cpp
class RecommendationGraph {
private:
    unordered_map<string, vector<string>> adjList;  // Adjacency list
};
```

- The graph is stored as an **adjacency list**: each song title maps to a list of similar song titles.
- This is an **undirected graph** — if A is similar to B, then B is also similar to A.

#### `isSimilar(const Song& a, const Song& b)` — *private*
```cpp
bool RecommendationGraph::isSimilar(const Song& a, const Song& b) {
    if (a.title == b.title) return false;  // Don't connect a song to itself
    if (a.artist == b.artist) return true;  // Same artist -> similar
    if (a.genre == b.genre) return true;    // Same genre -> similar
    return false;
}
```
Two songs are considered **similar** if they share the same artist OR the same genre.

#### `buildGraph(const vector<Song>& allSongs)` — *public*
```cpp
void RecommendationGraph::buildGraph(const vector<Song>& allSongs) {
    adjList.clear();
    for (size_t i = 0; i < allSongs.size(); i++) {
        for (size_t j = i + 1; j < allSongs.size(); j++) {
            if (isSimilar(allSongs[i], allSongs[j])) {
                adjList[allSongs[i].title].push_back(allSongs[j].title);
                adjList[allSongs[j].title].push_back(allSongs[i].title);
            }
        }
    }
}
```
- Compares every pair of songs — O(N^2).
- If similar, adds an **undirected edge** (both directions).
- Example: "Bohemian Rhapsody" (Rock) connects to "Stairway to Heaven" (Rock), "Hotel California" (Rock), etc.

#### `recommendSimilar(string seedTitle)` — *public*
```cpp
void RecommendationGraph::recommendSimilar(string seedTitle) {
    if (adjList.find(seedTitle) == adjList.end()) { ... return; }

    unordered_set<string> visited;
    queue<string> q;

    q.push(seedTitle);
    visited.insert(seedTitle);

    int count = 0;
    while (!q.empty() && count < 5) {
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
}
```
- Classic **BFS (Breadth-First Search)** starting from the seed song.
- Uses a `queue` for BFS and an `unordered_set` for the visited set.
- Skips the seed song itself; prints up to 5 recommended songs.
- BFS explores songs that are **directly similar first** (1-hop neighbors), then friends-of-friends (2-hop), etc.

**Time:** O(V + E) where V = songs, E = similarity edges.

**Viva Q:** *Why BFS and not DFS?*  
**A:** BFS explores by **distance levels** — it finds the most directly related songs first (same artist/genre), before going deeper to indirect connections. DFS could jump to an indirectly related song before exhausting direct matches.

---

### 3.5 `main.cpp`

> **Role:** The entry point. Provides a menu-driven console interface connecting all components.

#### `displayMenu()`
Prints the 17-option menu (options 0-16).

#### `main()`
```cpp
int main() {
    MusicDatabase db;         // Create database
    db.seedDatabase();        // Populate with 14 songs

    PlaylistManager pm(&db);  // Create playlist manager (linked to DB)

    RecommendationGraph rg;
    rg.buildGraph(db.getAllSongs());  // Build similarity graph

    int choice = -1;
    while (choice != 0) {
        displayMenu();
        if (!(cin >> choice)) {  // Input validation
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        cin.ignore();  // Clear the newline after the integer

        switch (choice) { ... }  // Route to appropriate function
    }
    return 0;
}
```

**Key design points:**
- `PlaylistManager` receives a **pointer** to `MusicDatabase` — this avoids copying the entire database and lets the playlist look up songs.
- `cin >> choice` reads an integer; `cin.ignore()` clears the trailing newline so that subsequent `getline()` calls work correctly.
- Input validation: if the user types a non-integer, `cin.clear()` resets the error flag and `cin.ignore(10000, '\n')` discards the bad input.

---

## 4. How to Compile & Run

### Using g++ (MinGW / GCC)
```bash
g++ -std=c++17 -o MusicApp.exe src/main.cpp src/MusicDatabase.cpp src/PlaylistManager.cpp src/Recommendation.cpp
.\MusicApp.exe
```

### Using MSVC (Visual Studio Developer Command Prompt)
```bash
cl /EHsc /std:c++17 src/main.cpp src/MusicDatabase.cpp src/PlaylistManager.cpp src/Recommendation.cpp /Fe:MusicApp.exe
.\MusicApp.exe
```

---

## 5. Sample Run Walkthrough

```
========== Music Playlist Manager ==========
1.  View Global Library (A-Z)             <-- BST in-order traversal
2.  Search / Autocomplete Song            <-- Binary Search (lower_bound)
3.  Add Song to Playlist                  <-- DLL append
4.  Remove Song from Playlist             <-- DLL delete
5.  Queue Song (Up Next)                  <-- Queue enqueue (FIFO)
6.  View Up Next Queue                    <-- Queue peek
7.  Play Next                             <-- DLL forward + Stack push + Queue dequeue
8.  Play Previous (Undo)                  <-- DLL backward + Stack pop
9.  Show Now Playing                      <-- DLL current pointer
10. Show Current Playlist                 <-- DLL full traversal
11. Sort Playlist by Duration             <-- Merge Sort on DLL
12. View Listening History                <-- Stack display (LIFO)
13. Clear Listening History               <-- Stack clear
14. Show Most Played Songs               <-- Max Heap (priority_queue)
15. Generate Target Duration Playlist     <-- DP 0/1 Knapsack
16. Recommend Similar Songs              <-- Graph + BFS
0.  Exit
============================================
```

**Example session:**
1. **Option 1** -> Displays all 14 songs alphabetically (BST in-order).
2. **Option 2**, type "Boh" -> Autocomplete finds "Bohemian Rhapsody" (binary search).
3. **Option 3**, type "Bohemian Rhapsody" -> Added to playlist (DLL append).
4. **Option 3**, add 3 more songs.
5. **Option 7** -> Plays first song, increments `playCount`, pushes nothing (first play).
6. **Option 7** -> Plays next song, pushes previous to history stack.
7. **Option 8** -> Goes back to previous song (stack pop + DLL backward).
8. **Option 11** -> Sorts playlist by duration (merge sort).
9. **Option 14** -> Shows most played songs using max heap.
10. **Option 15**, enter "15.0" -> DP Knapsack selects highest-rated songs fitting in 15 mins.
11. **Option 16**, enter "Bohemian Rhapsody" -> BFS recommends other Rock songs.

---

## 6. Viva Questions & Answers

### General Project Questions

**Q1: What is the overall purpose of this project?**  
A: It is a console-based Music Playlist Manager built in C++ that demonstrates 11 core DSA concepts (Hash Map, BST, Binary Search, DLL, Stack, Queue, Merge Sort, Max Heap, DP Knapsack, Graph, BFS) through a real-world music application.

**Q2: Why did you choose a music player as the application?**  
A: A music player naturally requires many different data structures — playlists need linked lists, history needs stacks, "up next" needs queues, search needs hash maps and binary search, etc. It's a single coherent application that justifies using multiple DSA concepts without feeling forced.

**Q3: What is the time complexity of each major operation?**  
A: See the table in Section 2. Key ones: Hash map lookup O(1), BST traversal O(N), Binary search O(log N), DLL insert/delete O(1)/O(N), Merge sort O(N log N), Knapsack O(N*W), BFS O(V+E).

---

### Data Structure-Specific Questions

**Q4: Why use three data structures (hash map, BST, sorted vector) for the same data?**  
A: Each serves a different access pattern:
- Hash map -> O(1) exact lookup ("does this song exist?")
- BST -> Natural A-Z display via in-order traversal
- Sorted vector -> Binary search for autocomplete (`lower_bound`)

This demonstrates the trade-off: no single data structure is optimal for all operations.

**Q5: What is the difference between a Stack and a Queue?**  
A: 
- **Stack (LIFO):** Last In, First Out. Like a stack of plates — you remove the most recently added item. Used for: undo/history.
- **Queue (FIFO):** First In, First Out. Like a line at a shop — the first person in line is served first. Used for: "Up Next" queue.

**Q6: Why is a Doubly Linked List used instead of an array/vector for the playlist?**  
A: 
1. **Bidirectional traversal:** "Play Next" and "Play Previous" need forward/backward movement.
2. **O(1) insertion/deletion** at known positions (no shifting elements like in arrays).
3. **Dynamic size** — no need to pre-allocate or resize.

**Q7: Explain how Merge Sort works on a linked list.**  
A:
1. **Split:** Use slow/fast pointers to find the middle. Cut the list in two.
2. **Recurse:** Sort each half independently.
3. **Merge:** Compare elements from both sorted halves, linking nodes in order.
The key advantage: no extra array needed — we just re-link existing nodes.

**Q8: What is the 0/1 Knapsack problem and how does it apply here?**  
A: Given items with weights and values, and a weight capacity, select items to maximize total value without exceeding capacity. Here:
- Items = songs, Weight = duration, Value = rating, Capacity = target time.
- "0/1" means each song is either included or not (no repeats).
- We use a 2D DP table where `dp[i][w]` stores the best rating using first i songs within w time units.

**Q9: Why multiply duration by 100 in the Knapsack?**  
A: DP table indices must be integers. Durations are decimals (e.g., 5.92 minutes), so multiplying by 100 converts them to integer centiseconds (592) for use as array indices.

**Q10: How does BFS find recommendations?**  
A: Starting from a seed song, BFS explores all directly similar songs first (same artist/genre), then songs similar to those, etc. This ensures the most closely related songs appear first.

---

### Code-Specific Questions

**Q11: What does `using namespace std;` do?**  
A: It allows using standard library names (like `cout`, `string`, `vector`) without the `std::` prefix. Example: `cout` instead of `std::cout`.

**Q12: What is a header guard (`#ifndef ... #define ... #endif`)?**  
A: It prevents a header file from being included multiple times in the same translation unit, which would cause "redefinition" compilation errors. The preprocessor checks if the macro is already defined; if so, it skips the file contents.

**Q13: What is `nullptr` and why use it over `NULL`?**  
A: `nullptr` is a C++11 keyword that represents a null pointer. Unlike `NULL` (which is just `0`), `nullptr` is type-safe — it can only be assigned to pointer types, preventing accidental misuse.

**Q14: What is an initializer list in a constructor?**  
A: The `: member(value)` syntax after the constructor signature. Example: `Node(Song s) : song(s), prev(nullptr), next(nullptr) {}`. It initializes members directly rather than first default-constructing them and then assigning — more efficient.

**Q15: What does `const` mean in `void displayPlaylist() const;`?**  
A: The `const` after the function signature means this function promises not to modify any member variables of the object. It can be called on `const` objects.

**Q16: What is a lambda function and where is it used?**  
A: A lambda is an anonymous function defined inline. Example: `[](const Song& a, const Song& b) { return a.title < b.title; }` is used as a comparator for `sort()`. The `[]` is the capture list, `()` has parameters, `{}` has the body.

**Q17: What is `auto` keyword?**  
A: `auto` lets the compiler deduce the type automatically. Example: `auto it = database.find(key);` — the compiler figures out `it` is an `unordered_map<string, Song>::iterator`.

**Q18: Explain `cin.clear()` and `cin.ignore()`.**  
A: 
- `cin.clear()` — resets error flags on the input stream after a failed read (e.g., typing "abc" when an integer is expected).
- `cin.ignore(10000, '\n')` — discards up to 10000 characters or until a newline is found, clearing the bad input from the buffer.

---

### Complexity & Trade-off Questions

**Q19: What is the space complexity of the Knapsack DP?**  
A: O(N x W) for the 2D table, where N = number of songs and W = target duration x 100. For 14 songs and 15 minutes -> 14 x 1500 = 21,000 cells. This is manageable but could be large for big inputs.

**Q20: Can the BST become unbalanced? What would happen?**  
A: Yes. If songs are inserted in alphabetical order, the BST degenerates into a linked list, making insert O(N) instead of O(log N). A self-balancing BST (AVL or Red-Black tree) would fix this. For this project's small dataset (14 songs), it's not a practical issue.

**Q21: What is the difference between `unordered_map` and `map`?**  
A: 
- `unordered_map` uses a **hash table** -> O(1) average lookup, O(N) worst case. Unordered.
- `map` uses a **Red-Black BST** -> O(log N) guaranteed lookup. Sorted by key.

We chose `unordered_map` because we don't need sorted keys and want the fastest lookups.

---

*End of Report*
