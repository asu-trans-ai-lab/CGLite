// CGLite.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
struct Node {
    int node_id;
    std::string zone_id; // Assuming zone_id can be empty and thus a string.
    double x_coord;
    double y_coord;
    int original_node_id;
    std::string activity_type; // Assuming activity_type can be empty and thus a string.
};


struct Link {
    int link_id;
    int from_node_id;
    int to_node_id;
    std::string facility_type; // assuming facility_type is sometimes empty and a string is more suitable.
    int dir_flag;
    double length;
    int lanes; // assuming lanes can be empty, so we might need to handle it accordingly.
    int capacity; // assuming capacity can be empty, so we might need to handle it accordingly.
    double free_speed; // assuming free_speed can be empty, so we might need to handle it accordingly.
    std::string link_type_name;
    int link_type;
    int cost; // assuming cost can be empty, so we might need to handle it accordingly.
};

using Links = std::vector<Link>;


struct Demand {
    int o_zone_id;
    int d_zone_id;
    double volume;
};

std::vector<Demand> readDemandsFromCSV(const std::string& filename) {
    std::vector<Demand> demands;
    std::ifstream file(filename);
    std::string line;

    // Skip the header line
    getline(file, line);

    while (getline(file, line)) {
        std::istringstream s(line);
        Demand demand;
        std::string token;

        getline(s, token, ',');
        demand.o_zone_id = std::stoi(token);

        getline(s, token, ',');
        demand.d_zone_id = std::stoi(token);

        getline(s, token, ',');
        demand.volume = std::stod(token);

        demands.push_back(demand);
    }
    file.close(); // Don't forget to close the file
    return demands;
}


using Demands = std::vector<Demand>;

std::vector<Node> readNodesFromCSV(const std::string& filename) {
    std::vector<Node> nodes;
    std::ifstream file(filename);
    std::string line, word;

    // Skip the header line
    getline(file, line);

    while (getline(file, line)) {
        std::istringstream s(line);
        Node node;
        std::string token;

        getline(s, token, ',');
        node.node_id = std::stoi(token);

        getline(s, token, ',');
        node.zone_id = token;

        getline(s, token, ',');
        node.x_coord = std::stod(token);

        getline(s, token, ',');
        node.y_coord = std::stod(token);

        getline(s, token, ',');
        node.original_node_id = std::stoi(token);

        getline(s, token, ',');
        node.activity_type = token;

        nodes.push_back(node);
    }
    return nodes;
}

Links readLinksFromCSV(const std::string& filename) {
    Links links;
    std::ifstream file(filename);
    std::string line, word;

    // Skip the header line
    getline(file, line);

    while (getline(file, line)) {
        std::istringstream s(line);
        Link link;
        std::string token;

        getline(s, token, ',');
        link.link_id = std::stoi(token);

        getline(s, token, ',');
        link.from_node_id = std::stoi(token);

        getline(s, token, ',');
        link.to_node_id = std::stoi(token);

        // ... Continue parsing other fields similarly

        links.push_back(link);
    }
    return links;
}


// Function to read the node CSV and return the unique count of zone IDs
size_t countUniqueZones(const std::string& filename) {
    std::ifstream file(filename);
    std::string line, token;
    std::unordered_set<std::string> unique_zones;

    // Skip the header line
    getline(file, line);

    while (getline(file, line)) {
        std::istringstream s(line);
        Node node;

        getline(s, token, ','); // Skip node_id
        getline(s, token, ','); // zone_id is the second token

        // Check if the zone_id is not empty and add to the set
        if (!token.empty()) {
            unique_zones.insert(token);
        }

        // Skip the rest of the line
    }
    file.close();
    return unique_zones.size();
}
struct RouteAssignment {
    int route_seq_id;
    int o_zone_id;
    int d_zone_id;
    std::string demand_period;
    double volume;
    double distance_km;
    double travel_time;
    int number_of_nodes;
    std::vector<int> node_sequence;
    std::vector<int> link_sequence;
};

std::vector<int> parseSequence(const std::string& sequence) {
    std::vector<int> result;
    std::stringstream ss(sequence);
    std::string item;
    while (getline(ss, item, ';')) {
        if (!item.empty()) {
            result.push_back(std::stoi(item));
        }
    }
    return result;
}

std::vector<RouteAssignment> readRouteAssignmentsFromCSV(const std::string& filename) {
    std::vector<RouteAssignment> route_assignments;
    std::ifstream file(filename);
    std::string line;

    // Skip the header line
    getline(file, line);

    while (getline(file, line)) {
        std::istringstream s(line);
        RouteAssignment assignment;
        std::string token;

        getline(s, token, ',');
        assignment.route_seq_id = std::stoi(token);

        getline(s, token, ',');
        assignment.o_zone_id = std::stoi(token);

        getline(s, token, ',');
        assignment.d_zone_id = std::stoi(token);

        getline(s, token, ',');
        assignment.demand_period = token;

        getline(s, token, ',');
        assignment.volume = std::stod(token);

        getline(s, token, ',');
        assignment.distance_km = std::stod(token);

        getline(s, token, ',');
        assignment.travel_time = std::stod(token);

        getline(s, token, ',');
        assignment.number_of_nodes = std::stoi(token);

        getline(s, token, ',');
        assignment.node_sequence = parseSequence(token);

        getline(s, token, ',');
        assignment.link_sequence = parseSequence(token);

        route_assignments.push_back(assignment);
    }
    file.close();
    return route_assignments;
}
// Define your indices as types for clarity
using Zone = int;
using Mode = std::string; // Mode can be represented by strings like "car", "bus", etc.
using TimePeriod = std::string; // Time period can be represented by strings like "am", "pm", etc.
using Link = int;
using Path = std::vector<Link>; // Simple representation of a Path as a sequence of Links

