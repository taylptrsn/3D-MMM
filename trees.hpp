
#pragma once
#include "structures.hpp"
#include "utilities.hpp"
#include <vector>
using namespace std;

int numSinks;
int nodeID = 0;         // Global variable to keep track of the next node ID
int zCutCount = 0;      // Keeps track of the number of Z-cuts performed
int ZeroSkewMerges = 0; // Keeps track of the number of ZSMs performed

// Function to perform Z-cut
void Zcut(const vector<Sink> &S, const ClockSource &Zs, vector<Sink> &St,
          vector<Sink> &Sb) {
  cout << endl;
  cout << "Z-cut!" << endl;
  int Zmin = getMinZ(S); // Set your Zmin value
  int Zmax = getMaxZ(S); // Set your Zmax value
  // cout<<"zmin:  " << Zmin << endl;
  // cout<<"zmax:  " << Zmax << endl;
  //  If Zs is less than or equal to Zmin
  if (Zs.z <= Zmin) {
    cout << "Zs is less than or equal to Zmin" << endl;
    // if (!S.empty()) {
    for (const auto &sink : S) {
      if (sink.z == Zmin) {
        Sb.push_back(sink);
      } else {
        St.push_back(sink);
      }
    }
    //}
  }
  // If Zs is greater than or equal to Zmax
  else if (Zs.z >= Zmax) {
    cout << "If Zs is greater than or equal to Zmax" << endl;
    // if (!S.empty()) {
    for (const auto &sink : S) {
      if (sink.z == Zmax) {
        St.push_back(sink);
      } else {
        Sb.push_back(sink);
      }
    }
    //}
  }
  // If Zs is between Zmin and Zmax
  else {
    cout << "If Zs is between Zmin and Zmax" << endl;
    for (const auto &sink : S) {
      if (sink.z >= Zs.z) {
        St.push_back(sink); // S top
      } else {
        Sb.push_back(sink); // S bottom
      }
    }
  }
  zCutCount++;
}

// Abstract Tree Generation
Node *AbsTreeGen3D(const vector<Sink> &S, int B) {
  // cout << "B " << B << endl;
  int B1 = 0;
  int B2 = 0;
  int deltaX = getMaxX(S) - getMinX(S);
  int deltaY = getMaxY(S) - getMinY(S);
  int deltaZ = getMaxZ(S) - getMinZ(S);
  vector<Sink> St, Sb;
  ClockSource Zs = clockSource;
  if (S.size() == 1) {
    // Base case: if die span = 1, 2d tree
    return new Node(S, "Gray", 0.0, 0.0, false,
                    nodeID++); // Assign a unique id to the node

    //} else if (deltaZ > 1 && B==1) {
  } else if (deltaX == 0 && deltaY == 0 && deltaZ >= 1) {
    // New condition for the edge case where all x and y are the same, but z
    // differs
    cout << "Special case: All x and y coordinates are the same. Performing "
            "Z-cut based on z coordinates."
         << endl;
    Zcut(S, Zs, St, Sb);
  } else if (deltaZ >= 1 && B == 1) {
    Zcut(S, Zs, St, Sb);
    //  Display St and Sb
    cout << "St (Top most die group):" << endl;
    for (const auto &sink : St) {
      cout << "(" << sink.x << "," << sink.y << "," << sink.z
           << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance << " fF"
           << endl;
    }
    cout << "Sb (Bottom most die group):" << endl;
    for (const auto &sink : Sb) {
      cout << "(" << sink.x << "," << sink.y << "," << sink.z
           << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance << " fF"
           << endl;
    }
    B1 = B2 = 1;

  } else {
    int medianX = calculateMedianX(S);
    int medianY = calculateMedianY(S);
    for (const auto &sink : S) {
      if (deltaX > deltaY) {
        if (sink.x < medianX) {
          St.push_back(sink); // top subset
        } else {
          Sb.push_back(sink); // bottom subset
        }
      } else {
        if (sink.y < medianY) {
          St.push_back(sink); // top subset
        } else {
          Sb.push_back(sink); // bottom subset
        }
      }
    }
    B1 = B / 2;
    B2 = B - B1;
  }
  Node *root = new Node(S, "Gray", 0.0, 0.0, false,
                        nodeID++); // Assign a unique id to the node

  root->leftChild = AbsTreeGen3D(St, B1);
  root->rightChild = AbsTreeGen3D(Sb, B2);
  return root;
}

// Function to delete the tree nodes recursively
void deleteTree(Node *node) {
  if (node) {
    deleteTree(node->leftChild);
    deleteTree(node->rightChild);
    delete node;
  }
}

