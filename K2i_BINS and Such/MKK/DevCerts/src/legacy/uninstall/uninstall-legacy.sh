#!/bin/sh
#
# $Id: uninstall-legacy.sh 10845 2014-08-23 15:52:09Z NiLuJe $
#

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils ] && source ./libotautils


HACKNAME="kindlet-dev"

otautils_update_progressbar

# Doooo eeeeet!
logmsg "I" "uninstall" "" "Removing combined developer keystore"
rm -f /var/local/java/keystore/developer.keystore

otautils_update_progressbar

logmsg "I" "uninstall" "" "Uninstalling Kindlet jailbreak"
cp -f json_simple-1.1.jar /opt/amazon/ebook/lib/json_simple-1.1.jar
chmod 0664 /opt/amazon/ebook/lib/json_simple-1.1.jar

otautils_update_progressbar

# Cleanup
rm -f json_simple-1.1.jar
logmsg "I" "uninstall" "" "done"

otautils_update_progressbar

return 0
