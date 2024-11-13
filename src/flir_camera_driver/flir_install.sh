#!/bin/sh

if [ "$(id -u)" = "0" ]
then
    echo
    echo "This script will assist users in configuring their udev rules to allow"
    echo "access to USB devices. The script will create a udev rule which will"
    echo "add FLIR USB devices to a group called flirimaging. The user may also"
    echo "choose to restart the udev daemon. All of this can be done manually as well."
    echo
else
    echo
    echo "This script needs to be run as root, e.g.:"
    echo "sudo ./flir_install.sh"
    echo
    exit 0
fi

cd /home/$(logname)

# Download & Install Spinnaker SDK
wget https://flir.netx.net/file/asset/66379/original/attachment/spinnaker-3.2.0.62-amd64-pkg.20.04.tar.gz
gzip -d spinnaker-3.2.0.62-amd64-pkg.20.04.tar.gz
tar xvf spinnaker-3.2.0.62-amd64-pkg.20.04.tar
cd spinnaker-3.2.0.62-amd64

# install_spinnaker.sh
echo "Installing Spinnaker packages..."
sudo dpkg -i libgentl_*.deb
sudo dpkg -i libspinnaker_*.deb
sudo dpkg -i libspinnaker-dev_*.deb
sudo dpkg -i libspinnaker-c_*.deb
sudo dpkg -i libspinnaker-c-dev_*.deb
sudo dpkg -i libspinvideo_*.deb
sudo dpkg -i libspinvideo-dev_*.deb
sudo dpkg -i libspinvideo-c_*.deb
sudo dpkg -i libspinvideo-c-dev_*.deb
sudo apt-get install -y ./spinview-qt_*.deb
sudo dpkg -i spinview-qt-dev_*.deb
sudo dpkg -i spinupdate_*.deb
sudo dpkg -i spinupdate-dev_*.deb
sudo dpkg -i spinnaker_*.deb
sudo dpkg -i spinnaker-doc_*.deb
############################################################################################################################
#configure_spinnaker.sh
grpname="flirimaging"
usrname=$(logname)
if (getent passwd $usrname > /dev/null)
    then
        groupadd -f $grpname
        usermod -a -G $grpname $usrname
        echo "Added user $usrname"
    else
        echo "User "\""$usrname"\"" does not exist" >&2
fi


UdevFile="/etc/udev/rules.d/40-flir-spinnaker.rules"
echo
echo "Writing the udev rules file...";
echo "SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"1e10\", GROUP=\"$grpname\"" 1>>$UdevFile
echo "SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"1724\", GROUP=\"$grpname\"" 1>>$UdevFile

/etc/init.d/udev restart

echo "Configuration complete."
echo "A reboot may be required on some systems for changes to take effect."
############################################################################################################################
# set USB-FS memory size to 1000 MB at startup (via /etc/rc.local)
# configure_usbfs.sh
echo "set USB-FS memory size to 1000 MB at startup (via /etc/rc.local)"
RCLOCAL_PATH="/etc/rc.local"

USBFS_DEFAULT_VALUE=1000
USBFS_COMMAND="sh -c 'echo $USBFS_DEFAULT_VALUE > /sys/module/usbcore/parameters/usbfs_memory_mb'"
USBFS_COMMAND_PATTERN="(sh -c 'echo )([0-9]+)( > \/sys\/module\/usbcore\/parameters\/usbfs_memory_mb')"

DEFAULT_RCLOCAL=$(cat <<-END
#!/bin/sh -e
#
# rc.local
#
# This script is executed at the end of each multiuser runlevel.
# Make sure that the script will "exit 0" on success or any other
# value on error.
#
# In order to enable or disable this script just change the execution
# bits.
#
# By default this script does nothing.

sh -c 'echo $USBFS_DEFAULT_VALUE > /sys/module/usbcore/parameters/usbfs_memory_mb'

exit 0

END
)

echo # Newline

