# Gomoku
This is the project for CSEE 4840, Spring 2024.

## How to run

tested on Linux, x86.

+ Install SWIG on your machine

**Enter src**
+ `swig -python -c++ gomoku.i`
+ `python3 setup.py build_ext --inplace`
+ `python3 gui.py`
