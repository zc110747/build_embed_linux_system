# Linux系统开发学习笔记

本系列整理一系列嵌入式Linux学习和开发中应用的技术，包括不限于Linux平台应用，Shell/Makefile语法，uboot/kernel/rootfs移植构建，驱动开发，应用开发等。可以用于系统的学习和了解嵌入式Linux开发的各方面知识，对于本项目同步在如下地址更新。

```shell
#github地址
git clone https://github.com/zc110747/build_embed_linux_system.git
```

另外本项目也配合产品应用项目用于验证相关技术知识，具体如下。

```shell
#用于远程管理的嵌入式Linux应用项目
git clone http://github.com/zc110747/remote_manage.git
```

本系列的目录如下所示。

[ch00.前言](./src/ch00.PRFEACE.md)

- [ch01.构建Linux开发平台环境](./src/ch01-00.platform_env_struct.md)
  - [ch01-01.shell命令](./src/ch01-01.linux_shell_cmd.md)
  - [ch01-02.Linux软件安装](./src/ch01-02.linux_software_install.md)
  - [ch01-03.Linux shell脚本语法](./src/ch01-03.linux_shell_grammar.md)
  - [ch01-04.Linux交叉编译方法](./src/ch01-04.linux_cross_compiler.md)
  - [ch01-05.Linux问题总结](./src/ch01-05.linux_issue_conclusion.md)
  - [ch01-06.Linux快速部署](./ch01-06.linux_quick_deploy.md)

- [ch02.Linux系统启动实现](./src/ch02-00.system_bringup.md)
  - [ch02-01.Makefile脚本语法](./src/ch02-01.makefile.md)
  - [ch02-02.menuconfig界面管理](./src/ch02-02.menuconfig.md)
  - [ch02-03.uboot移植](./src/ch02-03.uboot_transplant.md)
  - [ch02-04.uboot环境和命令](./src/ch02-04.uboot_env.md)
  - [ch02-05.uboot执行流程分析](./src/ch02-05.uboot_analyse.md)
  - [ch02-06.Linux kernel移植](./src/ch02-06.kernel_transplate.md)
  - [ch02-07.内核执行流程分析](./src/ch02-07.kernel_analyse.md)
  - [ch02-08.文件系统构建](./src/ch02-08.rootfs.md)
  - [ch02-09 qemu虚拟机环境构建](./src/ch02-09.qemu.md)

- [ch03.Linux驱动开发](./src/ch03-00.driver_design.md)
  - [ch03-01.内核主要接口](./src/ch03-01.kernel_base_api.md)
  - [ch03-02.内核设备树相关接口](./src/ch03-02.kernel_tree_api.md)
  - [ch03-03.内核devm相关接口](./src/ch03-03.kernel_devm_api.md)
  - [ch03-04.内核并发接口](./src/ch03-04.kernel_concurrency_api.md)
  - [ch03-05.设备树语法说明](./src/ch03-05.device_tree.md)
  - [ch03-06.ARM内核分析](./src/ch03-06.arm_core_analyse.md)
  - [ch03-07.字符型设备](./src/ch03-07.char_device.md)
  - [ch03-08.嵌入式中断管理](./src/ch03-08.interrput_interaction.md)
  - [ch03-09.子系统说明](./src/ch03-09.subsytem.md)
  - [ch03-10.块设备说明](./src/ch03-10.block_device.md)
  - [ch03-11.网络设备](./src/ch03-11.net_device.md)

- [ch04.Linux应用开发设计](./src/ch04-00.application_design.md)
  - [ch04-01.Linux应用API接口](./src/ch04-01.linux_api_interface.md)
  - [ch04-02.常用cpp语法应用](./src/ch04-02.cpp_grammar.md)
  - [ch04-03.Linux平台调试方法](./src/ch04-03.linux_debug.md)
  - [ch04-04.Linux平台应用设计方法](./src/ch04-04.linux_app_frame.md)
  - [ch04-05.web应用设计](./src/ch04-05.web_design.md)
  - [ch04-06.嵌入式界面设计](./src/ch04-06.embed_gui_design.md)

[ch05.结语](./src/ch05.CONCLUSION.md)

## 嵌入式Linux学习路线

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

目录文档:

- [Linux平台构建说明](./src/ch01-00.platform_env_struct.md)
- [Linux平台shell命令说明](./src/ch01-01.linux_shell_cmd.md)
- [Linux平台软件安装](./src/ch01-02.linux_software_install.md)
- [Linux平台常见问题解决](./src/ch01-05.linux_issue_conclusion.md)

### step2: 基于已移植源码构建嵌入式Linux平台

主要目标:

