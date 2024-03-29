#include <iostream>
#include <string>
#include "GraphProcessingTools.h"

void prompt() {
  std::cout << "\nChoose an action:\n  1. Generate a task graph (g)\n  "
            << "2. Compute time (t)\n  3. Exit (e)\n";
}

int main() { 
  std::string choice{};
  prompt();
  while (std::cin >> choice) {
    if (choice == "g")
        generate();
    if (choice == "t")
        analyse();
    if (choice == "e")
        return 0;
    prompt();
  }
}