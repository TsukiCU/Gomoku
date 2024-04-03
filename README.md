# Gomoku
This is the project for CSEE 4840, Spring 2024.


### **decision tree with alpha-beta prunning**

**to play with AI**
change makefile to:
```
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp
```

**to test AI functions by letting it play against itself**
```
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp selfplay.cpp
```

### How to run
```
cd alpha-beta
./run.sh
```

### MCTS (Monte Carlo Tree Search) 

switch to mcts branch. 

mcts is under developed, and it is actually not necessary. So let's focus on the important first and possibly some day we'll get back to that.

### References
https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/