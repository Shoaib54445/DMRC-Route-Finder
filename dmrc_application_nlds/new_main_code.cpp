// DMRC Route Generator with Crowd-Based Fastest Path Option
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>
#include <windows.h>
#include <time.h>

using namespace std;

const int V = 248;
float graph[V][V];
int routeMode = 1; // 1 = shortest, 2 = fastest (crowd-based)

// Structure to represent a metro station
struct Station {
    string name;
    int code;
    string color;
    float latitude;
    float longitude;
    vector<pair<int, float>> adjacentStations;
    float crowdFactor = 1.0; // NEW: default crowd multiplier
};

vector<Station> stations;

void secondWindow();
void clrscreen();
void delay(unsigned int ms);
void gotoxy(int x, int y);
int timetaken(float dist);
string makeCapital(string str);
int findStationIndex(const string& name);
void addConnection(const string& sourceName, const string& destName, float distance);
float calculateDistance(float lat1, float lon1, float lat2, float lon2);
void parseStationCodes(const string& filename);
void parseStationColors(const string& filename);
void parseStationCrowd(const string& filename);
void establishConnections(const string& filename);
int minDistance(float dist[], bool sptSet[]);
void dijkstra(int src, int dest);
void printShortestPath(const vector<int>& shortestPath);
void take_input();
void logo(int x, int y);
void UI();

COORD coord;
void clrscreen() { system("cls"); }
void delay(unsigned int ms) { clock_t goal = ms + clock(); while (goal > clock()); }
void gotoxy(int x, int y) { coord.X = x; coord.Y = y; SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord); }
int timetaken(float dist) { return ceil(dist / 0.55); }

string makeCapital(string str) {
    for (char& c : str) c = toupper(c);
    return str;
}

int findStationIndex(const string& name) {
    for (size_t i = 0; i < stations.size(); ++i) {
        if (makeCapital(stations[i].name) == makeCapital(name)) return i;
    }
    return -1;
}

void addConnection(const string& sourceName, const string& destName, float distance) {
    int sourceIndex = findStationIndex(sourceName);
    int destIndex = findStationIndex(destName);
    if (sourceIndex != -1 && destIndex != -1) {
        stations[sourceIndex].adjacentStations.emplace_back(destIndex, distance);
        stations[destIndex].adjacentStations.emplace_back(sourceIndex, distance);
    }
}

float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371.0;
    float dLat = (lat2 - lat1) * M_PI / 180.0;
    float dLon = (lon2 - lon1) * M_PI / 180.0;
    float a = sin(dLat/2)*sin(dLat/2) + cos(lat1*M_PI/180.0)*cos(lat2*M_PI/180.0)*sin(dLon/2)*sin(dLon/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}

void parseStationCodes(const string& filename) {
    ifstream file(filename);
    string stationName;
    while (getline(file, stationName)) {
        Station station;
        station.name = stationName;
        stations.push_back(station);
    }
    file.close();
}

void parseStationColors(const string& filename) {
    ifstream file(filename);
    string line;
    int index = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        string color;
        while (getline(ss, color, ',')) {
            stations[index].color = color;
            index++;
        }
    }
    file.close();
}

void parseStationCrowd(const string& filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string stationName;
        float crowd;
        getline(iss, stationName, ' ');
        iss >> crowd;
        int index = findStationIndex(stationName);
        if (index != -1) stations[index].crowdFactor = crowd;
    }
    file.close();
}

void establishConnections(const string& filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        int numConnections, stationIndex;
        ss >> numConnections >> stationIndex;
        for (int i = 0; i < numConnections; ++i) {
            int destIndex;
            float distance;
            ss >> destIndex >> distance;
            graph[stationIndex - 1][destIndex - 1] = distance;
        }
    }
    file.close();
    UI();
}

int minDistance(float dist[], bool sptSet[]) {
    float min = INT_MAX;
    int min_index;
    for (int v = 0; v < V; v++)
        if (!sptSet[v] && dist[v] <= min)
            min = dist[v], min_index = v;
    return min_index;
}

void dijkstra(int src, int dest) {
    float dist[V];
    bool sptSet[V];
    vector<int> shortestPath;
    vector<float> parent[V];
    for (int i = 0; i < V; i++) dist[i] = INT_MAX, sptSet[i] = false;
    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, sptSet);
        sptSet[u] = true;
        for (int v = 0; v < V; v++) {
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX) {
                float cost = graph[u][v];
                if (dist[u] + cost < dist[v]) {
                    dist[v] = dist[u] + cost;
                    parent[v].clear();
                    parent[v].push_back(u);
                } else if (dist[u] + cost == dist[v]) {
                    parent[v].push_back(u);
                }
            }
        }
    }

    int current = dest;
    while (current != src) {
        shortestPath.push_back(current);
        if (parent[current].empty()) break;
        current = parent[current][0];
    }
    shortestPath.push_back(src);
    reverse(shortestPath.begin(), shortestPath.end());
    printShortestPath(shortestPath);
}