1. 能够完成嵌入式Linux系统构建，并更新到硬件中实际运行
2. 熟悉在U-Boot和Linux下的环境变量参数和命令操作
3. 理解驱动的框架，参考例程实现字符型设备驱动

主要内容:

1. 熟悉U-Boot，Kernel和rootfs基本概念
2. 掌握menuconfig配置修改方法及Makefile编译
3. 熟悉至少一款芯片的下载更新方法(包含不限于sdcard, tftp，nfs等)
4. 尝试至少一种文件系统的编译构建过程(busybox, buildroot, yocto, debain, ubuntu或android)
5. 掌握U-Boot和Linux平台中的主要环境变量和系统命令
6. 参考源码实现字符型设备驱动，完成驱动加载，移除，并了解在设备中存在的地址。

目录文档:

- [嵌入式Linux平台启动说明](./src/ch02-00.system_bringup.md)
- [menuconfig语法](./src/ch02-02.menuconfig.md)
- [U-Boot环境变量和命令说明](./src/ch02-04.uboot_env.md)
- [Linux驱动开发说明](./src/ch03-00.driver_design.md)
- [Linux字符型设备开发](./src/ch03-05.char_device.md)

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
5. 基于打印或gdb的调试方法学习

目录文档:

- [Linux软件交叉编译实现](./src/ch01-04.linux_cross_compiler.md)
- [Makefile语法和实现说明](./src/ch02-01.makefile.md)
- [Linux应用设计说明](./src/ch04-00.application_design.md)
- [Linux应用层API接口](./src/ch04-01.linux_api_interface.md)
- [基于C++开发Linux应用](./src/ch04-02.cpp_grammar.md)
- [Linux应用调试方法](./src/ch04-03.linux_debug.md)

### step4：嵌入式驱动开发

主要目标：

1. 掌握Linux主要驱动的开发方法, 内核提供驱动的接口
2. 理解SOC结构，包含core，clock，peripheral等.
3. 理解设备树语法，与硬件的对应关系，在内核中的处理实现
4. 能够实现大部分字符型驱动
5. 能够理解block和网络驱动
6. 掌握子系统开发方法

主要内容:

1. Linux内核接口功能和使用说明
2. Linux内核并发控制接口说明
3. Cortex-A芯片设计和内核资源说明
4. 设备树语法以及设备树的应用
5. 字符型驱动开发
6. block和网络驱动开发
7. spi, i2c, input, usb子系统开发

目录文档:

- [内核驱动基本接口](./src/ch03-01.kernel_base_api.md)
- [内核驱动设备树接口](./src/ch03-02.kernel_tree_api.md)
- [内核资源管理接口](./src/ch03-03.kernel_devm_api.md)
- [内核并发控制接口](./src/ch03-04.kernel_concurrency_api.md)
- [设备树说明](./src/ch03-05.device_tree.md)
- [Cortex-A内核和硬件资源](./src/ch03-06.hardware_analyse.md)
- [字符型设备开发](./src/ch03-07.char_device.md)
- [中断资源说明](./src/ch03-08.interrput_interaction.md)
- [Linux内核子系统说明](./src/ch03-09.subsytem.md)
- [块设备驱动开发](./src/ch03-10.block_device.md)
- [网络设备驱动开发](./src/ch03-11.net_device.md)

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

目录文档:

- [Linux平台shell脚本语法](./src/ch01-03.linux_shell_grammar.md)
- [Linux平台快速部署实现](./src/ch01-06.linux_quick_deploy.md)
- [Linux应用框架构建](./src/ch04-04.linux_app_frame.md)
- [web应用开发](./src/ch04-05.web_design.md)
- [gui应用开发](./src/ch04-06.embed_gui_design.md)

### 扩展: 芯片bringup和Linux系统平台掌握

主要目标:

1. U-Boot移植，理解U-Boot执行流程
2. Kernel移植过程，理解Kernel执行流程
3. 完成至少一款芯片的bringup完整流程
4. 基于qemu的仿真环境构建执行

主要内容:

1. U-Boot移植方法
2. U-Boot执行流程分析
3. Kernel移植方法
4. Kernel执行流程分析
5. 文件系统移植和构建方法
6. qemu虚拟机模拟嵌入式Linux系统运行

目录文档:

- [U-Boot移植](./src/ch02-03.uboot_transplant.md)
- [U-Boot执行流程分析](./src/ch02-05.uboot_analyse.md)
- [Kernel移植](./src/ch02-06.kernel_transplate.md)
- [Kernel执行流程分析](./src/ch02-07.kernel_analyse.md)
- [文件系统移植和构建](./src/ch02-08.rootfs.md)
- [qemu虚拟机模拟](./src/ch02-09.qemu.md)