################################################################################
#
# This file is used to build u-boot shell
# 
#
################################################################################

#build boot.cmd
mkimage -C none -A arm -T script -d boot.cmd boot.scr
mv -fv boot.scr "${BUILD_TFTP_PATH}"/
cp -fv config.txt "${BUILD_TFTP_PATH}"/

#build dtbo
for file in $(find "./" -type f -name "*.dts"); do
    # 提取文件名（不包含路径）
    filename=$(basename "$file")
    filename="${filename%.*}"
    dtc -@ -I dts -O dtb -o ${filename}.dtbo ${filename}.dts
done

mv -fv *.dtbo "${BUILD_TFTP_PATH}"/