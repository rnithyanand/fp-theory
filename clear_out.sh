#!/bin/bash
clear

echo "Clearing crumbs from last run"
#rm -r 80*
rm core
rm *.txt
rm *.out
echo "Remaining files: "
ls
