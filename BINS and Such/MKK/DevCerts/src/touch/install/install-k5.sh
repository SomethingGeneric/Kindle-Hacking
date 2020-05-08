#!/bin/sh
#
# Combined Developer Keystore
#
# $Id: install-k5.sh 16922 2020-03-08 04:41:50Z NiLuJe $
#
##

HACKNAME="kindlet-dev"

# Pull libOTAUtils for logging & progress handling
[ -f ./libotautils5 ] && source ./libotautils5


# Hack specific stuff
MKK_PERSISTENT_STORAGE="/var/local/mkk"
RP_PERSISTENT_STORAGE="/var/local/rp"
MKK_BACKUP_STORAGE="/mnt/us/mkk"
RP_BACKUP_STORAGE="/mnt/us/rp"


# Doooo eeeeet!
logmsg "I" "install" "" "Installing combined developer keystore"
mkdir -p "/var/local/java/keystore"
cp -f developer.keystore /var/local/java/keystore/developer.keystore

otautils_update_progressbar

logmsg "I" "install" "" "Installing Kindlet jailbreak"
cp -f json_simple-1.1.jar /opt/amazon/ebook/lib/json_simple-1.1.jar
chmod 0664 /opt/amazon/ebook/lib/json_simple-1.1.jar

otautils_update_progressbar

# Make sure we have enough space left (>512KB) in /var/local first...
logmsg "I" "install" "" "checking amount of free storage space..."
if [ "$(df -k /var/local | awk '$3 ~ /[0-9]+/ { print $4 }')" -lt "512" ] ; then
    logmsg "C" "install" "code=1" "not enough space left in varlocal"
    # Cleanup & exit w/ prejudice
    rm -f developer.keystore
    rm -f json_simple-1.1.jar
    return 1
fi

otautils_update_progressbar

# Store a copy in /var/local for the bridge...
logmsg "I" "install" "" "Creating MKK persistent storage directory"
mkdir -p "${MKK_PERSISTENT_STORAGE}"

otautils_update_progressbar

logmsg "I" "install" "" "Storing combined developer keystore"
cp -f developer.keystore "${MKK_PERSISTENT_STORAGE}/developer.keystore"

otautils_update_progressbar

logmsg "I" "install" "" "Storing kindlet jailbreak"
cp -f json_simple-1.1.jar "${MKK_PERSISTENT_STORAGE}/json_simple-1.1.jar"

otautils_update_progressbar

logmsg "I" "install" "" "Storing gandalf"
cp -f gandalf "${MKK_PERSISTENT_STORAGE}/gandalf"

otautils_update_progressbar

logmsg "I" "install" "" "Setting up gandalf"
chown root:root "${MKK_PERSISTENT_STORAGE}/gandalf"
chmod a+rx "${MKK_PERSISTENT_STORAGE}/gandalf"
chmod +s "${MKK_PERSISTENT_STORAGE}/gandalf"
ln -sf "${MKK_PERSISTENT_STORAGE}/gandalf" "${MKK_PERSISTENT_STORAGE}/su"

otautils_update_progressbar

logmsg "I" "install" "" "Installing bridge job"
cp -f bridge.conf "/etc/upstart/bridge.conf"
chmod 0664 "/etc/upstart/bridge.conf"

otautils_update_progressbar

logmsg "I" "install" "" "Storing bridge job"
cp -af "/etc/upstart/bridge.conf" "${MKK_PERSISTENT_STORAGE}/bridge.conf"

otautils_update_progressbar

logmsg "I" "install" "" "Storing bridge script"
# Only if we don't already have one, to avoid potential version mismatches (since we don't ship with an up to date version, for simplicity's sake)...
if [ ! -f "${MKK_PERSISTENT_STORAGE}/bridge.sh" ] ; then
    cp -af "/var/local/system/fixup" "${MKK_PERSISTENT_STORAGE}/bridge.sh"
fi

otautils_update_progressbar

logmsg "I" "install" "" "Setting up persistent RP"
mkdir -p "${RP_PERSISTENT_STORAGE}"
for my_job in debrick cowardsdebrick ; do
    if [ -f "/etc/upstart/${my_job}.conf" ] ; then
        cp -af "/etc/upstart/${my_job}.conf" "${RP_PERSISTENT_STORAGE}/${my_job}.conf"
    fi
    if [ -f "/etc/upstart/${my_job}" ] ; then
        cp -af "/etc/upstart/${my_job}" "${RP_PERSISTENT_STORAGE}/${my_job}"
    fi
done

otautils_update_progressbar

logmsg "I" "install" "" "Setting up backup storage"
rm -rf "${MKK_BACKUP_STORAGE}"
mkdir -p "${MKK_BACKUP_STORAGE}"
rm -rf "${RP_BACKUP_STORAGE}"
mkdir -p "${RP_BACKUP_STORAGE}"
# Can't preserve symlinks & permissions on vfat, so do it the hard way ;).
for my_file in ${MKK_PERSISTENT_STORAGE}/* ; do
	if [ -f ${my_file} -a ! -L ${my_file} ] ; then
		cp -f "${my_file}" "${MKK_BACKUP_STORAGE}/"
	fi
done
for my_file in ${RP_PERSISTENT_STORAGE}/* ; do
	if [ -f ${my_file} -a ! -L ${my_file} ] ; then
		cp -f "${my_file}" "${RP_BACKUP_STORAGE}/"
	fi
done

otautils_update_progressbar

# Cleanup
rm -f developer.keystore
rm -f json_simple-1.1.jar
rm -f gandalf
rm -f bridge.conf
logmsg "I" "install" "" "done"

otautils_update_progressbar

return 0
