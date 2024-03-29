#include "GraphProcessingTools.h"

#include <iostream>
#include <string>
#include <set>
#include <stdlib.h>
#include <time.h>
#include <functional>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>

double minEdgeWeight = 10;
double maxEdgeWeight = 100;

double minPPCost = 200;
double maxPPCost = 1000;
double minHCCost = 1000;
double maxHCCost = 3000;

double minPPTaskTime = 10;
double maxPPTaskTime = 300;
double minHCTaskTime = 1;
double maxHCTaskTime = 20;

double minPPTaskCost = 10;
double maxPPTaskCost = 100;

double minCLCost = 50;
double maxCLCost = 100;

double minCLBandwidth = 1;
double maxCLBandwidth = 10;

struct Task {
  int id;
  int level;
  std::set<int, std::less<int>> edges;
  Task(int i, int l) : id{i}, level{l}, edges{} {}
  ~Task() {}
};

void createEdgeForTask(Task& task, int maxLevel, bool atLeastOne,
                       std::vector<Task>& tasks) {
  // Try to create an edge for each task from tasks
  if (task.level < maxLevel) {
    int chance = std::rand() % 10;
    chance = atLeastOne ? 0 : chance;
    // Connecting to an immediate bottom level task (60% chance)
    if (chance < 6) {
      std::vector<int> bottomTasksID{};
      for (Task bottomTask : tasks)
        if (bottomTask.level == task.level + 1)
          bottomTasksID.push_back(bottomTask.id);
      task.edges.insert(bottomTasksID[rand() % bottomTasksID.size()]);
    } else if (chance < 8) {
      // Connecting to any other bottom level task (20% chance)
      int anyBottomLevel = (rand() % 10 / 10.0) * (maxLevel - task.level);
      anyBottomLevel = anyBottomLevel != 0 ? anyBottomLevel : 1;
      anyBottomLevel += task.level;
      if (anyBottomLevel <= maxLevel) {
        std::vector<int> bottomTasksID{};
        for (Task bottomTask : tasks)
          if (bottomTask.level == anyBottomLevel)
            bottomTasksID.push_back(bottomTask.id);
        task.edges.insert(bottomTasksID[rand() % bottomTasksID.size()]);
      }
    }
    // Making no connection at all
  }
}

void addOneEdgeForTask(std::vector<Task>& tasks, int taskID) {
  // Add one edge from one of the tasks belonging to the immediate upper level
  Task task = tasks[taskID];
  int anyTopLevel = (rand() % 10 / 10.0) * task.level;
  std::vector<int> topTasksID{};
  for (Task topTask : tasks)
    if (topTask.level == anyTopLevel) topTasksID.push_back(topTask.id);
  tasks[topTasksID[rand() % topTasksID.size()]].edges.insert(taskID);
}

