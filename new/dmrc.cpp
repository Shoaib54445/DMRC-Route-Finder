#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>
#include <climits>
using namespace std;

const int V = 248;
float graph[V][V];
int routeMode = 1; // 1 = shortest, 2 = fastest (crowd-based)

struct Station {
    string name;
    int code;
    string color;
    vector<pair<int, float>> adjacentStations;
    float crowdFactor = 1.0;
};

vector<Station> stations;

int findStationIndex(const string& name) {
    for (size_t i = 0; i < stations.size(); ++i) {
        if (stations[i].name == name) return i;
    }
    return -1;
}

string makeCapital(string str) {
    for (char& c : str) c = toupper(c);
    return str;
}

void parseStationCodes(const string& filename) {
    ifstream file(filename);
    string stationName;
    while (getline(file, stationName)) {
        Station station;
        station.name = makeCapital(stationName);
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
        getline(ss, color, ',');
        if (index < stations.size()) stations[index++].color = color;
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
        int index = findStationIndex(makeCapital(stationName));
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
}

vector<int> dijkstra(int src, int dest) {
    float dist[V];
    bool sptSet[V];
    vector<int> parent(V, -1);
    for (int i = 0; i < V; i++) dist[i] = INT_MAX, sptSet[i] = false;
    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = -1;
        float min = INT_MAX;
        for (int v = 0; v < V; v++)
            if (!sptSet[v] && dist[v] <= min)
                min = dist[v], u = v;

        if (u == -1) break;
        sptSet[u] = true;

        for (int v = 0; v < V; v++) {
            if (!sptSet[v] && graph[u][v] > 0 && dist[u] != INT_MAX) {
                float cost = graph[u][v];
                if (routeMode == 2) {
                    float avgCrowd = (stations[u].crowdFactor + stations[v].crowdFactor) / 2.0;
                    cost *= avgCrowd;
                }
                if (dist[u] + cost < dist[v]) {
                    dist[v] = dist[u] + cost;
                    parent[v] = u;
                }
            }
        }
    }

    vector<int> path;
    for (int at = dest; at != -1; at = parent[at]) path.push_back(at);
    reverse(path.begin(), path.end());
    return path;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "{\"error\": \"Missing arguments\"}";
        return 1;
    }

    string src = makeCapital(argv[1]);
    string dst = makeCapital(argv[2]);
    routeMode = (string(argv[3]) == "least_crowded") ? 2 : 1;

    parseStationCodes("data/stationcodes.txt");
    parseStationColors("data/stationcolorcodes.txt");
    parseStationCrowd("data/station_crowd.txt");
    establishConnections("data/node_values_new.txt");
    

    int srcIndex = findStationIndex(src);
    int dstIndex = findStationIndex(dst);

    if (srcIndex == -1 || dstIndex == -1) {
        cout << "{\"error\": \"Invalid station name\"}";
        return 1;
    }

    vector<int> path = dijkstra(srcIndex, dstIndex);
    float totalDistance = 0;
    for (size_t i = 1; i < path.size(); i++) totalDistance += graph[path[i - 1]][path[i]];

    cout << "{\"path\": [";
    for (size_t i = 0; i < path.size(); ++i) {
        cout << "{\"station\": \"" << stations[path[i]].name << "\", \"color\": \"" << stations[path[i]].color << "\"}";
        if (i != path.size() - 1) cout << ", ";
    }
    cout << "], \"distance\": \"" << totalDistance << " km\", \"time\": \"" << ceil(totalDistance / 0.55) << " mins\", \"stops\": " << (int)path.size() - 1 << "}";

    ofstream outFile("output.json");
    outFile << "{";
    outFile << "\"path\": [";
    for (size_t i = 0; i < path.size(); ++i) {
        outFile << "{\"station\": \"" << stations[path[i]].name << "\", \"color\": \"" << stations[path[i]].color << "\"}";
        if (i != path.size() - 1) outFile << ", ";
    }
    outFile << "], ";
    outFile << "\"distance\": \"" << totalDistance << " km\", ";
    outFile << "\"time\": \"" << ceil(totalDistance / 0.55) << " mins\", ";
    outFile << "\"stops\": " << (int)path.size() - 1;
    outFile << "}";
    outFile.close();

    return 0;
}