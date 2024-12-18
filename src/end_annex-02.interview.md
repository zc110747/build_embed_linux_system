# 附录二：Linux面试题整理

学习嵌入式Linux，肯定是为了找工作或者解决工作中的难题，这里整理一些Linux面试题，用于快速整理学习。

- [Linux驱动问题整理](#linux-driver)
  - [Linux设备分类，阐述它们的区别，并分别列举一些实际设备](#interview-01-001)
  - [Linux中引入模块机制的好处](#interview-01-002)
  - [说明驱动模块的操作有哪些命令，加载和移除对应驱动中的接口](#interview-01-003)
  - [查看驱动打印信息的命令，如何查看内核中已注册的字符设备和中断号](#interview-01-004)
  - [字符型驱动设备创建设备文件的方法](#interview-01-005)
  - [简述主设备号和次设备号的用途。执行mknod chardev c 4 46，请分析使用的是那一类设备驱动程序](#interview-01-006)
  - [驱动注册字符设备的相关函数和接口说明](#interview-01-007)
  - [简述copy_to_user和copy_from_user主要功能，一般用于file_operations结构的哪些函数](#interview-01-008)
  - [简述设备驱动模型构成，platform总线的匹配规则，注册驱动和注册设备需要先后顺序吗](#interview-01-009)
  - [简述下Linux中断机制，注册和注销中断的相关函数。中断和轮询哪个效率高？怎样决定是采用中断方式还是采用轮询方式去实现驱动](#interview-01-010)
  - [IRQ(Interrupt Request)和FIQ(Fast Interrupt Request)在CPU实现中有什么区别](#interview-01-011)
  - [中断的为啥分上半部分和下半部分，讲下如何实现](#interview-01-012)
  - [linux内核里面，内存申请相关的函数，功能和区别说明](#interview-01-013)
  - [驱动使用什么函数接口访问绝对物理地址，为什么需要](#interview-01-014)
- [Linux应用操作整理](#linux-app)
  - [函数mmap的实现原理，机制和应用](#interview-02-001)

## linux-driver

### interview-01-001

Linux设备分类，阐述它们的区别，并分别列举一些实际设备。

1. Linux主要的设备包含字符设备，块设备和网络设备
2. 字符设备与块设备的主要区别体现在数据处理方式、访问模式、是否使用缓冲区以及典型应用等方面，字符设备以字节单位进行处理，顺序访问，一般无需缓冲直接读取块设备以数据块为单位进行处理，随机访问，请求有对应的缓冲区
3. 常见字符设备有按键，键盘，串口，鼠标等，块设备有U盘，SD卡，EMMC，Nand-FLASH，网络设备则有网口，wifi，蓝牙，can等。

### interview-01-002

Linux中引入模块机制的好处。

1. 提高了系统的可扩展性，模块机制允许开发者在不需要重新编译整个内核的情况下，动态地向内核添加或移除功能
2. 优化内核大小，通过模块机制，Linux内核可以实现最小化，即内核中只包含一些基本功能，如模块管理、内存管理等

### interview-01-003

说明驱动模块的操作有哪些命令，加载和移除对应驱动中的接口。

驱动加载和移除的命令包含如下。

1. insmod，加载模块，执行驱动中的module_init指定的函数
2. rmmod，移除模块，执行驱动中的module_exit指定的函数
3. lsmod，查看系统已经加载的模块
4. depmod，扫描/lib/modules下的所有模块文件，会在/lib/modules/$(uname -r)目录下生成模块依赖关系
5. modprobe，提供了加载、卸载和查询内核模块的功能，根据生成的模块依赖关系，加载和移除相应的模块

### interview-01-004

查看驱动打印信息的命令，如何查看内核中已注册的字符设备和中断号。

1. 查看驱动模块打印信息使用dmesg命令，配合grep可以查询指定的驱动打印信息，当然也可以修改"/proc/sys/kernel/printk"打印所有信息查看
2. 通过"cat /proc/deivces"查看内核已注册的字符设备
3. 通过"cat /proc/interrupts"查看正在使用的中断号

### interview-01-005

字符型驱动设备创建设备文件的方法。

1. 使用mknod手动创建设备文件，mknod  device  c  MAJOR  MINOR
2. 在内核中使用class_create来创建设备类，然后通过device_create创建设备，关联到已经注册字符设备的设备号

### interview-01-006

简述主设备号和次设备号的用途。执行mknod chardev c 4 46，请分析使用的是那一类设备驱动程序。

1. 主设备号用于区分不同种类的设备，次设备号用于区分同一个类型的多个设备
2. 常见字符设备主设备号代表类型， 1-内存设备，4-串口tty设备，5-console设备，10-杂项设备，13-输入设备，因此chartest主设备号为4，表示串口tty设备

### interview-01-007

驱动注册字符设备的相关函数和接口说明。

注册字符设备的相关函数包含如下。

```c
//根据范围注册申请一组设备号(from为起始的主从设备号)，仅申请设备号，设备后续创建
int register_chrdev_region(dev_t from, unsigned count, const char *name);

//动态申请一组设备号值
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)

//初始化字符型设备结构体
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
- cdev: 字符设备结构体
- fops: 设备支持的访问接口结构体

//将申请设备号和字符型设备功能结构在内核中添加并绑定
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
- p: 字符设备结构体
- dev: 关联设备号
- count: 添加内核的设备数目，用于分配连续的次设备号
```

### interview-01-008

简述copy_to_user和copy_from_user主要功能，一般用于file_operations结构的哪些函数。

1. copy_to_user和copy_from_user是Linux内核中的关键函数，它们主要用于在用户空间和内核空间之间安全地传输数据，其中copy_to_user用于用于将数据从内核空间复制到用户空间，copy_from_user函数用于将数据从用户空间复制到内核空间。
2. file_operation中的ioctl，read, write函数会用到这两个函数，用于实现应用层和内核之间的数据交互。

### interview-01-009

简述设备驱动模型构成，platform总线的匹配规则，注册驱动和注册设备需要先后顺序吗？

1. 设备驱动模型由总线，设备，驱动三部分组成
2. Platform总线是Linux内核中用于管理不直接挂接在物理总线上的设备（如LED、LCD、RTC等）的一种虚拟总线，其主要匹配支持platform访问的节点(一般为根节点，或者在其compatible属性中包含"simple-bus", "simple-mfd", "isa", "arm,amba-bus"节点的子节点)，与可访问设备字节中的compatible属性字符串匹配则加载驱动。
3. 对于片上设备和大部分板级设备，都是需要先注册设备，再注册驱动。部分支持热插拔的设备(USB，HDMI)等，则是先注册驱动，当设备插入时才注册设备，匹配驱动执行

### interview-01-010

简述下Linux中断机制，注册和注销中断的相关函数。中断和轮询哪个效率高？怎样决定是采用中断方式还是采用轮询方式去实现驱动。

1. Linux中断机制是指当CPU在执行程序的过程中，出现了某些突发事件急待处理时，CPU会暂停当前程序的执行，转去处理突发事件，处理完毕后又返回原程序被中断的位置继续执行。对于Linux中断，一般由外设模块触发，上报对应的中断控制器GIC，最终由相应的CPU内核进行处理
2. 注册和注销中断的相关函数主要有request_irq和free_irq，其中request_irq函数用于向内核注册一个中断处理函数，free_irq函数用于注销一个已经注册的中断处理函数
3. 中断是CPU处于被动状态下来接受设备的信号，具有即时响应和高效利用CPU的优点，轮询则是CPU主动去查询设备是否有请求，实现相对简单。大部分设备请求频率较低，适合使用中断的方式请求；对于频繁请求CPU的设备或者有大量数据请求的网络设备，可能需要CPU频繁地处理数据，轮询反而效率更高。

### interview-01-011

IRQ(Interrupt Request)和FIQ(Fast Interrupt Request)在CPU实现中有什么区别。

1. 优先级。FIQ具有更高的优先级，当FIQ发生时，CPU会在当前指令执行完成后立即响应FIQ中断，而忽略其他IRQ中断
2. 上下文保存。FIQ模式会保存完整的CPU上下文，包括更多的寄存器状态，以确保在中断处理完成后能够恢复到正确的执行状态。以确保在中断处理完成后能够恢复到正确的执行状态
3. 使用场景。FIQ通常用于更重要或时间敏感的中断处理，如时钟中断、外部硬件错误等，IRQ通常用于外部中断处理，如键盘、鼠标等
4. FIQ使用专用的FIQ栈来存储中断处理过程中的数据，IRQ一般使用主CPU栈

### interview-01-012

中断的为啥分上半部分和下半部分，讲下如何实现。

1. 中断分为上半部分和下半部分主要是为了提高系统效率和响应性，同时尽量减少中断处理对其他任务的干扰。这种设计允许中断处理程序快速响应中断并处理最关键和紧急的任务（上半部），而将那些不那么紧急或耗时的任务推迟到稍后执行。
2. 上半部是中断处理的第一阶段，它通常在硬件中断触发后立即执行。上半部的主要任务是：尽快响应中断，处理最关键和紧急的任务，必须在中断上下文中执行，执行速度要快，不能阻塞（如不能调用睡眠函数），不能抢占（不会被其他中断打断）。
3. 下半部是在中断响应结束后执行的，它负责处理那些不那么紧急的任务。下半部可以延迟执行，因此可以在更合适的时机被调度。下半部可以使用软中断，tasklet，workqueue等实现

### interview-01-013

linux内核里面，内存申请相关的函数，功能和区别说明。

1. kmalloc函数，用于内核空间申请较小内存(通常不超过128KB)，分配的内存在物理上是连续的，虚拟地址上自然也是连续的。其中flag指定内存分配的特性，GFP_KERNEL（进程可以睡眠，适用于进程上下文）、GFP_ATOMIC（进程不能睡眠，适用于中断处理函数、tasklet等原子上下文）等
2. kzalloc函数，与kmalloc()类似，但分配的内存块会被初始化为0
3. vmalloc函数，用于分配大块内存，分配的虚拟内存是连续的，但物理内存可以不连续
4. get_free_pages和__get_free_page函数，用于分配特定数量的连续物理页（以2的幂为单位）
5. alloc_pages和 __alloc_pages函数，这些函数返回一个指向struct page结构体的指针列表，该结构体包含了关于分配页面的更多信息。这通常用于更底层的内存管理操作。
6. alloc_page和 __alloc_page函数，分配一个单独的物理页，并返回一个指向struct page的指针

### interview-01-014

驱动使用什么函数接口访问绝对物理地址，为什么需要。

1. 驱动中使用ioremap函数将物理地址映射到内核的虚拟地址空间中，使得内核可以通过虚拟地址来访问物理内存。
2. 现代操作系统中，为了提供进程隔离、隐藏内存细节、增强可扩展性和提供存储保护，通常都会引入内存管理单元（MMU）。MMU的主要功能包括虚拟到物理地址的转换和内存保护。在启用MMU的系统中，内核中操作的都是虚拟地址，而不是物理地址。要访问物理内存，就必须先将物理地址映射为虚拟地址。内核不能直接访问物理地址空间，而需要通过某种机制将物理地址映射到内核地址空间中，然后才能对物理内存进行访问。

```c
void __iomem *iomem; // 映射物理内存到虚拟地址空间
iomem = ioremap(DEVICE_PHYS_ADDR, DEVICE_SIZE);
```

## interview-009

简述在cs8900的驱动设计中, 发送和接收数据的过程，是否都需要关中断处理。

## interview-011

简单描述skbuff这个数据结构在网络结构中所起到的作用，为什么需要一个skbuff，它的分配和释放主要都在什么部位

## interview-014

自旋锁和信号量在互斥使用时需要注意哪些？在中断服务程序里面的互斥是使用自旋锁还是信号量？还是两者都能用？为什么

## interview-015

原子操作你怎么理解？为了实现一个互斥，自己定义一个变量作为标记来作为一个资源只有一个使用者行不行

## interview-023

驱动里面为什么要有并发、互斥的控制？如何实现？讲个例子？

## interview-024

spinlock自旋锁是如何实现的？

## interview-025

信号量简介

## interview-030

在u-boot烧录进去后，上电后串口上没有任何输出，硬件和软件各应该去检查什么？

硬件上检查如下。

1. 控制启动的拨码开关或电平是否正确(一般烧录时和加载时是不一致的)，判断从正确的地址加载u-boot
2. 检测CPU和DRAM和存储的的供电是否正常
3. 检查串口的连接，是否连接正确的引脚，相关器件是否正常

软件上的检查

1. 检查设备树中指定调试的串口和硬件上是否一致
2. 如果一致，在common.h中增加DEBUG宏定义，打印更早的系统信息，再进行分析
3. 检查软件串口的设置是否和U-Boot中配置一致，波特率，数据位，停止位，奇偶校验位配置

## linux-app

### interview-02-001

函数mmap的实现原理，机制和应用。

## interview-026

任务调度的机制？

## interview-028

在Linux C中，ls这个命令是怎么被执行的?

## interview-029

LINUX下的Socket套接字和Windows下的WinSock有什么共同点？请从C/C++语言