// Parameters and Variables
std::unordered_map<Zone, std::unordered_map<Zone, std::unordered_map<Mode, std::unordered_map<TimePeriod, double>>>> D; // OD demand
std::unordered_map<Link, double> Cap; // Capacity of each link
std::unordered_map<Link, std::unordered_map<TimePeriod, double>> V; // Volume of each link
std::unordered_map<Link, std::unordered_map<TimePeriod, double>> TT; // Travel time for each link
std::unordered_map<Zone, std::unordered_map<Zone, std::unordered_map<Mode, std::unordered_map<TimePeriod, double>>>> TT_OD; // Travel time for OD
// Note: LP is not explicitly initialized here, assuming it's calculated as part of model functions.

// Function to compute path travel time
void computePathTravelTime() {
    // Implementation goes here
}

// Function to determine travel time for each OD pair
void determineODPairTravelTime() {
    // Implementation goes here
}

// Assuming that the types and data structures for TT, LP, D, and V are defined as follows:
using Zone = int;
using Mode = std::string;
using TimePeriod = std::string;
using Link = int;
using PathIndex = int;

// Travel time for each path
std::unordered_map<PathIndex, std::unordered_map<TimePeriod, double>> TT;

// Link proportion matrix
std::unordered_map<Zone, std::unordered_map<Zone, std::unordered_map<Mode, std::unordered_map<TimePeriod, std::unordered_map<PathIndex, std::unordered_map<Link, double>>>>>> LP;

// OD demand
std::unordered_map<Zone, std::unordered_map<Zone, std::unordered_map<Mode, std::unordered_map<TimePeriod, double>>>> D;

// Volume for each link
std::unordered_map<Link, std::unordered_map<TimePeriod, double>> V;

// Function to compute the link volumes based on the conservation of flow
void computeLinkVolumes() {
    // Iterate over each time period
    for (const auto& time_period_pair : TT) {
        const TimePeriod& tau = time_period_pair.first;

        // Reset volumes for the new time period
        for (auto& link_volume_pair : V) {
            link_volume_pair.second[tau] = 0.0;
        }

        // Iterate over all origins
        for (const auto& origin_pair : LP) {
            Zone o = origin_pair.first;

            // Iterate over all destinations
            for (const auto& destination_pair : origin_pair.second) {
                Zone d = destination_pair.first;

                // Iterate over all modes
                for (const auto& mode_pair : destination_pair.second) {
                    Mode m = mode_pair.first;

                    // Iterate over all paths for this OD pair and mode
                    for (const auto& path_pair : mode_pair.second[tau]) {
                        PathIndex p = path_pair.first;
                        double path_travel_time = TT[p][tau]; // Get the travel time for this path

                        // Iterate over all links in this path
                        for (const auto& link_proportion_pair : path_pair.second) {
                            Link l = link_proportion_pair.first;
                            double proportion = link_proportion_pair.second;

                            // Add the flow contribution to this link
                            V[l][tau] += path_travel_time * proportion;
                        }
                    }
                }
            }
        }
    }
}

int main() {
    // Initialize your parameters and variables
    // This will typically involve reading from input files or databases
    // For example:
    D[1][2]["car"]["am"] = 100; // 100 cars from zone 1 to 2 in the morning
    Cap[1] = 50; // Capacity of link 1

    // Call your model functions
    computeLinkVolumes(); 
    computePathTravelTime();
    determineODPairTravelTime();

    // Process results
    // ...

    return 0;
}
//int main() {
//    std::string node_filename = "node.csv";
//    std::vector<Node> nodes = readNodesFromCSV(node_filename);
//
//    std::string link_filename = "link.csv";
//    Links links = readLinksFromCSV(link_filename);
//
//    std::string filename = "node.csv"; // replace with your actual file path
//    size_t zone_count = countUniqueZones(filename);
//    std::cout << "Unique zone count: " << zone_count << std::endl;
//    // Process the nodes and links as needed
//    // ...
//    std::string filename = "route_assignment.csv"; // Replace with your actual file path
//    std::vector<RouteAssignment> assignments = readRouteAssignmentsFromCSV(filename);
//
//    // For demonstration, let's print the route assignments
//    for (const RouteAssignment& assignment : assignments) {
//        std::cout << "Route Sequence ID: " << assignment.route_seq_id
//            << ", Origin Zone ID: " << assignment.o_zone_id
//            << ", Destination Zone ID: " << assignment.d_zone_id
//            << ", Demand Period: " << assignment.demand_period
//            << ", Volume: " << assignment.volume
//            << ", Distance (km): " << assignment.distance_km
//            << ", Travel Time: " << assignment.travel_time
//            << ", Number of Nodes: " << assignment.number_of_nodes
//            << ", Node Sequence: ";
//        for (int node : assignment.node_sequence) {
//            std::cout << node << ";";
//        }
//        std::cout << ", Link Sequence: ";
//        for (int link : assignment.link_sequence) {
//            std::cout << link << ";";
//        }
//        std::cout << std::endl;
//    }
//
//    return 0;
//}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
