# Linux系统开发学习笔记

本系列整理一系列嵌入式Linux学习和开发中应用的技术，包括不限于Shell脚本，Makefile语法，uboot/kernal移植，驱动开发，应用开发等，详细目录如下。

[ch00.前言](./src/ch00.PRFEACE.md)

- [ch01.构建Linux开发平台环境](./src/ch01-00.platform_env_struct.md)
  - [ch01-01.shell命令](./src/ch01-01.linux_shell_cmd.md)
  - [ch01-02.Linux软件安装](./src/ch01-02.linux_software_install.md)
  - [ch01-03.Linux shell脚本语法](./src/ch01-03.linux_shell_grammar.md)
  - [ch01-04.Linux交叉编译方法](./src/ch01-04.linux_cross_compiler.md)
  - [ch01-05.Linux问题总结](./src/ch01-05.linux_issue_conclusion.md)

- [ch02.Linux系统启动实现](./src/ch02-00.system_bringup.md)
  - [ch02-01.Makefile脚本语法](./src/ch02-01.makefile.md)
  - [ch02-02.kconfig语法](./src/ch02-02.kconfig.md)
  - [ch02-03.uboot说明](./src/ch02-03.uboot.md)
  - [ch02-04.kernel说明](./src/ch02-04.kernel.md)
  - [ch02-05.rootfs说明](./src/ch02-05.rootfs.md)

- [ch03.Linux驱动开发](./src/ch03-00.driver_design.md)
  - [ch03-01.设备树说明](./src/ch03-01.device_tree.md)
  - [ch03-02.字符型设备](./src/ch03-02.char_device.md)
  - [ch03-03.中断和驱动中的交互](./src/ch03-03.interrput_interaction.md)
  - [ch03-04.i2c总线设备](./src/ch03-04.i2c_bus_device.md)
  - [ch03-05.spi总线和设备](./src/ch03-05.spi_bus_device.md)
  - [ch03-06.网络设备](./src/ch03-06.net_device.md)
  - [ch03-07.块设备](./src/ch03-07.block_device.md)

- [ch04.Linux应用开发设计](./src/ch04-00.application_design.md)
  - [ch04-01.Linux应用API接口](./src/ch04-01.linux_api_interface.md)
  - [ch04-02.Linux设备访问接口](./src/ch04-02.embed_linux_device_interface.md)
  - [ch04-03.Linux平台GUI显示设计](./src/ch04-03.embed_linux_gui_design.md)
  - [ch04-04.Linux平台应用设计](./src/ch04-04.embed_linux_app_design.md)

[ch05.结语](./src/ch05.CONCLUSION.md)
