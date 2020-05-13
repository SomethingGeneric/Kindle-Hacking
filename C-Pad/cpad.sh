#!/bin/sh

clear

echo "CPad Kindle Launcher"
echo "--------------------"

echo "Commands:"
sh cpadf/bin/listc

while true
do


printf "> "
read inp

if [ $inp = "exit" ]
then
    exit
else
    sh cpadf/bin/$inp
fi

done