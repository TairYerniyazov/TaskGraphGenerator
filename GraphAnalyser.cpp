#include "GraphProcessingTools.h"
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <cstddef>
#include <vector>
#include <limits>
#include <sstream>
#include <algorithm>
#include <stdio.h>

void input_prompt() {
  std::cout << "Input file name: ";
}

// Recurrsive function computing the time paths
int longestPath(int taskID, std::vector<std::vector<std::vector<int>>>& tasks,
                std::vector<int>& bestTime) {
  auto task = tasks[taskID];
  if (task.size() == 0) 
    return bestTime[taskID]; 
  std::vector<int> times{};
  for (auto nextTask : task) {
    times.push_back(longestPath(nextTask[0], tasks, bestTime) + bestTime[taskID]); 
  }
  return *std::max_element(times.begin(), times.end());
}

void analyse() {
  std::string fileName;
  input_prompt();  
  while (!(std::cin >> fileName)) {
    input_prompt();
  }
  std::ifstream inputFile;
  inputFile.open(fileName);
  if (inputFile.is_open()) {
    // 1. Data Parsing
    std::cout << "\nLoading a task graph...\n";
    std::cout << "Allocating resources (minimum execution time)...\n";
    std::string line;
    int nTasks{};
    int nPEs{};
    std::vector<std::vector<std::vector<int>>> tasks{}; // [task][next_task][next_task_index, edge]
    std::vector<std::vector<int>> channels{};           // [channel][value]
    std::vector<std::vector<int>> proc_PEs{};           // [task][cost, access, type, index]
    std::vector<int> proc_HCs_indices{};                // [index_in_proc_PEs]
    std::vector<int> proc_PPs_indices{};                // [index_in_proc_PEs]
    std::vector<std::vector<int>> times{};              // [task][time_for_PE]
    while (std::getline(inputFile, line)) {
      if (line.find("@tasks") != std::string::npos) {
        nTasks = std::stoi(line.substr(7));
        for (int i = 0; i < nTasks; ++i) {
          std::getline(inputFile, line);
          line = line.substr(1);
          std::stringstream ss(line);
          std::string dev_null;
          int n_tasks;
          ss >> dev_null;
          ss >> n_tasks;
          std::vector<std::vector<int>> next_tasks{};
          for (int j = 0; j < n_tasks; j++) {
            int task_index;
            int edge;
            std::string temp;
            ss >> temp;
            sscanf(temp.c_str(), "%d(%d)", &task_index, &edge);
            std::vector<int> values{}; 
            values.push_back(task_index);
            values.push_back(edge);
            next_tasks.push_back(values);
           }
          tasks.push_back(next_tasks);
        }
      }
      if (line.find("@proc") != std::string::npos) {
        nPEs = std::stoi(line.substr(6));
        for (int i = 0; i < nPEs; ++i) {
          int cost, access, type;
          std::getline(inputFile, line);
          std::stringstream ss(line);
          ss >> cost >> access >> type;
          std::vector<int> temp{};
          temp.push_back(cost);
          temp.push_back(access);
          temp.push_back(type);
          temp.push_back(i);
          proc_PEs.push_back(temp);;
          if (type == 0)
            proc_HCs_indices.push_back(i);
          else
            proc_PPs_indices.push_back(i);
        }
      }
      if (line.find("@times") != std::string::npos) {
        for (int t = 0; t < nTasks; ++t) {
          std::getline(inputFile, line);
          std::stringstream ss(line);
          std::vector<int> time{};
          for (int i = 0; i < nPEs; ++i) {
            std::getline(ss, line, ' ');
            time.push_back(std::stoi(line));
          }
          times.push_back(time);
        }
      }
      if (line.find("@comm") != std::string::npos) {
        int nChannels = std::stoi(line.substr(6));
        std::string dev_null;
        for (int i = 0; i < nChannels; ++i) {
          std::getline(inputFile, line);
          std::stringstream ss(line);
          ss >> dev_null;
          std::vector<int> temp_v{};
          for (int j = 0; j < nPEs + 2; ++j) {
            int temp;
            ss >> temp;
            temp_v.push_back(temp);
          }
          channels.push_back(temp_v);
        }
      }
    }
    inputFile.close();
    // 3. Resource Allocation    
    std::vector<int> bestTime{};
    for (int i = 0; i < nTasks; ++i) {
      auto min_time = std::min_element(times[i].begin(), times[i].end());
      auto PE_index = std::distance(times[i].begin(), min_time);
      bestTime.push_back(*min_time);
      std::string type = proc_PEs[PE_index][2] == 0 ? "HC" : "PP";
      std::cout << "T" << i << " -> " << type;
      int unit_type_index = 0;
      if (type == "HC") {
        auto found_index = std::find(proc_HCs_indices.begin(),
                                    proc_HCs_indices.end(),
                                    PE_index);
        unit_type_index = std::distance(proc_HCs_indices.begin(), found_index);
      } else {
        auto found_index = std::find(proc_PPs_indices.begin(), 
                                    proc_PPs_indices.end(),
                                    PE_index);
        unit_type_index = std::distance(proc_PPs_indices.begin(), found_index);
      }
      unit_type_index += 1;
      std::cout << unit_type_index << '\n';
    }
    // 4. Computing Time
    // Finding root tasks
    std::vector<int> taskCounter{};
    for (int i = 0; i < nTasks; ++i) 
      taskCounter.push_back(0);
    for (auto task : tasks)
      for (auto nextTask : task) 
        taskCounter[nextTask[0]]++;
    // Computing the overall time for each paths beggining at any of the root level task
    std::vector<int> longestTime{};
    for (int t = 0; t < nTasks; ++t) {
      if (taskCounter[t] == 0) {
        int time = longestPath(t, tasks, bestTime); 
        longestTime.push_back(time); 
      }
    } 
    std::cout << "Critical path total time: "
              << *std::max_element(longestTime.begin(), longestTime.end())
              << '\n';
  } else {
    std::cout << "The file cannot be opened.\n";
  }
}