void printTree(Node *node, int level = 0) {
  if (!node) {
    return; // Base case: if the node is null, return.
  }
  string indent = string(level * 8, ' '); // Adjust indentation for each level.
  // Print the left branch (child) first.
  if (node->leftChild) {
    printTree(node->leftChild, level + 1);
  }
  // Print the current node along with its color, memory address, capacitance,
  // and Elmore delay.
  cout << indent << node->node_type << " Node ID: " << node->id
       << " Cluster ID: " << node->cluster_id << " at position: (" << node->x
       << ", " << node->y << ", " << node->z << ") -  Depth " << level
       << " - Color: " << node->color
       << " - Total Capacitance: " << node->capacitance << " fF"
       << " - Elmore Delay: " << node->elmoreDelay << " fs" << endl;

  // If the node is a leaf, also print its sinks.
  if (!node->leftChild && !node->rightChild) {
    for (const auto &sink : node->sinks) {
      cout << indent << "    Sink: (" << sink.x << ", " << sink.y << ", "
           << sink.z << "), Input Capacitance: " << sink.inputCapacitance
           << " fF, Color: " << sink.color << endl;
    }
  }
  cout << endl;
  if (node->rightChild) {
    printTree(node->rightChild, level + 1);
  }
}

void printLeaves(const Node *node) {
  if (node == nullptr) {
    return;
  }
  if (node->leftChild == nullptr && node->rightChild == nullptr) {
    for (const auto &sink : node->sinks) {
      cout << "Color: " << sink.color
           << " - Sink: ("
              "Leaf Nodes(Sinks):  ("
           << sink.x << ", " << sink.y << ", " << sink.z << ")" << endl;
    }
  } else {
    printLeaves(node->leftChild);
    printLeaves(node->rightChild);
  }
}

void colorTree(Node *node, const vector<string> &dieColors,
               const ClockSource &source, int depth = 0) {
  if (node == nullptr) {
    return;
  }
  // Determine the color of the source based on its z-coordinate
  string sourceColor = dieColors[source.z % dieColors.size()];

  if (depth == 0) { // If the node is the root
    node->color = sourceColor;
  } else {
    int Zmin = getMinZ(node->sinks);
    int Zmax = getMaxZ(node->sinks);
    if (Zmin > source.z) {
      // If Zmin is greater than the z-coordinate of the source, set to the
      // color of Zmin
      node->color = dieColors[Zmin % dieColors.size()];
    } else if (Zmax < source.z) {
      // If Zmax is less than the z-coordinate of the source, set to the color
      // of Zmax
      node->color = dieColors[Zmax % dieColors.size()];
    } else {
      // Otherwise, set to the color of the source
      node->color = sourceColor;
    }
  }
  // Assign colors to the sinks within this node
  for (auto &sink : node->sinks) {
    sink.color = node->color;
  }
  colorTree(node->leftChild, dieColors, source, depth + 1);
  colorTree(node->rightChild, dieColors, source, depth + 1);
}

// Helper function to find a node by its ID
Node *findNodeById(Node *node, int id) {
  if (node == nullptr)
    return nullptr; // Base case: node is null
  if (node->id == id)
    return node;

  // Recursively search in the left subtree
  Node *leftResult = findNodeById(node->leftChild, id);
  if (leftResult != nullptr)
    return leftResult; // Found in the left subtree

  // Recursively search in the right subtree
  return findNodeById(node->rightChild,
                      id); // Could return nullptr if not found
}

// Function to search for a node by its children's coordinates with verbose
// output
Node *findNodeByChildren(Node *root, int childX, int childY) {
  if (root == nullptr) {
    cout << "Reached a null node. Returning nullptr." << endl;
    return nullptr;
  }

  cout << "Visiting Node ID: " << root->id << " with coordinates (" << root->x
       << ", " << root->y << ", " << root->z << ")" << endl;

  // Check left child
  if (root->leftChild != nullptr) {
    cout << "Checking left child of Node ID: " << root->id << endl;
    if (root->leftChild->x == childX && root->leftChild->y == childY) {
      cout << "Match found in left child of Node ID: " << root->id << endl;
      return root;
    }
  } else {
    cout << "No left child for Node ID: " << root->id << endl;
  }

  // Check right child
  if (root->rightChild != nullptr) {
    cout << "Checking right child of Node ID: " << root->id << endl;
    if (root->rightChild->x == childX && root->rightChild->y == childY) {
      cout << "Match found in right child of Node ID: " << root->id << endl;
      return root;
    }
  } else {
    cout << "No right child for Node ID: " << root->id << endl;
  }

  // Recursively search in the left subtree
  cout << "Recursively searching left subtree of Node ID: " << root->id << endl;
  Node *foundNode = findNodeByChildren(root->leftChild, childX, childY);
  if (foundNode != nullptr) {
    return foundNode;
  }

  // Recursively search in the right subtree
  cout << "Recursively searching right subtree of Node ID: " << root->id
       << endl;
  return findNodeByChildren(root->rightChild, childX, childY);
}