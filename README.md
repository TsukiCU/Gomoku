# Gomoku
This is the project for CSEE 4840, Spring 2024.

The project is to develop a Gomoku Game, as well as user-friendly interfaces, and display it combining with the hardware.

## Main features
- ### Basic Game Logic
- ### AI Algorithm

## How to Run
To test the software function, switch to branch tsuki

If you want to play with AI, change Makefile to:
```
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp

# or
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp evp.cpp
```


If you want to play with PVP mode, change Makefile to:
```
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp
```


Then enter /alpha-beta, run
```
./run.sh
```

And begin to have fun.

<img width="476" alt="image" src="https://github.com/TsukiCU/Gomoku/assets/155032275/b638a8c5-c991-4cd4-8120-b756b60f8494">


### References
https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
