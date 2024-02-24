from distutils.core import setup, Extension

gomoku_module = Extension('_gomoku',
                          sources=['gomoku_wrap.cxx', 'gomoku.cpp', 'players.cpp'],
                          extra_compile_args=['-std=c++11'])

setup(name='gomoku',
      version='0.1',
      author='CSEE W4119 Project',
      description='Gomoku basic logic module',
      ext_modules=[gomoku_module],
      py_modules=["gomoku"])
