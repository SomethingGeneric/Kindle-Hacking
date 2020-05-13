#!/bin/sh -e
#
# Package the ToolChain, patches & Build Script used to build the binaries bundled with my Kindle Hacks for release on MR
#
# $Id: make-packages.sh 16747 2019-12-06 19:07:03Z NiLuJe $
#

##
### Config
#

# We need GNU tar
if [[ "$(uname -s)" == "Darwin" ]] ; then
	TAR_BIN="gtar"
else
	TAR_BIN="tar"
fi
if ! ${TAR_BIN} --version | grep -q "GNU tar" ; then
	echo "You need GNU tar to build this package."
	exit 1
fi
# Setup xz multi-threading...
export XZ_DEFAULTS="-T 0"

# X TC
TC_FULL="Cross ToolChain"
TC_DIR="."
TC_FILE="x-tc"
TC_VER="2019.02.N"

##
### The meat of the thing
### $1: "Hack CFG Tag"
#

package_a_hack()
{
	# Check args
	if [[ -z "$1" ]] ; then
		echo "* We're missing an arg, abort."
		exit 1
	fi

	# Set args
	TAG="$1"
	# And the useful ones
	FULLNAME="${TAG}_FULL"
	DIRNAME="${TAG}_DIR"
	FILENAME="${TAG}_FILE"
	VERSION="${TAG}_VER"

	# Say hello
	echo "* Updating ${!FULLNAME} package (v${!VERSION})..."

	# Remove old version
	rm -f kindle-${!FILENAME}-*.tar.xz
	rm -f kindle-${!FILENAME}-*.tar.gz

	# Make our new package
	cd ${!DIRNAME}

	# Create the VERSION tag file
	SVN_REV="$(svnversion -c . | awk '{print $NF}' FS=':' | tr -d 'P')"
	PKG_DATE="$(date +'%Y-%b-%d @ %H:%M')"
	# Just dump it at the root of the package
	echo "${!VERSION} @ r${SVN_REV} on ${PKG_DATE}" > VERSION

	# Put it in a shiny tarball
	${TAR_BIN} --hard-dereference --exclude-vcs --exclude='*.tar.*' --exclude='fix_ss.sh' --exclude='kindle_backup.sh' --exclude='kindle_restore.sh' --exclude='upload.sh' --exclude='go.sh' --exclude='koreader-mirror.sh' --exclude='koreader-mirror.lftp' --exclude='MR_SNAPSHOTS_THREAD' --exclude='koreader-upload.sh' --exclude='MR_KO_SNAPSHOTS_THREAD' -cvJf kindle-${!FILENAME}-${!VERSION}-r${SVN_REV}.tar.xz *

	# Snapshots package only?
	if [[ "${NO_SNAPSHOTS_ONLY_PACKAGES}" == "true" ]] ; then
		${TAR_BIN} --hard-dereference --exclude-vcs --exclude='*.tar.*' --exclude='fix_ss.sh' --exclude='fix_ss.sh' --exclude='kindle_backup.sh' --exclude='kindle_restore.sh' --exclude='upload.sh' --exclude='go.sh' --exclude='koreader-mirror.sh' --exclude='koreader-mirror.lftp' --exclude='MR_SNAPSHOTS_THREAD' --exclude='koreader-upload.sh' --exclude='MR_KO_SNAPSHOTS_THREAD' -I pigz -cvf kindle-${!FILENAME}-${!VERSION}.tar.gz *
	fi
}

##
### Main
#

# Update the WC, because I'm pretty sure I'm capable of forgetting to do it...
svn update .

# Update the ChangeLogs...
for khack in . ; do
	svn2cl --linelen=80 --break-before-msg=2 -i -a -o ${khack}/ChangeLog ${khack}
done

# And package all the hacks
for khack_tag in TC ; do
	package_a_hack "${khack_tag}"
done
