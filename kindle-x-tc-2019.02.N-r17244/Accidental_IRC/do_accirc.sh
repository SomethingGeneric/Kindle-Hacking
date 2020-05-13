#!/bin/bash -e

# Start by cloning the repo
git clone git@github.com:neutrak/accirc.git accirc && cd accirc || cd accirc && git fetch origin
git reset --hard origin/master

# Quickly launch a full set of builds for my TCs
for my_tc in K3 K5 PW2 KOBO ; do
	# Setup the x-compile env for this TC
	source ../../x-compile.sh ${my_tc} env

	# And... GO!
	echo "* Launching ${KINDLE_TC} build . . ."

	# We link statically against OpenSSL & ncurses, to avoid ABI/API mismatches... And to zlib too, because the bundled one often sucks, performance wise.
	mkdir -p ../${KINDLE_TC}
	echo "${CROSS_TC}-gcc accidental_irc.c ${CPPFLAGS} -I${TC_BUILD_DIR}/include/ncursesw -D_OPENSSL -Wall ${CFLAGS} ${LDFLAGS} -o ../${KINDLE_TC}/accirc -l:libncursesw.a -l:libssl.a -l:libcrypto.a -l:libz.a"
	${CROSS_TC}-gcc accidental_irc.c ${CPPFLAGS} -I${TC_BUILD_DIR}/include/ncursesw -D_OPENSSL -Wall ${CFLAGS} ${LDFLAGS} -o ../${KINDLE_TC}/accirc -l:libncursesw.a -l:libssl.a -l:libcrypto.a -l:libz.a
	echo "${CROSS_TC}-strip --strip-unneeded ../${KINDLE_TC}/accirc"
	${CROSS_TC}-strip --strip-unneeded ../${KINDLE_TC}/accirc
done

cd -
