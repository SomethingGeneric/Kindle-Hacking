#!/bin/bash -e
#
# Build KOReader snapshots
#
# $Id: koreader-build.sh 17014 2020-04-04 17:47:10Z NiLuJe $
#
##

## NOTE: cf. https://github.com/koreader/koreader-base/wiki/Remote-debugging-with-gdbserver for debugging hints.. Addendum:
#	Disable stripping by passing STRIP=true to make, exporting STRIP=true in your env
#	Kill some stray hardcoded -O3 in base...
#		sed -e 's/ -O3//g' -i Makefile.third thirdparty/libk2pdfopt/CMakeLists.txt
#
#	Pass KODEBUG=1 to make
##

## Remember where we are...
SCRIPTS_BASE_DIR="$(readlink -f "${BASH_SOURCE%/*}")"
## And this is where we'll build KOReader
KO_BUILD_DIR="${HOME}/Kindle/CrossTool/Build_KO"
## And where our packages will end up
KO_WEBROOT="${HOME}/files/niluje/kopub"
mkdir -p "${KO_BUILD_DIR}"
cd "${KO_BUILD_DIR}"

## Do we want to move packages to the webroot?
if [[ "${1}" == "w" ]] ; then
	DO_MOVE_TO_WEBROOT="true"
fi

## Do we want to build everything?
if [[ "${2}" == "a" ]] ; then
	BUILD_ALL_TARGETS="true"
fi

## We don't have and don't want to use CCACHE
export USE_NO_CCACHE=1
## Verbose builds
export VERBOSE=1
export V=1

## Clone & fetch Koreader (cf. https://github.com/koreader/koreader-misc/blob/master/koreader-nightlybuild/koreader-nightlybuild.sh)
koreader_repo="https://github.com/koreader/koreader.git"

echo "* Cloning KOReader . . ."
git clone ${koreader_repo} && cd koreader || cd koreader && git fetch origin
git reset --hard origin/master
echo "* Fetching third-party stuff . . ."
./kodev fetch-thirdparty

## Now we need to apply our minor Makefile tweaks...
echo "* Applying Makefile tweaks . . ."
cd base
git checkout -- Makefile.defs
patch -p1 < "${SCRIPTS_BASE_DIR}/koreader-makefile-tweaks.patch"
patch -p1 < "${SCRIPTS_BASE_DIR}/koreader-luarocks-3.patch"
# Use our own toolchain file, because it points to the GCC wrappers of AR & friends, for LTO
cp -av "${SCRIPTS_BASE_DIR}/CMakeCross.txt" thirdparty/cmake_modules/CMakeCross.cmake
# Neuter pkg-config, we specifically don't want to pull anything from my own tree for KOReader...
sed -e 's%set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_FIND_ROOT_PATH}/lib/pkgconfig/)%#set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_FIND_ROOT_PATH}/lib/pkgconfig/)%' -i thirdparty/cmake_modules/CMakeCross.cmake
cd -

## And we can start building the corresponding package for each of our TCs...
# Kobo
echo "* Building KOReader for the Kobo . . ."
source "${SCRIPTS_BASE_DIR}/x-compile.sh" kobo env ko
rm -fv koreader-kobo-${CHOST}-*.zip
./kodev release kobo
if [[ "${DO_MOVE_TO_WEBROOT}" == "true" ]] ; then
	cp -av koreader-kobo-${CHOST}-*.zip "${KO_WEBROOT}/"
fi
echo "* Done :)"

if [[ "${BUILD_ALL_TARGETS}" == "all" ]] ; then
	# K5
	echo "* Building KOReader for the K5 . . ."
	source "${SCRIPTS_BASE_DIR}/x-compile.sh" k5 env ko
	rm -fv koreader-kindle-${CHOST}-*.zip
	./kodev release kindle5
	if [[ "${DO_MOVE_TO_WEBROOT}" == "true" ]] ; then
		cp -av koreader-kindle-${CHOST}-*.zip "${KO_WEBROOT}/"
	fi
	echo "* Done :)"
	# PW2
	echo "* Building KOReader for the PW2 . . ."
	source "${SCRIPTS_BASE_DIR}/x-compile.sh" pw2 env ko
	# Enable extra compat flags if we're using a Glibc 2.19 TC...
	if [[ "${KINDLE_TC_IS_GLIBC_219}" == "true" ]] ; then
		sed -e 's/-D_GLIBCXX_USE_CXX11_ABI=0/-D_GLIBCXX_USE_CXX11_ABI=0 -fno-finite-math-only/' -i base/Makefile.defs
	fi
	rm -fv koreader-kindlepw2-${CHOST}-*.zip
	./kodev release kindlepw2
	if [[ "${DO_MOVE_TO_WEBROOT}" == "true" ]] ; then
		cp -av koreader-kindle-${CHOST}-*.zip "${KO_WEBROOT}/"
	fi
	# And restore the flags once we're done :)
	if [[ "${KINDLE_TC_IS_GLIBC_219}" == "true" ]] ; then
		sed -e 's/-D_GLIBCXX_USE_CXX11_ABI=0 -fno-finite-math-only/-D_GLIBCXX_USE_CXX11_ABI=0/' -i base/Makefile.defs
	fi
	echo "* Done :)"
	# K3
	echo "* Building KOReader for the K3 . . ."
	source "${SCRIPTS_BASE_DIR}/x-compile.sh" k3 env ko
	rm -fv koreader-kindle-legacy-${CHOST}-*.zip
	./kodev release kindle-legacy
	if [[ "${DO_MOVE_TO_WEBROOT}" == "true" ]] ; then
		cp -av koreader-kindle-${CHOST}-*.zip "${KO_WEBROOT}/"
	fi
	echo "* Done :)"
fi

# Whee!
echo "* All done! :)"
