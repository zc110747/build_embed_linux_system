# 嵌入式Linux系统总结

## 系列目录

[ch00.前言](./src/ch00.PRFEACE.md)

- [ch01.构建linux开发平台环境](./src/ch01-00.platform_env_struct.md)
  - [ch01-01.shell命令](./src/ch01-01.linux_shell_cmd.md)
  - [ch01-02.linux软件安装](./src/ch01-02.linux_software_install.md)
  - [ch01-03.linux shell脚本语法](./src/ch01-03.linux_shell_grammar.md)
  - [ch01-04.linux交叉编译方法](./src/ch01-04.linux_cross_compiler.md)
  - [ch01-05.linux快速部署和SDK构建方法](./src/ch01-05.linux_quick_deploy.md)
  - [ch01-xz.linux平台常见问题总结](./src/ch01-xz.linux_feature_notes.md)

- [ch02.linux系统启动实现](./src/ch02-00.system_bringup.md)
  - [ch02-01.makefile脚本语法](./src/ch02-01.makefile.md)
  - [ch02-02.menuconfig界面管理](./src/ch02-02.menuconfig.md)
  - [ch02-03.uboot移植](./src/ch02-03.uboot_imx_transplate.md)
  - [ch02-04.uboot环境和命令](./src/ch02-04.uboot_env.md)
  - [ch02-05.uboot执行流程分析](./src/ch02-05.uboot_analyse.md)
  - [ch02-06.linux kernel移植](./src/ch02-06.kernel_transplate.md)
  - [ch02-07.内核执行流程分析](./src/ch02-07.kernel_analyse.md)
  - [ch02-08.文件系统构建综述](./src/ch02-08.rootfs_overview.md)
  - [ch02-09.基于busybox构建文件系统](./src/ch02-09.rootfs_busybox.md)
  - [ch02-10.基于buildroot构建文件系统](./src/ch02-10.rootfs_buildroot.md)
  - [ch02-11.基于debian构建文件系统](./src/ch02-11.rootfs_debian.md)
  - [ch02-12.基于ubuntu构建文件系统](./src/ch02-12.rootfs_ubuntu.md)
  - [ch02-13 支持qt的linux文件系统构建](./src/ch02-13.rootfs_qtsupport.md)
  - [ch02-14 基于openwrt构建系统平台](./src/ch02-14.rootfs_openwrt.md)
  - [ch02-15 安卓sdk构建](./src/ch02-15.android_sdk.md)
  - [ch02-16 qemu虚拟机环境构建](./src/ch02-16.qemu.md)
  - [ch02-17.u-boot spl和TF-A机制](./src/ch02-17.spl_and_tfa.md)
  - [ch02-x1.imx6ull芯片平台编译方法](./src/ch02-x1.imx6ull_platform.md)
  - [ch02-x2.瑞芯微rk3568平台编译方法](./src/ch02-x2.rk3568_platform.md)
  - [ch02-x3.全志t113i平台编译方法](./src/ch02-x3.t113i_platform.md)
  - [ch02-x4.全志h618平台编译方法](./src/ch02-x4.h618_platform.md)
  - [ch02-x5.从stm32移植u-boot理解启动流程](./src/ch02-x5.stm32_uboot.md)
  - [ch02-x6.uboot命令行脚本和设备树overlay实现](./src/ch02-x6.uboot_script_overlay.md)
  - [ch02-xz.系统平台常见问题总结](./src/ch02-xz.system_feature_notes.md)

