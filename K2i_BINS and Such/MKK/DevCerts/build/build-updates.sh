#!/bin/bash -e
#
# $Id: build-updates.sh 16157 2019-07-12 01:38:41Z NiLuJe $
#

HACKNAME="mkk"
PKGNAME="${HACKNAME}"
PKGVER="20141129"

# We need kindletool (https://github.com/NiLuJe/KindleTool) in $PATH
if (( $(kindletool version | wc -l) == 1 )) ; then
	HAS_KINDLETOOL="true"
fi

if [[ "${HAS_KINDLETOOL}" != "true" ]] ; then
	echo "You need KindleTool (https://github.com/NiLuJe/KindleTool) to build this package."
	exit 1
fi

## Move stuff around a bit to have everything in the right place in the least amount of steps, and without duplicating stuff in the repo
# Build a list of temp stuff to cleanup later while we're at it...
declare -a cleanup_list
for family in legacy touch ; do
	for setup in install uninstall ; do
		for resource in ../src/${setup}/* ; do
			target_resource="../src/${family}/${setup}/${resource##*/}"

			cleanup_list[${#cleanup_list[*]}]="${target_resource}"
			ln -f ${resource} ${target_resource}
		done
	done
done

# Pickup our common stuff...
if [[ ! -d "../../Common" ]] ; then
        echo "The tree isn't checked out in full, missing the Common directory..."
        exit 1
fi

# LibOTAUtils 5
resource="libotautils5"
family="touch"
for setup in install uninstall ; do
	target_resource="../src/${family}/${setup}/${resource}"

	# Don't add it to the cleanup list so that it ships in the src package...
	#cleanup_list[${#cleanup_list[*]}]="${target_resource}"
	ln -f ../../Common/lib/${resource} ${target_resource}
done

# LibOTAUtils
resource="libotautils"
family="legacy"
for setup in install uninstall ; do
	target_resource="../src/${family}/${setup}/${resource}"

	# Don't add it to the cleanup list so that it ships in the src package...
	#cleanup_list[${#cleanup_list[*]}]="${target_resource}"
	ln -f ../../../Hacks/Common/lib/${resource} ${target_resource}
done


## K5
family="touch"
for setup in install uninstall ; do
	kindletool create ota2 -d kindle5 -C ../src/${family}/${setup} Update_${PKGNAME}-${PKGVER}-k5-ALL_${setup}.bin
done

## K4
family="legacy"
for setup in install uninstall ; do
	kindletool create ota2 -d kindle4 -C ../src/${family}/${setup} Update_${PKGNAME}-${PKGVER}-k4-ALL_${setup}.bin
done

## K2/3
KINDLE_MODELS=( k2 k2i dx dxi dxg k3g k3w k3gb )
# Model -> device code map. People will shout at me if I use bash 4 associative arrays, so fake it with 2 bash arrays
KINDLE_DEVCODES=( B002 B003 B004 B005 B009 B006 B008 B00A )
for (( id = 0; id < ${#KINDLE_MODELS[@]}; id++ )) ; do
	# Prepare our files for this specific kindle model...
	ARCH=${PKGNAME}-${PKGVER}-${KINDLE_MODELS[${id}]}-${KINDLE_DEVCODES[${id}]}

	# Install
	kindletool create ota -d ${KINDLE_MODELS[${id}]} -C ../src/legacy/install  Update_${ARCH}_install.bin
	# Uninstall
	kindletool create ota -d ${KINDLE_MODELS[${id}]} -C ../src/legacy/uninstall Update_${ARCH}_uninstall.bin
done


## Move our updates
mv -f *.bin ../

## Cleanup the temporary stuff :)
for cleanup_item in "${cleanup_list[@]}" ; do
	[[ -f "${cleanup_item}" ]] && rm -f "${cleanup_item}"
done
