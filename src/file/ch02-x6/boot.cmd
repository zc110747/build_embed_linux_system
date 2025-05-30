# boot script file

## gloabl config
echo "## boot script loaded!"
setenv load_addr 80810000
setenv kernel_addr_r 80800000
setenv fdt_addr_r 83000000
setenv config_file "config.txt"
setenv boot_mod "net"
setenv console_baud "115200"
setenv config_filesize 0x10000
setenv console_serial "ttymxc0"

## support loader config file
if tftp ${load_addr} ${config_file}; then
    echo "## load import file: ${config_file}"
    env import -t ${load_addr} ${config_filesize}
else
    echo "## use default information"
fi

if test "${display_bootinfo}" = "enable"; then setenv consoleargs_diplay "console=tty0"; fi

echo "## boot script load finished, kernel start!"

## support device driver dtbo
if test "${boot_mod}" = "net"; then 
    setenv bootargs "console=${console_serial},${console_baud} ${consoleargs_diplay} root=/dev/nfs nfsroot=${serverip}:${nfspath},proto=tcp,nfsvers=3 rw ip=${ipaddr}:${serverip}:${gateway}:${netmask}::eth0:off earlyprintk loglevel=${printk_level}"
    printenv bootargs

    # setenv netboot_cmd "tftp 80800000 zImage; tftp 83000000 ${fdt_file}; bootz 80800000 - 83000000;"
    # run netboot_cmd

    tftp ${fdt_addr_r} ${fdt_file}

    fdt addr ${fdt_addr_r}
    fdt resize 65536

    for overlay_file in ${overlays}; do
        if tftp ${load_addr} ${overlay_file}.dtbo; then
            echo "Applying kernel provided DT overlay ${overlay_file}.dtbo"
            fdt apply ${load_addr} || setenv overlay_error "true"
        fi
    done

    if test "${overlay_error}" = "true"; then
        echo "Applying kernel overlay failed, use default dtb"
        tftp ${fdt_addr_r} ${fdt_file}
    else
        echo "Applying kernel overlay success!!!!!!!!!!!!!"
    fi

    tftp ${kernel_addr_r} zImage
    
    ## jump to kernel
    bootz ${kernel_addr_r} - ${fdt_addr_r}
else 
    setenv bootargs "console=${console_serial},${console_baud} ${consoleargs_diplay} panic=5 rootwait root=${mmcroot} earlyprintk rw loglevel=${printk_level}"
    run mmcboot_cmd
fi
