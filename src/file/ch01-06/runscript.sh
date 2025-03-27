runpath=$(pwd)

titlestr="Choose Run Script"
backtitle="Remote Manage building script"
menustr=""
TTY_X=$(($(stty size | awk '{print $2}')-6)) 			# determine terminal width
TTY_Y=$(($(stty size | awk '{print $1}')-6)) 			# determine terminal height

options+=("prepare"	 "prepare for enviroment")
options+=("uboot"	 "build for uboot")
options+=("kernel"	 "build for kernel")
options+=("rootfs"	 "build for rootfs")
options+=("application"	 "build for application")

CONF_DIR=$(whiptail --title "${titlestr}" --backtitle "${backtitle}" --notags \
    --menu "${menustr}" "${TTY_Y}" "${TTY_X}" $((TTY_Y - 8))  \
    --cancel-button Exit --ok-button Select "${options[@]}" \
3>&1 1>&2 2>&3)
echo $CONF_DIR
[[ -z $CONF_DIR ]] && exit

case "${CONF_DIR}" in
    "prepare")
        ${runpath}/preBuildEnvironment.sh
    ;;
    "uboot")
        SysBuildUboot
        cd ${runpath}
    ;;
esac