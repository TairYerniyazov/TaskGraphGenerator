# Task Graph Generator
Generator aiming to build task graphs for **embedded systems**. It can also compute the maximum amount of time required to finish all tasks (by identifying the critical path).

**Compiling the program (Linux):**
```console
$ g++ -std=c++14 GraphGenerator.cpp GraphAnalyser.cpp main.cpp -o TaskGenerator
```

**Running the program:**
```console
$ ./TaskGenerator
```

**Sneak peek of the CLI:**
```console
Choose an action:
  1. Generate a task graph (g)
  2. Compute time (t)
  3. Exit (e)

>>> g

Number of tasks: 300
Number of PPs: 3
Number of HCs: 4
Number of CLs: 3
Zero edges (y/else): y
Output file name: test_graph.dat

Generating a task graph...
The generated task graph has been successfully saved in test_graph.dat
```