void printShortestPath(const vector<int>& shortestPath) {
    if (shortestPath.empty()) {
        cout << "No path found!" << endl;
        return;
    }
    gotoxy(44, 13);
    cout << "****** LOADING ******";
    delay(2000);
    gotoxy(42, 13);
    cout << "****** ROUTE FOUND ******\n";
    int n_of_stations = shortestPath.size();
    cout << "Shortest Path: ";
    for (int i = 0; i < n_of_stations; ++i) {
        cout << stations[shortestPath[i]].name;
        if (i < n_of_stations - 1) cout << " -> ";
    }
    cout << endl;
    delay(1000);
    gotoxy(72, 12);
    cout << "NO OF STATIONS : ";
    gotoxy(89, 12);
    cout << n_of_stations - 1;
    delay(2500);
    gotoxy(44, 9);
    cout << "WANT TO SEARCH AGAIN ?  ";
    string choice;
    cin >> choice;
    choice = makeCapital(choice);
    if (choice == "Y" || choice == "YES") secondWindow();
    gotoxy(5, 30);
    char ch;
    scanf("%c", &ch);
    cout << endl;
}

void take_input() {
    string start_s, end_s;
    gotoxy(16, 3); cout << "ENTER THE STARTING STATION: ";
    gotoxy(20, 5); getline(cin, start_s);
    gotoxy(72, 3); cout << "ENTER THE DESTINATION STATION: ";
    gotoxy(76, 5); getline(cin, end_s);

    start_s = makeCapital(start_s);
    end_s = makeCapital(end_s);
    int startcode = findStationIndex(start_s);
    int endcode = findStationIndex(end_s);
    int fault = 0;

    if (startcode == -1) { gotoxy(42, 10); cout << "INVALID STARTING STATION NAME ENTERED"; delay(2500); fault = 1; }
    if (endcode == -1) { gotoxy(40, 11); cout << "INVALID DESTINATION STATION NAME ENTERED"; delay(2500); fault = 1; }

    if (fault) { secondWindow(); return; }

    if (routeMode == 2) {
        for (int i = 0; i < V; ++i) {
            for (int j = 0; j < V; ++j) {
                if (graph[i][j] > 0) {
                    float avgCrowd = (stations[i].crowdFactor + stations[j].crowdFactor) / 2.0;
                    graph[i][j] *= avgCrowd;
                }
            }
        }
    }
    dijkstra(startcode, endcode);
}

void logo(int x, int y) {
    gotoxy(x, y);     printf("   ___                     ___       ____");
    gotoxy(x, y + 1); printf(" ||   \\\\   ||\\\\    //||  ||   \\\\   //    \\\\");
    gotoxy(x, y + 2); printf(" ||    ||  || \\\\  // ||  ||    ||  ||");
    gotoxy(x, y + 3); printf(" ||    ||  ||  \\\\//  ||  ||___//  ||");
    gotoxy(x, y + 4); printf(" ||    ||  ||        ||  ||  \\\\   ||");
    gotoxy(x, y + 5); printf(" ||___//   ||        ||  ||   \\\\   \\\\____//");
}


void secondWindow() {
    clrscreen();
    gotoxy(48, 19);
    system("color 0D");
    delay(90);
    take_input();
}

void UI() {
    delay(90);
    system("color 0A");
    logo(37, 4);
    delay(90);
    gotoxy(42, 14); cout << "WELCOME TO DELHI METRO RAIL APP";
    delay(90);
    gotoxy(47, 18); printf("PRESS ENTER TO CONTINUE");
    char ch; scanf("%c", &ch);

    clrscreen();
    gotoxy(40, 8); cout << "SELECT ROUTE TYPE:";
    gotoxy(40, 10); cout << "1. SHORTEST PATH";
    gotoxy(40, 11); cout << "2. FASTEST PATH (CONSIDERING CROWD)";
    gotoxy(40, 13); cout << "ENTER CHOICE (1 or 2): ";
    cin >> routeMode;
    cin.ignore();

    secondWindow();
}

int main() {
    parseStationCodes("stationcodes.txt");
    parseStationColors("stationcolorcodes.txt");
    parseStationCrowd("station_crowd.txt");
    establishConnections("node_values_new.txt");
    return 0;
}