# Check if rc.local exists
if [ -f "$RCLOCAL_PATH" ]
then
    # If the command doesn't already exist in rc.local, insert it.
    #   egrep, grep -E         interpret pattern as an extended regular expression
    #   -q, --quiet, --silent  do not write to stdout and exit immediately with status 0 if any match is found
    egrep -q "$USBFS_COMMAND_PATTERN" "$RCLOCAL_PATH"
    if [ $? -eq 0 ]
    then # Command already exists

        # Count number of occurrences of command
        if [ $(egrep "$USBFS_COMMAND_PATTERN" "$RCLOCAL_PATH" | wc --lines) -eq 1 ]
        then
            USBFS_EXISTING_VALUE=$(egrep "$USBFS_COMMAND_PATTERN" "$RCLOCAL_PATH" | sed -r "s/[ ]*$USBFS_COMMAND_PATTERN/\2/")

            # Memory is already >= the recommended value?
            if [ $USBFS_EXISTING_VALUE -ge $USBFS_DEFAULT_VALUE ]
            then
                echo "USB-FS memory is already set to $USBFS_EXISTING_VALUE MB which is greater than or equal to the recommended value of $USBFS_DEFAULT_VALUE MB."
                echo "No changes were made."
            else
                echo "Found an existing USB-FS memory configuration (currently $USBFS_EXISTING_VALUE MB)."
                echo "Would you like to change this to $USBFS_DEFAULT_VALUE MB?"

                # -i modifies the file and creates a backup, -r enables extended regex, -n suppresses output
                sudo sed -i.backup -r "s/$USBFS_COMMAND_PATTERN/\1$USBFS_DEFAULT_VALUE\3/" "$RCLOCAL_PATH"
                echo "Changed USB-FS memory to $USBFS_DEFAULT_VALUE MB and created $RCLOCAL_PATH.backup."

            fi
        else # If there is more than one occurrence, notify to update manually to avoid breaking things
            echo "Found more than one occurrence of the USB-FS memory configuration command in /etc/rc.local."
            echo "Please consult the 'USB notes' section in the included README for manual configuration instructions."
        fi
    else # Command doesn't exist in rc.local yet
        # If there exists an `exit 0` in rc.local, insert command before the last occurrence
        grep -Fxq "exit 0" "$RCLOCAL_PATH"
        if [ $? -eq 0 ]
        then
            # Get last 'exit 0' in format `<line #>:exit 0`
            #     -n, --line-number  prefix each line with 1-based line number
            USBFS_GREP_LAST_EXIT=$(grep -nFx "exit 0" "$RCLOCAL_PATH" | tail -1)

            # Strip text after colon
            USBFS_EXIT_LINE=${USBFS_GREP_LAST_EXIT%:*}

            # Insert usbfs command right before the `exit 0`
            sudo sed -i.backup -r "${USBFS_EXIT_LINE}i ${USBFS_COMMAND}" "$RCLOCAL_PATH"
        else # Otherwise, append command to end of file
            sudo sed -i.backup -r "\$a${USBFS_COMMAND}\nexit 0" "$RCLOCAL_PATH"
        fi
        echo "Set USB-FS memory to $USBFS_DEFAULT_VALUE MB and created $RCLOCAL_PATH.backup."
    fi
else
    # Using tee to redirect standard output with superuser rights
    echo "$DEFAULT_RCLOCAL" | sudo tee --append "$RCLOCAL_PATH" >/dev/null
    sudo chmod 777 "$RCLOCAL_PATH"
    echo "Created /etc/rc.local and set USB-FS memory to $USBFS_DEFAULT_VALUE MB."
fi

# Check if owner group has execute permissions
stat -c "%a" "$RCLOCAL_PATH" | egrep -q "[1|3|5|7][0-7][0-7]"
if [ $? -eq 1 ]
then
    echo
    echo "The /etc/rc.local file may not be able to run with its current"
    echo -n "permissions ("
    echo -n $(stat -c "%A" "$RCLOCAL_PATH")
    echo "). Set recommended (-rwxr--r--) permissions?"

    sudo chmod 777 "$RCLOCAL_PATH"
    echo "Changed $RCLOCAL_PATH permissions to 744."
