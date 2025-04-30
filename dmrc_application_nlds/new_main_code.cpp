// in this dijkstra's Algorithm is used on multilist to make it more optimized
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
// Structure to represent a metro station
struct Station {
    string name;
    int code;
    string color;
    float latitude;
    float longitude;
    vector<pair<int, float>> adjacentStations; // Store adjacent station code and distance
};

// Multilist to represent metro stations and their connections
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
void establishConnections(const string& filename);
int minDistance(float dist[], bool sptSet[]);
void dijkstra(int src, int dest);
void printShortestPath(const vector<int>& shortestPath);
void take_input();
// void Path(float d, int e, int st);
void logo(int x, int y);
void UI();

COORD coord;
void clrscreen()
{
    system("cls");
}

void delay(unsigned int ms)
{
    clock_t goal = ms + clock();
    while (goal > clock())
        ;
}

void gotoxy(int x, int y)
{
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

int timetaken(float dist)
{
    float speed = 0.55;
    return ceil(dist / speed);
}

// Function to convert string to uppercase
string makeCapital(string str) {
    for (char& c : str) {
        if (c >= 'a' && c <= 'z') {
            c = toupper(c);
        }
    }
    return str;
}

// Function to find index of station with given name in multilist
int findStationIndex(const string& name) {
    for (size_t i = 0; i < stations.size(); ++i) {
        if (makeCapital(stations[i].name) == makeCapital(name)) {
            return i;
        }
    }
    return -1; // Station not found
}

// Function to add connection between two stations in multilist
void addConnection(const string& sourceName, const string& destName, float distance) {
    int sourceIndex = findStationIndex(sourceName);
    int destIndex = findStationIndex(destName);
    if (sourceIndex != -1 && destIndex != -1) {
     stations[sourceIndex].adjacentStations.emplace_back(destIndex, distance);
     stations[destIndex].adjacentStations.emplace_back(sourceIndex, distance);
    }
}

// Function to calculate distance between two stations using their latitude and longitude
float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371.0; // Radius of Earth in kilometers
    float lat1Rad = lat1 * M_PI / 180.0;
    float lon1Rad = lon1 * M_PI / 180.0;
    float lat2Rad = lat2 * M_PI / 180.0;
    float lon2Rad = lon2 * M_PI / 180.0;
    float dLat = lat2Rad - lat1Rad;
    float dLon = lon2Rad - lon1Rad;
    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(lat1Rad) * cos(lat2Rad) *
              sin(dLon / 2) * sin(dLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c; // Distance in kilometers
}

// Function to parse the station codes file
void parseStationCodes(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string stationName;
        while (getline(file, stationName)) {
            Station station;
            station.name = stationName;
            stations.push_back(station);
        }
        file.close();
    } else {
        cout << "Unable to open file: " << filename << endl;
    }
}

// Function to parse the station color codes file
void parseStationColors(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
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
    } else {
        cout << "Unable to open file: " << filename << endl;
    }
}

// Function to parse the node value file and establish connections between stations
void establishConnections(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            int stationIndex, numConnections;
            ss >> numConnections;
            ss >> stationIndex;
            for (int i = 0; i < numConnections; ++i) {
                int destIndex;
                float distance;
                ss >> destIndex >> distance;
                graph[stationIndex - 1][destIndex - 1] = distance;
            }
        }
        file.close();
    } else {
        cout << "Unable to open file: " << filename << endl;
    }
    UI();
}

int minDistance(float dist[], bool sptSet[]) {
    float min = INT_MAX;
    int min_index;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}

// Function to perform Dijkstra's algorithm and return the shortest path
void dijkstra(int src, int dest) {
    float dist[V];
    bool sptSet[V];
    vector<int> shortestPath;
    vector<float> parent[V];

    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = false;
    }

    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, sptSet);
        sptSet[u] = true;

        for (int v = 0; v < V; v++) {
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
                parent[v].clear();
                parent[v].push_back(u);
            } else if (!sptSet[v] && graph[u][v] && dist[u] + graph[u][v] == dist[v]) {
                parent[v].push_back(u);
            }
        }
    }

    // Construct the shortest path from parent array
    int current = dest;
    while (current != src) {
        shortestPath.push_back(current);
        if (parent[current].empty())
            break;
        current = parent[current][0];
    }
    shortestPath.push_back(src);
    std::reverse(shortestPath.begin(), shortestPath.end());

    printShortestPath(shortestPath);
}

