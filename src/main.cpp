#include <iostream>
#include <string>
#include "MusicDatabase.h"
#include "PlaylistManager.h"
#include "Recommendation.h"

using namespace std;

void displayMenu() {
    cout << "\n========== Music Playlist Manager ==========\n";
    cout << "1.  View Global Library (A-Z)\n";
    cout << "2.  Search / Autocomplete Song\n";
    cout << "3.  Add Song to Playlist\n";
    cout << "4.  Remove Song from Playlist\n";
    cout << "5.  Queue Song (Up Next)\n";
    cout << "6.  View Up Next Queue\n";
    cout << "7.  Play Next\n";
    cout << "8.  Play Previous (Undo)\n";
    cout << "9.  Show Now Playing\n";
    cout << "10. Show Current Playlist\n";
    cout << "11. Sort Playlist by Duration\n";
    cout << "12. View Listening History\n";
    cout << "13. Clear Listening History\n";
    cout << "14. Show Most Played Songs\n";
    cout << "15. Generate Target Duration Playlist (DP Knapsack)\n";
    cout << "16. Recommend Similar Songs\n";
    cout << "0.  Exit\n";
    cout << "============================================\n";
    cout << "Enter your choice: ";
}

int main() {
    // 1. Initialize DB and seed
    MusicDatabase db;
    db.seedDatabase();

    // 2. Initialize Playlist Manager
    PlaylistManager pm(&db);

    // 3. Initialize Recommendation Graph
    RecommendationGraph rg;
    rg.buildGraph(db.getAllSongs());

    int choice = -1;
    string inputStr;
    double inputDouble;

    while (choice != 0) {
        displayMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        cin.ignore(); // clear newline

        switch (choice) {
            case 1:
                db.displayLibraryAlphabetical();
                break;
            case 2:
                cout << "Enter search query: ";
                getline(cin, inputStr);
                db.autocompleteSearch(inputStr);
                break;
            case 3:
                cout << "Enter exact song title to add: ";
                getline(cin, inputStr);
                pm.addSong(inputStr);
                break;
            case 4:
                cout << "Enter exact song title to remove: ";
                getline(cin, inputStr);
                pm.removeSong(inputStr);
                break;
            case 5:
                cout << "Enter exact song title to queue next: ";
                getline(cin, inputStr);
                pm.queueUpNext(inputStr);
                break;
            case 6:
                pm.viewQueue();
                break;
            case 7:
                pm.playNext();
                break;
            case 8:
                pm.playPrevious();
                break;
            case 9:
                pm.displayCurrentSong();
                break;
            case 10:
                pm.displayPlaylist();
                break;
            case 11:
                pm.sortByDuration();
                break;
            case 12:
                pm.displayHistory();
                break;
            case 13:
                pm.clearHistory();
                break;
            case 14:
                pm.showMostPlayed();
                break;
            case 15:
                cout << "Enter target duration in minutes (e.g., 15.5): ";
                if (!(cin >> inputDouble)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Invalid input for duration.\n";
                } else {
                    cin.ignore();
                    pm.generateExactDurationPlaylist(inputDouble);
                }
                break;
            case 16:
                cout << "Enter a song title you like: ";
                getline(cin, inputStr);
                rg.recommendSimilar(inputStr);
                break;
            case 0:
                cout << "Exiting Music Playlist Manager...\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