fi

############################################################################################################################
# Spinnaker prebuilt examples available in your system path
# configure_spinnaker_paths.sh

SPINNAKER_SETUP_SCRIPT="setup_spinnaker_paths.sh"
SPINNAKER_SETUP_PATH="/etc/profile.d/$SPINNAKER_SETUP_SCRIPT"
SPINNAKER_BIN_PATH="/opt/spinnaker/bin"

PATH_VAR=\$PATH

cat << EOF > $SPINNAKER_SETUP_PATH
#!/bin/sh
if [ -d $SPINNAKER_BIN_PATH ]; then
    if [[ $PATH_VAR != *"$SPINNAKER_BIN_PATH"* ]]; then
        export PATH=$SPINNAKER_BIN_PATH:$PATH_VAR
    fi
fi
EOF

echo "$SPINNAKER_SETUP_SCRIPT has been added to /etc/profile.d"
echo "The PATH environment variable will be updated every time a user logs in."
echo "To run Spinnaker prebuilt examples in the current session, you can update the paths by running:"
echo "  source $SPINNAKER_SETUP_PATH"

############################################################################################################################
# install_spinnaker.sh
ARCH=$(ls libspinnaker_* | grep -oP '[0-9]_\K.*(?=.deb)' || [[ $? == 1 ]])
if [ "$ARCH" = "amd64" ]; then
    BITS=64
elif [ "$ARCH" = "i386" ]; then
    BITS=32
fi
############################################################################################################################
# configure_gentl_paths.sh
if [ "$(id -u)" -ne "0" ]
then
    echo "This script needs to be run as root, e.g.:"
    echo "sudo configure_gentl_paths.sh 64"
    exit 1
fi

if { [ "$BITS" -ne 32 ] && [ "$BITS" -ne 64 ]; }; then

    echo "Invalid arguments: Missing GenTL producer architecture (32 or 64 bit)"
    exit 1
fi

GENTL_SETUP_SCRIPT="setup_flir_gentl_$BITS.sh"
GENTL_SETUP_PATH="/etc/profile.d/$GENTL_SETUP_SCRIPT"
CTI_PATH="/opt/spinnaker/lib/flir-gentl"
CTI_FILE_PATH="${CTI_PATH}/FLIR_GenTL.cti"

FLIR_GENTL_VAR_NAME="FLIR_GENTL${BITS}_CTI"
GENTL_VAR_NAME="GENICAM_GENTL${BITS}_PATH"
GENTL_VAR=\$${GENTL_VAR_NAME}

cat << EOF > $GENTL_SETUP_PATH

export $FLIR_GENTL_VAR_NAME=$CTI_FILE_PATH
if [ -d $CTI_PATH ]; then
    if [ -z $GENTL_VAR ]; then
        export $GENTL_VAR_NAME=$CTI_PATH
    elif [[ $GENTL_VAR != *"$CTI_PATH"* ]]; then
        export $GENTL_VAR_NAME=$CTI_PATH:$GENTL_VAR
    fi
fi
EOF

echo "$GENTL_SETUP_SCRIPT has been added to /etc/profile.d"
echo "The $FLIR_GENTL_VAR_NAME and $GENTL_VAR_NAME environment variables will be updated every time a user logs in."
echo "To use the FLIR GenTL producer in the current session, you can update the $FLIR_GENTL_VAR_NAME and $GENTL_VAR_NAME environment variables by running:"
echo "  source $GENTL_SETUP_PATH $BITS"

echo "Spinnaker SDK Installation complete."


sudo apt install ros-noetic-camera-info-manager ros-noetic-diagnostic-updater ros-noetic-dynamic-reconfigure ros-noetic-image-exposure-msgs ros-noetic-image-transport ros-noetic-nodelet ros-noetic-roscpp ros-noetic-sensor-msgs ros-noetic-wfov-camera-msgs -y

cd ..
sudo rm spinnaker-3.2.0.62-amd64-pkg.20.04.tar

exit 0
