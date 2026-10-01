#include "PlaylistManager.h"

PlaylistManager::PlaylistManager(MusicDatabase* database) {
    head = nullptr;
    tail = nullptr;
    current = nullptr;
    db = database;
}

PlaylistManager::~PlaylistManager() {
    Node* temp = head;
    while (temp != nullptr) {
        Node* next = temp->next;
        delete temp;
        temp = next;
    }
}

void PlaylistManager::addSong(string title) {
    Song s;
    if (!db->getSong(title, s)) {
        cout << "Error: Song '" << title << "' not found in global database.\n";
        return;
    }

    Node* newNode = new Node(s);
    if (head == nullptr) {
        head = tail = current = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }

    cout << "Confirmation: '" << s.title << "' was added to the playlist.\n";
}

void PlaylistManager::removeSong(string title) {
    Node* temp = head;
    while (temp != nullptr) {
        if (temp->song.title == title) {
            if (temp == head) {
                head = temp->next;
                if (head) head->prev = nullptr;
            } else if (temp == tail) {
                tail = temp->prev;
                if (tail) tail->next = nullptr;
            } else {
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
            }
            if (current == temp) {
                current = temp->next ? temp->next : temp->prev;
            }
            delete temp;
            cout << "Removed '" << title << "' from playlist.\n";
            return;
        }
        temp = temp->next;
    }
    cout << "Song '" << title << "' not found in playlist.\n";
}

void PlaylistManager::queueUpNext(string title) {
    Song s;
    if (!db->getSong(title, s)) {
        cout << "Error: Song not found.\n";
        return;
    }
    upNextQueue.push(s);
    cout << "Queued '" << title << "' to play next.\n";
}

void PlaylistManager::viewQueue() const {
    cout << "\n=== Up Next Queue ===\n";
    if (upNextQueue.empty()) {
        cout << "Queue is empty.\n";
    } else {
        queue<Song> temp = upNextQueue;
        int count = 1;
        while (!temp.empty()) {
            cout << count++ << ". " << temp.front().title << "\n";
            temp.pop();
        }
    }
    cout << "=====================\n";
}

void PlaylistManager::playNext() {
    // If there's a song in the queue, play it first (interrupt normal flow)
    if (!upNextQueue.empty()) {
        Song nextQueued = upNextQueue.front();
        upNextQueue.pop();
        
        if (current) {
            playHistory.push(current->song);
        }
        
        // We create a temporary node for the queued song and insert it right after current
        Node* queuedNode = new Node(nextQueued);
        if (current) {
            queuedNode->next = current->next;
            queuedNode->prev = current;
            if (current->next) {
                current->next->prev = queuedNode;
            } else {
                tail = queuedNode;
            }
            current->next = queuedNode;
            current = queuedNode;
        } else {
            // Playlist was empty
            head = tail = current = queuedNode;
        }
        
        current->song.playCount++;
        cout << "Playing (from Queue): " << current->song.title << "\n";
        return;
    }

    // Normal forward traversal
    if (current == nullptr) {
        if (head != nullptr) {
            current = head;
            current->song.playCount++;
            cout << "Playing: " << current->song.title << "\n";
        } else {
            cout << "Playlist is empty.\n";
        }
        return;
    }

    if (current->next != nullptr) {
        playHistory.push(current->song);
        current = current->next;
        current->song.playCount++;
        cout << "Playing: " << current->song.title << "\n";
    } else {
        cout << "Reached the end of the playlist.\n";
    }
}

void PlaylistManager::playPrevious() {
    if (playHistory.empty()) {
        cout << "No previous songs in history.\n";
        return;
    }

    if (current != nullptr && current->prev != nullptr) {
        // Technically popping from history just means we go back.
        // If we strictly follow the history stack, we pop.
        current = current->prev;
        playHistory.pop();
        current->song.playCount++;
        cout << "Playing: " << current->song.title << "\n";
    } else {
        cout << "At the beginning of the playlist.\n";
    }
}

void PlaylistManager::displayCurrentSong() const {
    if (current == nullptr) {
        cout << "No song is currently playing.\n";
    } else {
        cout << "\n=== Now Playing ===\n";
        cout << "Title: " << current->song.title << "\n";
        cout << "Artist: " << current->song.artist << "\n";
        cout << "Duration: " << current->song.duration << " mins\n";
        cout << "===================\n";
    }
}

void PlaylistManager::displayPlaylist() const {
    cout << "\n=== Current Playlist ===\n";
    Node* temp = head;
    int index = 1;
    if (!temp) cout << "Playlist is empty.\n";
    while (temp != nullptr) {
        if (temp == current) cout << " -> ";
        else cout << "    ";
        
        cout << index++ << ". " << temp->song.title << " (" << temp->song.duration << " mins)\n";
        temp = temp->next;
    }
    cout << "========================\n";
}

