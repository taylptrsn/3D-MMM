#include "clustering.hpp"
#include "dme.hpp"
#include "globals.hpp"
#include "structures.hpp"
#include "tree.hpp"
#include "utilities.hpp"
#include <chrono>
#include <iomanip>

/* TODO
- Unit Conversion at Data read-in
- Split into header files
- Check delay calculations
- Find way to make sure MIV delay/cap from previous tier is not overwritten/is
assigned correctly.
- Note: This program is fairly robust, but there are still some edge cases that
may cause it to throw an error or infinite loop!
*/

using namespace std;

int main() {
  cleanupPreviousFiles();
  auto start = std::chrono::high_resolution_clock::now();
  // Create log file with timestamp
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  std::stringstream timestamp;
  timestamp << std::put_time(std::localtime(&time_t_now), "%Y%m%d_%H%M%S");
  std::ofstream logFile("log_" + timestamp.str() + ".txt");
  if (!logFile.is_open()) {
    std::cerr << "Error opening log file!" << std::endl;
    return 1;
  }

  // Redirect cout to the log file
  std::streambuf *coutBuf = std::cout.rdbuf();
  std::cout.rdbuf(logFile.rdbuf());
  int bound = 150; // Inserts Bound+1 MIVs per tier, Bound MIVs + 1 MIV for the
                   // unclustered sinks, total MIVs inserted = (Bound * Tier)
  int idealSum = 0;
  int subtreeTotalSum = 0;
  parseInput("benchmark10.txt");
  displayParsedData();
  // Separate sinks by their z-coordinate
  map<int, vector<Sink>> sinksByZ;
  std::map<int, std::vector<Sink>> tierMIVSinks;
  for (const auto &sink : sinks) {
    sinksByZ[sink.z].push_back(sink);
  }

  for (const auto &pair : sinksByZ) {
    int z = pair.first;
    // const auto &sinksGroup = pair.second;
    vector<Sink> sinksGroup = pair.second;
    int tierZsmSum = 0;

    // Add MIV sinks from lower tier (z-1) if they exist
    if (z > 1 && tierMIVSinks.find(z - 1) != tierMIVSinks.end()) {
      const auto &mivSinks = tierMIVSinks[z - 1];
      sinksGroup.insert(sinksGroup.end(), mivSinks.begin(), mivSinks.end());
    }

    // For each z-coordinate, generate the tree and then perform zero skew tree
    Node *root = AbsTreeGen3D(sinksGroup, bound - 1);
    int wireLength = calculateWirelength(sinksGroup);
    root->z = z; // Assign the z coordinate of the sink group to the root node
    cout << "\nProcessing z-coordinate: " << z << endl;
    // Create the tier-specific filename
    string tierFilename =
        "zeroskew_points_and_lines_z_" + to_string(z) + ".txt";
    assignPhysicalLocations(root);
    // assignPhysicalCharacteristics(root); WIP
    // Baseline case, eps = layout.width, minPts=1 or numSinks
    // double eps = layout.width * .085; // Epsilon distance
    double eps = layout.width; // Epsilon distance
    int minPts = 1;            // Minimum points to form a cluster
    runDBSCANAndAssignClusters(root, eps, minPts, bound - 1);
    clusterMidpoints = calculateClusterMidpoints(root);
    //  Create subtrees for each cluster
    std::vector<Node *> clusterRoots = createClusterSubtrees(root);
    printNodesByClusterId(root);
    calculateClusterMidpoints(root);
    // Print each subtree using printTree function
    for (Node *subtreeRoot : clusterRoots) {
      std::cout << "Printing subtree with Cluster ID: "
                << subtreeRoot->cluster_id << std::endl;
      Point currMidpoint = getMidpointByClusterId(subtreeRoot->cluster_id);
      std::cout << "Cluster " << subtreeRoot->cluster_id << " midpoint ("
                << currMidpoint.x << "," << currMidpoint.y << ")" << endl;
      std::vector<Sink> subtreeSinks = treeToSinkVector(subtreeRoot);
      Node *AbstractSubtree = AbsTreeGen3D(subtreeSinks, bound - 1);
      std::cout << "***********************************" << std::endl;
      assignPhysicalLocations(AbstractSubtree);
      assignClusterIdToTree(AbstractSubtree, subtreeRoot->cluster_id);
      depthFirstCapacitance(AbstractSubtree);
      depthFirstDelay(AbstractSubtree, tsvUnits.resistance);
      Node *zeroSkewSubtree = zeroSkewTree(AbstractSubtree);
      zeroSkewSubtree->node_type = "MIV";
      depthFirstCapacitance(zeroSkewSubtree);
      depthFirstDelay(zeroSkewSubtree, tsvUnits.resistance);

      // Store the subtree information in map
      Sink rootSink;
      rootSink.x = zeroSkewSubtree->x;
      rootSink.y = zeroSkewSubtree->y;
      rootSink.z = z;
      rootSink.delay = zeroSkewSubtree->elmoreDelay;
      rootSink.cluster_id = zeroSkewSubtree->cluster_id;
      rootSink.sink_type = "MIV";
      tierMIVSinks[z].push_back(rootSink);
      printTree(zeroSkewSubtree);

      int subtreeZsmWireLength =
          calculateZeroSkewTreeWirelength(AbstractSubtree);
      cout << "~~~Zero Skew Tree Wirelength for cluster "
           << subtreeRoot->cluster_id << ": " << subtreeZsmWireLength << endl;
      std::cout << "---------------------------------" << std::endl;
      tierZsmSum += subtreeZsmWireLength;
      cout << "subtreeZsmSum for z " << z << "=" << tierZsmSum << endl;

      // Export each subtree to the tier-specific file in append mode
      exportPointsAndLines(zeroSkewSubtree, tierFilename);
      cout << "Exported subtree for cluster " << subtreeRoot->cluster_id
           << " to tier " << z << " file" << endl;
      deleteTree(zeroSkewSubtree); // Clean up after exporting
    }

    cout << "Completed exporting all subtrees for tier " << z << " to "
         << tierFilename << endl;

    idealSum += wireLength;
    subtreeTotalSum += tierZsmSum;
    deleteTree(root); // Clean up after exporting (Might not be right)
  }
  // To print/access the information:
  for (const auto &pair : tierMIVSinks) {
    cout << "Tier " << pair.first << " subtree roots:" << endl;
    for (const auto &sink : pair.second) {
      cout << "Sink type: " << sink.sink_type << "Cluster " << sink.cluster_id
           << " at (" << sink.x << "," << sink.y << ") with delay "
           << sink.delay << endl;
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  // Calculate runtime
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  // Cout to logfile
  cout << "Ideal Wirelength Sum = " << idealSum << endl;
  cout << "Cluster ZST total = " << subtreeTotalSum << endl;
  std::cout << "Execution time: " << duration.count() << " microseconds"
            << std::endl;
  // Cout to console
  std::cout.rdbuf(coutBuf);
  cout << "Ideal Wirelength Sum = " << idealSum << endl;
  cout << "Cluster ZST total = " << subtreeTotalSum << endl;
  std::cout << "Execution time: " << duration.count() << " microseconds"
            << std::endl;
  return 0;
}
