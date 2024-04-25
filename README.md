# Gomoku
This is the project for CSEE 4840, Spring 2024.

This project is to develop a Gomoku game as well as a user-friendly interface and develop nice user interface in conjunction with hardware.

## Main features
- ### Basic Game Logic
- ### AI Algorithm

## How to Run
To test the game logic and AI function, switch to branch tsuki

To play with AI, change Makefile to:
```
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp

# or
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp evp.cpp
```


To play the PVP mode, change Makefile to:
```
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp
```


cd /alpha-beta, run
```
./run.sh
```

And begin to have fun.

<img width="476" alt="image" src="https://github.com/TsukiCU/Gomoku/assets/155032275/b638a8c5-c991-4cd4-8120-b756b60f8494">


### References
https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
