#!/bin/bash -e

# Quickly launch a full set of builds for my TCs
for my_tc in K3 K5 PW2 KOBO ; do
	# Setup the x-compile env for this TC
	source ../x-compile.sh ${my_tc} env

	# And... GO!
	echo "* Launching ${KINDLE_TC} build . . ."

	mkdir -p ${KINDLE_TC}
	make clean
	make static

	cp -av fbgrab ${KINDLE_TC}/fbgrab
	make clean
done
