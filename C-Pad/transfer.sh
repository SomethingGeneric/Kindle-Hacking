#!/bin/bash

# read -p "Kindle mount point: " mnt

mnt=/media/matt/Kindle
echo "Path: $mnt"

cp cpad.sh $mnt/cpad
mkdir -p $mnt/cpadf/bin
rm $mnt/cpadf/bin/*

for file in $(find scripts/); do echo "$file"; cp $file $mnt/cpadf/bin/.; done


echo "Done"
echo "Exit and re-start cpad if you made changes to it. If you changed a command then you should be fine"