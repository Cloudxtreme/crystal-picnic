#!/bin/bash

mkdir $1

cp heightmap $1
cp area $1
cp script.lua $1
cp info $1

mv $1 ~/Baryon/data/areas