// Function to print the shortest path including station names and relevant information
void printShortestPath(const vector<int>& shortestPath) {
    if (shortestPath.empty()) {
        cout << "No path found!" << endl;
        return;
    }

    gotoxy(44, 13);
    cout << "****** LOADING ******";
    delay(2000);
    gotoxy(42, 13);
    cout << "****** ROUTE FOUND ******";
    cout<<endl;
    int n_of_stations = shortestPath.size();
    cout << "Shortest Path: ";
    for (int i = 0; i < shortestPath.size(); ++i) {
        int stationIndex = shortestPath[i];
        cout << stations[stationIndex].name;
        if (i < shortestPath.size() - 1)
            cout << " -> ";
    }
    cout << endl;
    delay(1000);
    gotoxy(72, 12);
    cout << "NO OF STATIONS : ";
    gotoxy(89, 12);
    cout << n_of_stations - 1 << "";

    // this is for searching again
    delay(2500);
    gotoxy(44, 9);
    cout << "WANT TO SEARCH AGAIN ?  ";
    string choice;
    cin >> choice;
    choice = makeCapital(choice);
    if (choice == "Y" || choice == "YES")
        secondWindow();
    gotoxy(5, 30);
    char ch;
    scanf("%c", &ch);
    cout << endl;
    return;

}

void take_input() {
    char ch;
    string start_s, end_s;

    // Get input for starting and ending stations
    gotoxy(16, 3);
    cout << "ENTER THE STARTING STATION: ";
    gotoxy(20, 5);
    getline(cin, start_s);
    gotoxy(72, 3);
    cout << "ENTER THE DESTINATION STATION: ";
    gotoxy(76, 5);
    getline(cin, end_s);

    // Convert input to uppercase
    start_s = makeCapital(start_s);
    end_s = makeCapital(end_s);

    // Find station indices
    int fault = 0;
    int startcode = -1, endcode = -1;
    for (int i = 0; i < V; ++i) {
        if (stations[i].name == start_s) {
            startcode = i;
            break;
        }
    }
    if (startcode == -1) {
        gotoxy(42, 10);
        cout << "INVALID STARTING STATION NAME ENTERED" << endl;
        delay(2500);
        fault = 1;
    }
    for (int i = 0; i < V; ++i) {
        if (stations[i].name == end_s) {
            endcode = i;
            break;
        }
    }
    if (endcode == -1) {
        gotoxy(40, 11);
        cout << "INVALID DESTINATION STATION NAME ENTERED" << endl;
        delay(2500);
        fault = 1;
    }
    if (fault)
    {
        secondWindow();
        return;
    }
    else{
        // Call Dijkstra's algorithm to find shortest path
        dijkstra(startcode, endcode);
    }
}

void logo(int x, int y)
{
    gotoxy(x, y);
    printf("   ___                     ___       ____");
    gotoxy(x, y + 1);
    printf(" ||   \\\\   ||\\\\    //||  ||   \\\\   //    \\\\");
    gotoxy(x, y + 2);
    printf(" ||    ||  || \\\\  // ||  ||    || ||");
    gotoxy(x, y + 3);
    printf(" ||    ||  ||  \\\\//  ||  ||___//  ||");
    gotoxy(x, y + 4);
    printf(" ||    ||  ||        ||  ||  \\\\   ||");
    gotoxy(x, y + 5);
    printf(" ||___//   ||        ||  ||   \\\\   \\\\____//");
}


void secondWindow() {
    clrscreen();
    gotoxy(48, 19);
    system("color 0D");
    delay(90);
    
    // Prompt user for input and find shortest path
    take_input();
}

void UI()
{
    delay(90);
    system("color 0A");
    delay(90);
    logo(37, 4);
    delay(90);
    delay(90);
    delay(90);
    gotoxy(42, 14);
    cout << "WELCOME TO DELHI METRO RAIL APP";
    delay(90);
    delay(90);
    gotoxy(47, 18);
    printf("PRESS ENTER TO CONTINUE");
    char ch;
    scanf("%c", &ch);
    secondWindow();
}

int main() {
    // Parse station codes and colors
    parseStationCodes("stationcodes.txt");
    parseStationColors("stationcolorcodes.txt");

    // Establish connections between stations
    establishConnections("node_values_new.txt");

    // Now, the connections between stations are established in the graph array
    return 0;
}
