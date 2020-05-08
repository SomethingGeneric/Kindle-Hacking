#!/bin/sh
#
# Combined Developer Keystore
#
# $Id: uninstall-k5.sh 10845 2014-08-23 15:52:09Z NiLuJe $
#
##

HACKNAME="kindlet-dev"

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils5 ] && source ./libotautils5


# Hack specific stuff
MKK_PERSISTENT_STORAGE="/var/local/mkk"


# Doooo eeeeet!
logmsg "I" "uninstall" "" "Removing combined developer keystore"
rm -f /var/local/java/keystore/developer.keystore

otautils_update_progressbar

logmsg "I" "uninstall" "" "Installing Kindlet jailbreak"
cp -f json_simple-1.1.jar /opt/amazon/ebook/lib/json_simple-1.1.jar
chmod 0664 /opt/amazon/ebook/lib/json_simple-1.1.jar

otautils_update_progressbar

# Clear the bridge copy, too
logmsg "I" "install" "" "Clearing MKK persistent storage directory"
rm -rf "${MKK_PERSISTENT_STORAGE}"

otautils_update_progressbar

# Cleanup
rm -f json_simple-1.1.jar
logmsg "I" "uninstall" "" "done"

otautils_update_progressbar

return 0
