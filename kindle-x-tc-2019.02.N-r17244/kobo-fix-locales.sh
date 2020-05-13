#!/bin/sh
#
# Build minimal UTF-8 locales, because Kobo ships with none -_-"
# c.f., http://www.linuxfromscratch.org/lfs/view/development/chapter06/glibc.html
#
##

# Choose your target!
if (( $# != 1 )) ; then
	echo "Need to pass a hostname!"
	exit 1
fi
REMOTE="${1}"

# Assumes you've got a TC matching the target glibc installed
# c.f., https://github.com/koreader/koxtoolchain
TARGET_TC="arm-kobomk7-linux-gnueabihf"

# On the Kobo
ssh root@${REMOTE} mkdir -p /usr/share/i18n/locales /usr/share/i18n/charmaps /usr/lib/locale

# Charmaps
for my_charmap in ISO-8859-1.gz UTF-8.gz ; do
	scp ${HOME}/x-tools/${TARGET_TC}/${TARGET_TC}/sysroot/usr/share/i18n/charmaps/${my_charmap} root@${REMOTE}:/usr/share/i18n/charmaps/
done

# Locales
for my_locale in POSIX en_GB en_US i18n iso14651_t1 iso14651_t1_common translit_circle translit_cjk_compat translit_combining translit_compat translit_font translit_fraction translit_narrow translit_neutral translit_small translit_wide ; do
	scp ${HOME}/x-tools/${TARGET_TC}/${TARGET_TC}/sysroot/usr/share/i18n/locales/${my_locale} root@${REMOTE}:/usr/share/i18n/locales/
done

# localedef
	scp ${HOME}/x-tools/${TARGET_TC}/${TARGET_TC}/sysroot/usr/bin/localedef root@${REMOTE}:/tmp/root

# On the Kobo
## /tmp/root/localedef -i POSIX -f UTF-8 C.UTF-8
ssh root@${REMOTE} /tmp/root/localedef -i en_US -f ISO-8859-1 en_US
ssh root@${REMOTE} /tmp/root/localedef -i en_US -f UTF-8 en_US.UTF-8
ssh root@${REMOTE} rm -rf /usr/share/i18n/charmaps /usr/share/i18n/locales
ssh root@${REMOTE} rm -f /tmp/root/localedef

# Magic is now stored in /usr/lib/locale/locale-archive

# NOTE: If your device is running glibc 2.19 (i.e., FW 4.6+), I have a precompiled package here:
#       http://files.ak-team.com/niluje/mrpub/Other/Kobo-glibc-2.19-locales.tar.gz
#       (unpack in /)
