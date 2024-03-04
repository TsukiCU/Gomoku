from distutils.core import setup, Extension

gomoku_module = Extension('_gomoku',
                          sources=['gomoku_wrap.cxx', 'gomoku.cpp', 'players.cpp', 'gomokuAI.cpp'],
                          extra_compile_args=['-std=c++11'])

setup(name='gomoku',
      version='0.1',
      author='CSEE W4840 Project',
      description='Gomoku',
      ext_modules=[gomoku_module],
      py_modules=["gomoku"])
