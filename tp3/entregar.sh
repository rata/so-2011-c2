#!/bin/bash

rm -f tp3.tar.gz

cd src
make clean
cd -

tar czf tp3.tar.gz informe/informe.pdf src/*