void PlaylistManager::displayHistory() const {
    cout << "\n=== Listening History (Most Recent First) ===\n";
    if (playHistory.empty()) {
        cout << "History is empty.\n";
    } else {
        stack<Song> temp = playHistory; // Copy to avoid modifying original
        int count = 1;
        while (!temp.empty()) {
            cout << count++ << ". " << temp.top().title << "\n";
            temp.pop();
        }
    }
    cout << "=============================================\n";
}

void PlaylistManager::clearHistory() {
    while (!playHistory.empty()) {
        playHistory.pop();
    }
    cout << "Listening history cleared.\n";
}

// ================= SORTING (Merge Sort on DLL) =================

Node* PlaylistManager::split(Node* h) {
    Node* fast = h;
    Node* slow = h;
    while (fast->next && fast->next->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    Node* temp = slow->next;
    slow->next = nullptr;
    return temp;
}

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

Node* PlaylistManager::mergeSortRecursive(Node* node) {
    if (!node || !node->next) return node;

    Node* second = split(node);
    
    node = mergeSortRecursive(node);
    second = mergeSortRecursive(second);

    return merge(node, second);
}

void PlaylistManager::sortByDuration() {
    if (!head || !head->next) return;
    
    head = mergeSortRecursive(head);
    
    // Fix tail
    Node* temp = head;
    while (temp->next) {
        temp = temp->next;
    }
    tail = temp;
    
    // Reset current pointer (it might be lost in shuffle, so we just set to head)
    current = head;
    cout << "Playlist sorted by duration!\n";
}

// ================= MAX HEAP =================

void PlaylistManager::showMostPlayed() {
    priority_queue<Song, vector<Song>, ComparePlayCount> maxHeap;
    
    // Iterate through all songs in DLL and put in heap
    // Wait, the playCounts are updated in the DLL nodes but they are copies.
    // If we want a global "Most Played", we should look at a global record or just DLL.
    // We'll look at the DLL for this local playlist session.
    Node* temp = head;
    while(temp) {
        if(temp->song.playCount > 0)
            maxHeap.push(temp->song);
        temp = temp->next;
    }
    
    cout << "\n=== Top Played Songs in this Session ===\n";
    if (maxHeap.empty()) {
        cout << "No songs have been played yet.\n";
    } else {
        int count = 1;
        while (!maxHeap.empty() && count <= 5) { // Show top 5
            Song s = maxHeap.top();
            maxHeap.pop();
            cout << count++ << ". " << s.title << " (Played " << s.playCount << " times)\n";
        }
    }
    cout << "========================================\n";
}


// ================= DP 0-1 Knapsack =================
// Finds combination of songs from current playlist that perfectly fits target time

void PlaylistManager::generateExactDurationPlaylist(double targetMinutes) {
    // Collect all songs from current DLL
    vector<Song> songs;
    Node* temp = head;
    while(temp) {
        songs.push_back(temp->song);
        temp = temp->next;
    }
    
    int n = songs.size();
    if(n == 0) {
        cout << "Playlist is empty.\n";
        return;
    }
    
    // DP needs integers. We'll multiply minutes by 100 to avoid floats.
    int target = (int)(targetMinutes * 100);
    
    // DP table: dp[i][w] = max rating we can get from first i items within weight limit w
    vector<vector<int>> dp(n + 1, vector<int>(target + 1, 0));
    
    for (int i = 1; i <= n; i++) {
        int weight = (int)(songs[i-1].duration * 100);
        int value = songs[i-1].rating; // Maximize user enjoyment/rating
        
        for (int w = 1; w <= target; w++) {
            if (weight <= w) {
                dp[i][w] = max(dp[i-1][w], dp[i-1][w - weight] + value);
            } else {
                dp[i][w] = dp[i-1][w];
            }
        }
    }
    
    // Backtrack to find which songs were included
    vector<Song> selected;
    int res = dp[n][target];
    int w = target;
    for (int i = n; i > 0 && res > 0; i--) {
        if (res == dp[i-1][w]) continue;
        else {
            selected.push_back(songs[i-1]);
            int weight = (int)(songs[i-1].duration * 100);
            int value = songs[i-1].rating;
            res -= value;
            w -= weight;
        }
    }
    
    cout << "\n=== DP Knapsack: Suggested Playlist for ~" << targetMinutes << " mins ===\n";
    double totalTime = 0.0;
    int totalRating = 0;
    for(const auto& s : selected) {
        cout << " - " << s.title << " (" << s.duration << " mins, Rating: " << s.rating << ")\n";
        totalTime += s.duration;
        totalRating += s.rating;
    }
    cout << "Total Suggested Time: " << totalTime << " mins | Total Rating: " << totalRating << "\n";
    cout << "========================================================\n";
}
