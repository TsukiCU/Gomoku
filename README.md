# Gomoku
This is the project for CSEE 4840, Spring 2024.

## Note :

**The gui.py is broken. No GUI for now. And since we are probably not able to use pygame so meh, forget about it.**

## How to run

tested on Linux, x86.

+ Install SWIG on your machine

**Enter src**
+ `swig -python -c++ gomoku.i`
+ `python3 setup.py build_ext --inplace`
+ `python3 gui.py`
