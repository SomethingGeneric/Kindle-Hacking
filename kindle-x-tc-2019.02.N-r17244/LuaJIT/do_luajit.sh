#!/bin/bash -e
#
# $Id: do_luajit.sh 16605 2019-10-26 14:15:53Z NiLuJe $
#
##

# Start by cloning the repo
git clone git://repo.or.cz/luajit-2.0.git -b v2.1 luajit && cd luajit || cd luajit && git fetch origin
git reset --hard origin/v2.1

# Then we apply a small Makefile patch to make it LTO-friendly...
patch -p1 < ../../koreader-luajit-makefile-tweaks.patch
# And verbose
patch -p1 < ../../koreader-luajit-verbose-build.patch

# Remember our native CFLAGs before they get squished by the TC env...
HOST_CFLAGS="${CFLAGS}"

# Quickly launch a full set of builds for my TCs
for my_tc in K3 K5 PW2 KOBO ; do
	# Setup the x-compile env for this TC
	source ../../x-compile.sh ${my_tc} env bare
	# Setup our rpaths...
	# Because escaping this is hell..
	ORIGIN_RPATH='-Wl,-rpath=\$$ORIGIN/lib'
	export LDFLAGS="${LDFLAGS} ${ORIGIN_RPATH} -Wl,-rpath=/usr/local/fbink/lib -Wl,-rpath=${DEVICE_USERSTORE}/linkfonts/lib -Wl,-rpath=${DEVICE_USERSTORE}/linkss/lib -Wl,-rpath=${DEVICE_USERSTORE}/usbnet/lib -Wl,-rpath=${DEVICE_USERSTORE}/python/lib"

	# And... GO!
	echo "* Launching ${KINDLE_TC} build . . ."
	make HOST_CC="gcc -m32" CFLAGS="" CCOPT="" HOST_CFLAGS="${HOST_CFLAGS}" CROSS="${CROSS_PREFIX}" TARGET_CFLAGS="${RICE_CFLAGS}" clean

	# c.f., http://luajit.org/install.html#cross
	mkdir -p ../${KINDLE_TC}
	make HOST_CC="gcc -m32" CFLAGS="" CCOPT="" HOST_CFLAGS="${HOST_CFLAGS}" CROSS="${CROSS_PREFIX}" TARGET_CFLAGS="${RICE_CFLAGS}" amalg

	# And we're done!
	cp -av src/luajit ../${KINDLE_TC}/luajit
	cp -av src/jit ../${KINDLE_TC}/
done

cd -

# Package it!
rm -f LuaJIT-v2.1-kindle.tar.gz
tar --exclude-vcs -cvzf LuaJIT-v2.1-kindle.tar.gz CREDITS do_luajit.sh K3 K5 PW2
rm -f LuaJIT-v2.1-kobo.tar.gz
tar --exclude-vcs -cvzf LuaJIT-v2.1-kobo.tar.gz CREDITS do_luajit.sh KOBO
