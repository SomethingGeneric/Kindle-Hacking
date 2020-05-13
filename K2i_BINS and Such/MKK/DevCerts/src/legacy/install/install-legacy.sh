#!/bin/sh
#
# $Id: install-legacy.sh 10836 2014-08-22 15:40:00Z NiLuJe $
#

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils ] && source ./libotautils


HACKNAME="kindlet-dev"

otautils_update_progressbar

# Doooo eeeeet!
logmsg "I" "install" "" "Installing combined developer keystore"
cp -f developer.keystore /var/local/java/keystore/developer.keystore

otautils_update_progressbar

logmsg "I" "install" "" "Installing Kindlet jailbreak"
cp -f json_simple-1.1.jar /opt/amazon/ebook/lib/json_simple-1.1.jar
chmod 0664 /opt/amazon/ebook/lib/json_simple-1.1.jar

otautils_update_progressbar

# Cleanup
rm -f developer.keystore
rm -f json_simple-1.1.jar
logmsg "I" "install" "" "done"

otautils_update_progressbar

return 0