- [ch03.linux驱动开发](./src/ch03-00.driver_design.md)
  - [ch03-01.设备树说明](./src/ch03-01.device_tree.md)
  - [ch03-02.arm内核和硬件分析](./src/ch03-02.armcore_analyse.md)
  - [ch03-03.字符设备驱动说明](./src/ch03-03.char_device.md)
  - [ch03-04.pinctrl和gpio子系统](./src/ch03-04.pinctrl_subsystem.md)
  - [ch03-05.input输入子系统](./src/ch03-05.input_subsystem.md)
  - [ch03-06.驱动中断管理](./src/ch03-06.interrput_interaction.md)
  - [ch03-07.i2c设备和驱动管理框架](./src/ch03-07.i2c_frame.md)
  - [ch03-08.spi设备和驱动管理框架](./src/ch03-08.spi_frame.md)
  - [ch03-09.iio子系统驱动框架](./src/ch03-09.iio_subsystem.md)
  - [ch03-10.regmap驱动框架说明](./src/ch03-10.regmap_interface.md)
  - [ch03-11.rtc硬件管理框架](./src/ch03-11.rtc_subsystem.md)
  - [ch03-12.pwm子系统设备框架](./src/ch03-12.pwm_frame.md)
  - [ch03-13.背光驱动管理框架](./src/ch03-13.brightness.md)
  - [ch03-14.系统温控驱动管理框架](./src/ch03-14.thermal_frame.md)
  - [ch03-15.watchdog设备管理框架](./src/ch03-15.wdt.md)
  - [ch03-16.random随机数模块](./src/ch03-16.random.md)
  - [ch03-17.regulator电源管理](./src/ch03-17.regulator.md)
  - [ch03-18.串口tty管理框架](./src/ch03-18.tty_serial.md)
  - [ch03-19.nvmem子系统管理框架](./src/ch03-19.nvmem.md)
  - [ch03-20.块设备驱动](./src/ch03-20.block_device.md)
  - [ch03-21.usb驱动管理框架](./src/ch03-21.usb.md)
  - [ch03-22.can网络管理框架](./src/ch03-22.can_net_device.md)
  - [ch03-23.网络设备驱动实现](./src/ch03-23.emac_net_device.md)
  - [ch03-24.图形显示framebuffer管理框架](./src/ch03-24.video_frame.md)
  - [ch03-25.图形显示drm管理框架](./src/ch03-25.drm_frame.md)
  - [ch03-26.摄像头csi接口管理驱动](./src/ch03-26.csi_frame.md)
  - [ch03-27.系统时钟和OPP管理模块](./src/ch03-27.cpufreq_opp.md)
  - [ch03-28.音频dac输出模块](./src/ch03-28.audio.md)
  - [ch03-29.内核中的并发管理](./src/ch03-29.core_concurrency.md)
  - [ch03-x1.驱动基础接口](./src/ch03-x1.kernel_base_api.md)
  - [ch03-x2.驱动设备树接口](./src/ch03-x2.kernel_tree_api.md)
  - [ch03-x3.驱动基础devm接口](./src/ch03-x3.kernel_resource_base_api.md)
  - [ch03-x4.驱动扩展devm接口](./src/ch03-x4.kernel_resource_improve_api.md)
  - [ch03-x5.驱动并发接口](./src/ch03-x5.kernel_concurrency_api.md)
  - [ch03-x6.imx6ull设备树分析](./src/ch03-x6.mx6ull_dts_analysis.md)
  - [ch03-x7.t113i设备树分析](./src/ch03-x7.t113_i_dts_analysis.md)
  - [ch03-x8.rk3568设备树分析](./src/ch03-x8.rk3568_dts_analysis.md)
  - [ch03-xz.驱动问题分析和解决](./src/ch03-xz.driver_feature_notes.md)

- [ch04.linux应用开发设计](./src/ch04-00.application_design.md)
  - [ch04-01.c++语法说明](./src/ch04-01.cpp_grammar.md)
  - [ch04-02.linux环境下的调试方法](./src/ch04-02.linux_debug_gdb.md)
  - [ch04-03.时间相关处理应用](./src/ch04-03.time_function.md)
  - [ch04-04.基于lvgl的界面开发](./src/ch04-04.socket_design.md)
  - [ch04-05.linux平台web应用开发](./src/ch04-05.process_communication.md)
  - [ch04-06.linux平台的socket网络实现](./src/ch04-06.hardware_access.md)
  - [ch04-07.使用python进行应用开发](./src/ch04-07.web_design.md)
  - [ch04-08.QT图形应用开发](./src/ch04-08.gui_qt_design.md)
  - [ch04-09.LVGL移植和图形应用开发](./src/ch04-09.lvgl_application.md)
  - [ch04-08.Python应用开发](./src/ch04-10.python_design.md)
  - [ch04-08.综合应用开发项目](./src/ch04-11.integrated_design.md)
  - [ch04-x1.基础api接口说明](./src/ch04-x1.linux_base_api.md)
  - [ch04-x2.复杂api接口说明](./src/ch04-x2.linux_complex_api.md)
  - [ch04-x3.cmake编译语法](./src/ch04-x3..cmake.md)

- [ch05.结语](./src/ch05.CONCLUSION.md)

- [附录一:网址和开源项目](./src/end_annex-01.website.md)
- [附录二:面试问题整理](./src/end_annex-02.interview.md)

## 说明

本系列整理一系列嵌入式Linux学习和开发中应用的技术，按照功能划分为SDK平台构建、嵌入式Linux运行平台构建，驱动开发技术和应用开发技术，理论上涉及嵌入式Linux开发中的主要技术。

- SDK平台构建
  - Linux平台安装(Ubuntu、Arch、centos)
  - 软件和支持库安装，交叉编译
  - shell语法
  - 快速部署SDK脚本工具集实现
- 嵌入式Linux运行平台构建
  - u-boot移植和框架
  - kernel移植和框架
  - rootfs文件系统构建方法
  - qemu实用虚拟机启动Linux平台
- 驱动开发技术
  - Linux内核驱动接口
  - 设备树语法和设备树接口
  - 字符设备，块设备和网络设备
  - 子系统功能实现(i2c, spi, input, iio)
- 应用开发技术
  - Linu应用层接口
  - cpp语法小结
  - Linux平台调试方法
  - Linux框架设计和web应用开发
  - Linux平台GUI开发技术