int generate() {
  int nTasks, nPPs, nHCs, nCLs;
  bool zeroEdges;
  std::string fileName = "output.txt";

  // 1. Getting the user's input
  std::cout << "Number of tasks: ";
  std::cin >> nTasks;
  std::cout << "Number of PPs: ";
  std::cin >> nPPs;
  std::cout << "Number of HCs: ";
  std::cin >> nHCs;
  std::cout << "Number of CLs: ";
  std::cin >> nCLs;
  std::cout << "Zero edges (y/else): ";
  std::string temp;
  std::cin >> temp;
  zeroEdges = temp.compare("y") ? false : true;
  std::cout << "Output file name: ";
  std::cin >> fileName;
  if (nTasks <= 0 || (nPPs <= 0 && nHCs <= 0) || nCLs <= 0) {
    std::cout << "\n>>> RUN AGAIN: There must be at least 1 task, 1 processing "
                 "element and 1 "
                 "communication link.\n";
    return 0;
  }
  std::cout << "\nGenerating a task graph...\n";
  std::ofstream outputFile;
  outputFile.open(fileName);

  // 2. Creating a list of tasks
  std::vector<Task> tasks;
  for (int i = 0; i < nTasks; ++i) tasks.push_back(Task(i, 0));

  // 3. Assigning levels to the tasks (first 1-10% of all tasks to Level 0)
  std::srand(time(NULL));
  int level = 0;
  double fraction = std::rand() % 10 / 100.0;
  for (int i = 0; i < nTasks;) {
    if (i > 0) fraction = std::rand() % 50 / 100.0;
    int nTasksToAssign = (int)(nTasks * fraction);
    nTasksToAssign = nTasksToAssign != 0 ? nTasksToAssign : 1;
    for (int j = i; j < i + nTasksToAssign && j < nTasks; ++j)
      tasks[j].level = level;
    level++;
    i += nTasksToAssign;
  }

  // 4. Creating edges
  // First path
  int maxLevel = (tasks.end() - 1)->level;
  for (Task& task : tasks) createEdgeForTask(task, maxLevel, true, tasks);
  // Adding additional edges (5% of nTasks/(maxLevel + 1))
  int nEdgePaths = (int)(nTasks / (maxLevel + 1) * 0.05);
  for (int i = 0; i < nEdgePaths; i++)
    for (Task& task : tasks) createEdgeForTask(task, maxLevel, false, tasks);
  // Checking whether all the task has an input edge
  std::set<int, std::less<int>> allTasksIDs{};
  std::set<int, std::less<int>> allConnectedTasks{};
  std::set<int, std::less<int>> toBeConnected{};
  do {
    for (Task& task : tasks) {
      if (task.level != 0) allTasksIDs.insert(task.id);
      for (int edge : task.edges) allConnectedTasks.insert(edge);
    }
    std::set_difference(allTasksIDs.begin(), allTasksIDs.end(),
                        allConnectedTasks.begin(), allConnectedTasks.end(),
                        std::inserter(toBeConnected, toBeConnected.begin()));
    for (auto taskID : toBeConnected) addOneEdgeForTask(tasks, taskID);
  } while (allConnectedTasks.size() != allTasksIDs.size());

  // 5. Generating @tasks table
  outputFile << "@tasks " << nTasks << '\n';
  for (Task task : tasks) {
    outputFile << "T" << task.id << " " << task.edges.size() << " ";
    for (int edge : task.edges) {
      outputFile << edge << "("
                 << (!zeroEdges
                         ? (std::rand() % (int)(maxEdgeWeight - minEdgeWeight) +
                            minEdgeWeight)
                         : 0)
                 << ") ";
    }
    outputFile << '\n';
  }

  // 6. Generating @proc table
  std::vector<std::vector<double>> proc{};
  for (int i = 0; i < nHCs; i++) {
    std::vector<double> resource = {
        std::rand() % (int)(maxHCCost - minHCCost) + minHCCost, 0, 0};
    proc.push_back(resource);
  }
  for (int i = 0; i < nPPs; i++) {
    std::vector<double> resource = {
        std::rand() % (int)(maxPPCost - minPPCost) + minPPCost, 0, 1};
    proc.push_back(resource);
  }
  outputFile << "@proc " << nPPs + nHCs << '\n';
  for (auto resource : proc)
    outputFile << resource[0] << " " << resource[1] << " " << resource[2]
               << "\n";

  // 7. Creating @times table
  std::vector<std::vector<double>> times{};
  for (int i = 0; i < nTasks; i++) {
    std::vector<double> row{};
    for (int j = 0; j < nHCs; j++)
      row.push_back(std::rand() % (int)(maxHCTaskTime - minHCTaskTime) +
                    minHCTaskTime);
    for (int j = 0; j < nPPs; j++)
      row.push_back(std::rand() % (int)(maxPPTaskTime - minPPTaskTime) +
                    minPPTaskTime);
    times.push_back(row);
  }
  outputFile << "@times\n";
  for (auto row : times) {
    for (double column : row) outputFile << column << " ";
    outputFile << '\n';
  }

  // 8. Creating @cost table
  std::vector<std::vector<double>> costs{};
  for (int i = 0; i < nTasks; i++) {
    std::vector<double> row{};
    for (int j = 0; j < nHCs; j++) row.push_back(0);
    for (int j = 0; j < nPPs; j++)
      row.push_back(std::rand() % (int)(maxPPTaskCost - minPPTaskCost) +
                    minPPTaskTime);
    costs.push_back(row);
  }
  outputFile << "@cost\n";
  for (auto row : costs) {
    for (double column : row) outputFile << column << " ";
    outputFile << '\n';
  }

  // 9. Creating @comm table
  std::vector<std::vector<double>> comm{};
  for (int i = 0; i < nCLs; ++i) {
    std::vector<double> row{};
    row.push_back(std::rand() % (int)(maxCLCost - minCLCost) + minCLCost);
    row.push_back(std::rand() % (int)(maxCLBandwidth - minCLBandwidth) +
                  minCLBandwidth);
    for (int j = 0; j < nPPs + nHCs; ++j)
      row.push_back(std::rand() % 2);
    comm.push_back(row);
  }
  for (int i = 0; i < nPPs + nHCs; ++i) {
    bool noAccess = true;
    for (auto row : comm)
      if (row[2 + i] == 1.0) noAccess = false;
    if (noAccess) comm[std::rand() % comm.size()][2 + i] = 1.0;
  }
  for (auto& row : comm) {
    int nConnectedPE = 0;
    while (nConnectedPE < 2) {
      nConnectedPE = 0;
      for (int i = 0; i < nPPs + nHCs; ++i)
        if (row[2 + i] == 1.0) nConnectedPE += row[2 + i];
      if (nConnectedPE == 1 && (nPPs + nHCs) == 1) nConnectedPE = 2;
      if (nConnectedPE < 2) row[2 + std::rand() % (row.size() - 2)] = 1.0;
    }
  }
  outputFile << "@comm " << nCLs << '\n';
  for (int i = 0; i < nCLs; ++i) {
    outputFile << "CHAN" << i << " " << comm[i][0] << " " << comm[i][1] << " ";
    for (int j = 0; j < nPPs + nHCs; ++j)
      outputFile << comm[i][2 + j] << " ";
    outputFile << '\n';
  }

  std::cout << "The generated task graph has been successfully saved in "
            << fileName << '\n';
  outputFile.close();
   //Debugging
   /*std::cout << "Task ID | Level | # Edges | Edges (other task IDs)\n";
   for (Task task : tasks) {
     std::cout << task.id << " | " << task.level << " | " << task.edges.size()
               << " | ";
     if (task.edges.size() != 0)
       for (auto it = task.edges.begin(); it != task.edges.end(); ++it)
         std::cout << *it << " ";
     std::cout << '\n';
   }*/
  return 0;
}
