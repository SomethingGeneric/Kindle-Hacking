#!/bin/bash -ex
#
# Small extension to x-compile.sh to build & package git
#
# emerge -1 -f dev-vcs/git
#
# $Id: x-git-build.sh 17176 2020-05-01 21:45:44Z NiLuJe $
#
# kate: syntax bash;
#
##

# Need fancy functions...
source ./x-compile.sh kobo env

cd "${TC_BUILD_DIR}"
# Fake an USBNet-shaped staging tree
GIT_STAGING="${TC_BUILD_DIR}/GIT_STAGING"
mkdir -p ${GIT_STAGING}/usbnet/{lib,libexec/git-core,bin,share}
mkdir -p ${GIT_STAGING}/usr/{lib,libexec/git-core,bin}

## git
GIT_VERSION="2.26.2"
echo "* Building git . . ."
echo ""
tar -xvJf /usr/portage/distfiles/git-${GIT_VERSION}.tar.xz
cd git-${GIT_VERSION}
update_title_info
# Gentoo Patches...
patch -p1 < /usr/portage/dev-vcs/git/files/git-2.22.0_rc0-optional-cvs.patch
patch -p1 < /usr/portage/dev-vcs/git/files/git-2.2.0-svn-fe-linking.patch
patch -p1 < /usr/portage/dev-vcs/git/files/git-2.21.0-quiet-submodules-testcase.patch
autoreconf -fi
# Setup our rpath...
export LDFLAGS="${BASE_LDFLAGS} -Wl,-rpath=${DEVICE_USERSTORE}/usbnet/lib"
sed -i -e '/\/usr\/local/s/BASIC_/#BASIC_/' Makefile
export EXTLIBS="-lpcre2-8"
sed -i \
	-e 's:^\(CFLAGS[[:space:]]*=\).*$:\1 $(OPTCFLAGS) -Wall:' \
	-e 's:^\(LDFLAGS[[:space:]]*=\).*$:\1 $(OPTLDFLAGS):' \
	-e 's:^\(CC[[:space:]]* =\).*$:\1$(OPTCC):' \
	-e 's:^\(AR[[:space:]]* =\).*$:\1$(OPTAR):' \
	Makefile contrib/svn-fe/Makefile
sed -r -i 's/DOCBOOK2X_TEXI[[:space:]]*=[[:space:]]*docbook2x-texi/DOCBOOK2X_TEXI = docbook2texi.pl/' \
		Documentation/Makefile
# Hard-code our own DEFAULT_GIT_TEMPLATE_DIR
sed -e 's:^\(template_dir_SQ[[:space:]]* =\).*$:\1 /mnt/onboard/.niluje/usbnet/share/git-core/templates:' -i Makefile
make ${JOBSFLAGS} ASCIIDOC_NO_ROFF=YesPlease NO_CVS=YesPlease NO_PERL=YesPlease NO_PYTHON=YesPlease NO_SVN_TESTS=YesPlease NO_TCLTK=YesPlease BLK_SHA1=YesPlease PPC_SHA1=YesPlease NO_FINK=YesPlease NO_DARWIN_PORTS=YesPlease INSTALL=install TAR=tar SHELL_PATH=/bin/zsh SANE_TOOL_PATH= OLD_ICONV= NO_EXTERNAL_GREP= USE_LIBPCRE2=YesPlease prefix=${TC_BUILD_DIR} sysconfdir="${TC_BUILD_DIR}/etc" OPTAR="${CROSS_TC}-gcc-ar" OPTCC="${CROSS_TC}-gcc" OPTCFLAGS="${CFLAGS}" OPTLDFLAGS="${LDFLAGS}" NO_GETTEXT=YesPlease NO_SYMLINK_HEAD=YesPlease NO_FAST_WORKING_DIRECTORY=UnfortunatelyYes NO_IPV6=YesPlease NO_ICONV=YesPlease INSTALL_SYMLINKS=YesPlease DEFAULT_EDITOR=nano DEFAULT_PAGER="${DEVICE_USERSTORE}/usbnet/bin/less" HOST_CPU="armv7l" RUNTIME_PREFIX=YesPlease V=1
make ASCIIDOC_NO_ROFF=YesPlease NO_CVS=YesPlease NO_PERL=YesPlease NO_PYTHON=YesPlease NO_SVN_TESTS=YesPlease NO_TCLTK=YesPlease BLK_SHA1=YesPlease PPC_SHA1=YesPlease NO_FINK=YesPlease NO_DARWIN_PORTS=YesPlease INSTALL=install TAR=tar SHELL_PATH=/bin/zsh SANE_TOOL_PATH= OLD_ICONV= NO_EXTERNAL_GREP= USE_LIBPCRE2=YesPlease prefix=${TC_BUILD_DIR} sysconfdir="${TC_BUILD_DIR}/etc" OPTAR="${CROSS_TC}-gcc-ar" OPTCC="${CROSS_TC}-gcc" OPTCFLAGS="${CFLAGS}" OPTLDFLAGS="${LDFLAGS}" NO_GETTEXT=YesPlease NO_SYMLINK_HEAD=YesPlease NO_FAST_WORKING_DIRECTORY=UnfortunatelyYes NO_IPV6=YesPlease NO_ICONV=YesPlease INSTALL_SYMLINKS=YesPlease DEFAULT_EDITOR=nano DEFAULT_PAGER="${DEVICE_USERSTORE}/usbnet/bin/less" HOST_CPU="armv7l" RUNTIME_PREFIX=YesPlease V=1 install
unset EXTLIBS
export LDFLAGS="${BASE_LDFLAGS}"
cp -av ../libexec/git-core ${GIT_STAGING}/usr/libexec/.
for my_bin in ${GIT_STAGING}/usr/libexec/git-core/git* ; do
	[[ -f "${my_bin}" ]] && [[ -x "${my_bin}" ]] && [[ "$(file "${my_bin}")" == *ELF*ARM*executable* ]] && ${CROSS_TC}-strip --strip-unneeded "${my_bin}"
done
cp -av ../bin/git* ${GIT_STAGING}/usr/bin/.
for my_bin in ${GIT_STAGING}/usr/bin/git* ; do
	[[ -f "${my_bin}" ]] && [[ -x "${my_bin}" ]] && [[ "$(file "${my_bin}")" == *ELF*ARM*executable* ]] && ${CROSS_TC}-strip --strip-unneeded "${my_bin}"
done
cp -av ../share/git-core ${GIT_STAGING}/usbnet/share/.
cp -av COPYING ${GIT_STAGING}/usbnet/share/git-core/.

## Package it for unpack in / (i.e., a KoboRoot)
cd ..
# Handle symlinks
cd GIT_STAGING
mkdir -p mnt/onboard/.niluje
mv -v usbnet mnt/onboard/.niluje
# Relocate every stock symlink to a partition that actually handles symlinks!
# There's one broken symlink in 2.26.1...
if [[ -L "usr/libexec/git-core/git-remote-https git-remote-ftp git-remote-ftps" ]] ; then
	rm -v "usr/libexec/git-core/git-remote-https git-remote-ftp git-remote-ftps"
	rm -v "usr/libexec/git-core/git-remote-ftp"
	rm -v "usr/libexec/git-core/git-remote-ftps"
	ln -sf git-remote-http usr/libexec/git-core/git-remote-https
	ln -sf git-remote-http usr/libexec/git-core/git-remote-ftp
	ln -sf git-remote-http usr/libexec/git-core/git-remote-ftps
fi
# And package
tar --owner=root --group=root -cvzf ../Kobo-git-${GIT_VERSION}.tar.gz .
