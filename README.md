# Gomoku
This is the project for CSEE 4840, Spring 2024.

This project is to create a Gomoku game. This includes the basic game logic, AI algorithms, as well as a user-friendly interface that integrates with hardware components.

## How to Run
To simply test the **game logic** and **AI function**, switch to branch ```tsuki```

To play ```in the terminal```

change Makefile to:
```
# Player vs AI
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp
# or
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp evp.cpp

# or
# Player vs Player
SOURCES = ../src/gomoku.cpp ../src/players.cpp gomokuAI.cpp pve.cpp
```

To play ```with dummy GUI```, use the default Makefile.

cd /alpha-beta, run
```
./run.sh
```

And begin to have fun.

### In Terminal

<img width="480" alt="image" src="https://github.com/TsukiCU/Gomoku/assets/155032275/7732d2d3-911f-48c3-92d0-157cd9324475">

Note the '@' means the last move (played by any player).

### With GUI

<img width="608" alt="image" src="https://github.com/TsukiCU/Gomoku/assets/155032275/09a26fdf-5000-4bb5-ac86-1e69a6803f04">


### References
https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