本系列主要按照如何构建一个完整嵌入Linux产品应用的开发流程来说明，而不是以难易度进行先后顺序的整理。如果在学习和理解中遇到难题，可以跳过直接进行后续的学习，从简单的入手，熟悉基础知识后可能更高效。另外嵌入式Linux系统是用于设计产品的技术，只依靠文档学习是远远不够的，需要在实际硬件平台调试验证，理解源码，自己设计开发应用，才能够深入理解掌握。可以看到本系列文章涉及内容广泛，限于篇幅部分技术只能浅尝辄止，在讲解相应章节时也会列出我在整理以及开发时涉及的书籍，可以同步去学习。

```shell
#github地址
git clone https://github.com/zc110747/build_embed_linux_system.git
```

另外本项目也配合产品应用项目用于验证相关技术知识，具体如下。

```shell
#用于远程管理的嵌入式Linux应用项目
git clone http://github.com/zc110747/remote_manage.git
```

## 学习路线

### step1: 安装和熟悉Linux平台

主要目标:

1. 能够安装Linux系统，并熟悉命令基本操作
2. 掌握Linux下软件安装方法(apt-get或yum)，效率类工具应用
3. 理解环境变量的基本概念，交叉编译的基础

主要内容:

1. 虚拟机使用以及安装Linux系统
2. 常用指令熟悉(cd, mv, cp, echo, rm, chmod...)
3. 常用软件查询，安装和应用(ssh, tftp, nfs, samba...)
4. 能够使用gcc和Makefile构建应用

### step2: 基于已移植源码构建嵌入式Linux平台

主要目标:

1. 能够完成嵌入式Linux系统构建，并更新到硬件中实际运行
2. 熟悉在U-Boot和Linux下的环境变量参数和命令操作
3. 理解驱动的框架，参考例程实现字符型设备驱动
4. 处理理解设备树的概念，并应用到驱动开发中

主要内容:

1. 熟悉U-Boot，Kernel和rootfs基本概念
2. 掌握menuconfig配置修改方法及Makefile编译
3. 熟悉至少一款芯片的下载更新方法(包含不限于sdcard, tftp，nfs等)
4. 尝试至少一种文件系统的编译构建过程(busybox, buildroot, yocto, debian, ubuntu或android)
5. 掌握U-Boot和Linux平台中的主要环境变量和系统命令
6. 参考源码实现字符型设备驱动，完成驱动加载，移除, 设备的创建过程

### step3：嵌入式Linux应用初步开发

主要目标:

1. 掌握Linux平台下应用的常见接口
2. 能够构建嵌入式Linux应用，模块化和Makefile构建
3. 常见软件和库的交叉编译方法，添加应用支持
4. 掌握Linux平台下的代码调试能力
5. 基于C++实现Linux平台应用开发

主要内容:

1. Linux应用层API接口掌握
2. Makefile语法和项目构建方法
3. 基于gcc实现Linux平台应用
4. Linux平台下库的交叉编译和应用(json, mqtt, asio, fmt...)

### step4：深入学习嵌入式驱动开发

主要目标：

1. 掌握Linux主要驱动的开发方法, 内核提供驱动的接口
2. 理解SOC结构，包含core，clock，peripheral等.
3. 理解中断机制，掌握内核常用中断相关接口使用
4. 能够实现大部分字符型驱动，掌握主要硬件框架的驱动开发方法
5. 能够基于常用子系统开发驱动
6. 掌握并发控制相关接口应用

主要内容:

1. Linux内核接口功能和使用说明
2. Linux内核并发控制接口说明
3. 中断机制和相关接口实现
4. Cortex-A芯片设计和内核资源说明
5. spi, i2c框架下的驱动开发
6. iio, rtc子系统下的驱动开发

### step5： 嵌入式Linux平台应用方案构建

主要目标:

1. APP功能框架构建和代码实现
2. 实现基于Qt C++的应用实现
3. 基于node和vue的web应用构建
4. 实现系统快速部署和编译命令实现

主要内容:

1. 应用需求分析，实现APP功能框架，分解模块
2. 基于Qt C++的方法实现
3. 基于node和vue的前后端框架实现
4. 熟练掌握shell语法，基础上构建快速部署方案
5. 基于打印或gdb的调试方法学习

### 扩展: 芯片bringup和Linux系统平台掌握

主要目标:

1. 掌握块设备，网络设备的驱动开发
2. 实现基于DRM框架的驱动开发
3. U-Boot移植，理解U-Boot执行流程
4. Kernel移植过程，理解Kernel执行流程
5. 完成至少一款芯片的bringup完整流程
6. 支持文件系统构建，busybox, debian, ubuntu和android.
7. 基于qemu的仿真环境构建系统

主要内容:

1. 块设备和网络设备驱动
2. drm框架驱动设备开发
3. U-Boot移植方法
4. U-Boot执行流程分析
5. Kernel移植方法
6. Kernel执行流程分析
7. 文件系统移植和构建方法
8. qemu虚拟机模拟嵌入式Linux系统运行
