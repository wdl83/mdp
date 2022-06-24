#! /bin/bash

git clone --recurse-submodules https://github.com/wdl83/mdp
cd mdp
RELEASE=1 OBJ_DIR=$HOME/dst make
