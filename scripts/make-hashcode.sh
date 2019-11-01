#!/bin/bash

read -p "Year? " YEAR
[[ -z "$YEAR" ]] && exit 0

COMPETITION="hashcode/$YEAR"

read -p "Round? " ROUND
[[ -z "$ROUND" ]] && exit 0

read -p "Problem name? (verbose) " NAME
[[ -z "$NAME" ]] && exit 0

HEADER="Hashcode $YEAR - $ROUND - $NAME"
PROBLEM="$ROUND"
PROBLEM_FOLDER="problems/$COMPETITION/$PROBLEM"

mkdir -p "$PROBLEM_FOLDER"
cp -RnT "$TEMPLATE" "$PROBLEM_FOLDER"

echo "# $HEADER" > "$PROBLEM_FOLDER/README.md"
ln -s -T "$PWD/hacktools" "$PWD/$PROBLEM_FOLDER/lib"

cd "$PROBLEM_FOLDER"
code .