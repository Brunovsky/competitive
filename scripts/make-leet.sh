#!/bin/bash

read -p "Difficulty? " DIFFICULTY
[[ -z "$DIFFICULTY" ]] && exit 0

COMPETITION="leet/$DIFFICULTY"

read -p "Problem name? " NAME
[[ -z "$NAME" ]] && exit 0

HEADER="LeetCode - $DIFFICULTY - $NAME"
PROBLEM="$NAME"
PROBLEM_FOLDER="problems/$COMPETITION/$PROBLEM"

mkdir -p "$PROBLEM_FOLDER"
cp -RnT "$TEMPLATE" "$PROBLEM_FOLDER"

echo "# $HEADER" > "$PROBLEM_FOLDER/README.md"
ln -s -T "$PWD/hacktools" "$PWD/$PROBLEM_FOLDER/lib"

cd "$PROBLEM_FOLDER"
code .
