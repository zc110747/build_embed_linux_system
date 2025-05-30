# Linux面试题整理(一)：Linux系统基础面试问题整理

学习嵌入式Linux，肯定是为了找工作或者解决工作中的难题，这里整理一些Linux面试题，有网络上收集的，也有自己在面试或者工作中遇到的问题。

- [内核空间和用户空间说明，区别有哪些](#interview-001)
- [内核启动中发生panic，如何分析解决](#interview-002)
- [内核中内存申请相关的函数，功能和区别说明](#interview-003)
- [(一致性问题，DMA)内核中内存分配的函数带不带Cache，如果带Cache，何种情况会出现一致性问题，如何解决](#interview-004)
- [简述内存分配的页面置换算法](#interview-005)
- [Linux系统中常用的动态内存分配算法有哪些，简要说明。](#interview-006)
- [在U-Boot烧录进去后，上电后串口上没有任何输出，硬件和软件各应该去检查什么？](#interview-030)
- [描述下系统的启动流程，有没有优化过启动流程的经历，详细说明下](#interview-031)
- [(Bringup)有没有做过芯片bringup，说说主要做了什么工作?](#interview-032)
- [有没有修改过系统启动中的Logo，实现过开机进度条，简述下实现方法](#interview-033)
- [内核的核心组成部分有哪些，简要描述各模块功能](#interview-034)

## kernel

### interview-001

什么是内核空间，什么是用户空间，区别有哪些。

1. 内核空间是操作系统内核运行的区域，包含内核代码和数据结构。
2. 用户空间是用户程序运行的区域，它包含了用户程序代码和数据。
3. 主要区别包含访问权限，代码执行处理，数据隔离以及上下文切换开销的差别
    - 内核空间具有最高的访问权限，可以访问所有硬件资源，用户空间具有较低的访问权限，需要通过系统调用来访问内核空间
    - 内核空间代码执行出错后，通常会panic，导致系统崩溃，用户空间代码执行出错后，通常会终止或返回错误码
    - 内核空间数据是共享的，用户空间数据是独立的
    - 用户空间切换到内核空间，需要保存用户空间的寄存器状态，切换内核态，加载内核的页面等操作，开销大，用户空间的进程切换开销较小，因为共享相同的内核空间

### interview-002

内核启动中发生panic，如何分析解决。

### interview-003

内核中内存申请相关的函数，功能和区别说明。

1. kmalloc函数，用于内核空间申请较小内存(通常不超过128KB)，分配的内存在物理上是连续的，虚拟地址上自然也是连续的。其中flag指定内存分配的特性，GFP_KERNEL（进程可以睡眠，适用于进程上下文）、GFP_ATOMIC（进程不能睡眠，适用于中断处理函数、tasklet等原子上下文）等
2. kzalloc函数，与kmalloc()类似，但分配的内存块会被初始化为0
3. vmalloc函数，用于分配大块内存，分配的虚拟内存是连续的，但物理内存可以不连续
4. get_free_pages和__get_free_page函数，用于分配特定数量的连续物理页（以2的幂为单位）
5. alloc_pages和 __alloc_pages函数，这些函数返回一个指向struct page结构体的指针列表，该结构体包含了关于分配页面的更多信息。这通常用于更底层的内存管理操作。
6. alloc_page和 __alloc_page函数，分配一个单独的物理页，并返回一个指向struct page的指针

### interview-004

内核中内存分配的函数带不带Cache，如果带Cache，何种情况会出现一致性问题，如何解决。

1. 对于Linux内存分配的函数，其中kmalloc、kzalloc、vmalloc等函数都带有Cache。对于直接与硬件交互的设备，需要不带Cache的内存，以DMA为例，其申请内存的函数dma_alloc_coherent和dma_alloc_attrs_coherent，这两个函数申请的内存不带Cache
2. 对于带Cache的内存，产生一致性问题主要有两种情况
    - 与硬件设备进行交互，硬件直接访问物理内存，例如CPU通过Cache访问内存，DMA直接访问物理内存，此时借助DMA（直接内存访问）同步函数来保证一致性。CPU读取时调用dma_sync_single_for_cpu函数，无效Cache；CPU写入时调用dma_sync_single_for_device，将Cache同步到物理内存，从而确保DMA能读取到最新的数据
    - 多个CPU访问同一块内存，需要借助CPU Cache一致性协议（如MESI、MOESI等）来保证Cache的一致性。CPU访问内存时，会根据协议规则更新Cache状态，确保各个CPU看到的Cache数据是一致的。MESI协议定义了四种缓存行状态，分别是Modified（修改）、Exclusive（独占）、Shared（共享）和Invalid（无效）。
        - Modified状态，处理器直接从缓存读和写数据
        - Exclusive，处理器直接从缓存读数据，写数据则更新为Modified，并在缓存种修改数据
        - shared, 处理器直接从缓存读数据，处理器会向总线发送一个Invalidate请求，将其他处理器中相同缓存行的状态置为Invalid，然后将自己的缓存行状态更新为Modified，并在缓存中修改数据
        - Invalid，读取数据时，处理器会发起一个总线请求，从主内存或其他处理器的缓存中获取最新的数据，并更新缓存行的状态。写入数据时，处理器会发起一个总线请求，从主内存或其他处理器的缓存中获取最新的数据，将缓存行状态更新为Modified，并在缓存中修改数据。

### interview-005

简述内存分配的页面置换算法

1. 最优置换算法（Optimal Replacement，OPT），选择将来不再使用的页面或者在最远的将来才被访问的页面进行置换
2. 先进先出算法（First-In-First-Out，FIFO），选择最早加载到内存中的页面进行置换
3. 最近最少使用算法（Least Recently Used，LRU），选择最近最久没被访问的页面进行置换
4. 时钟算法（Clock Algorithm），是LRU算法的一种近似算法。它将所有的页面按照进入内存的先后次序用链接指针链成一个循环队列，用一个指针指向该队列。进行页面置换时，操作系统扫描该循环队列，查找第一个访问位被置为0的页用于置换
5. 改进的时钟算法，在时钟算法的基础上，为每个页面增设一个修改位，用于记录页面是否被修改过。在选择用于置换的页时，把既未访问过又未修改过的页面作为首选

### interview-006

Linux系统中常用的动态内存分配算法有哪些，简要说明。

1. 首次适应算法（First Fit）：从空闲分区链表的头部开始查找，找到第一个能够满足分配请求的空闲分区。优点分配速率块，缺点会产生大量的外部碎片
2. 最佳适应算法（Best Fit）：从空闲分区链表中找到能够满足分配请求且大小最小的空闲分区，优点是减少内存浪费，缺点是产生细小碎片，维护排序开销大
3. 最坏适应算法（Worst Fit）：从空闲分区链表中找到能够满足分配请求且大小最大的空闲分区，优点是减少剩余碎片大小，缺点是内存消耗快
4. 邻近适应算法（Next Fit）：从上次查找结束的位置开始查找，找到第一个能够满足分配请求的空闲分区，优点是分配更均匀，缺点是缺乏系统性的地址规划

### interview-030

在系统烧录进去后，上电后串口上没有任何输出，硬件和软件各应该去检查什么？

硬件上检查如下。

1. 控制启动的拨码开关或电平是否正确(一般烧录时和加载时是不一致的)，判断从正确的地址加载启动流程
2. 检测系统的各部分供电是否正常(CPU、DRAM、存储器等)
3. 检测DRAM的工作时钟和初始化数据是否正确通讯
4. 检查串口的连接，是否连接正确的引脚，相关器件是否正常

软件上的检查

1. 检查设备树中指定调试的串口和硬件上是否一致
2. 如果一致，在U-Boot源码include/common.h中增加DEBUG宏定义，打印更早的系统信息，再进行分析
3. 检查软件串口的设置是否和U-Boot中配置一致，波特率，数据位，停止位，奇偶校验位配置

### interview-031

描述下Linux系统的启动流程，有没有优化过启动流程的经历，详细说明下。

Linux的启动流程如下。

- 可选的启动第一阶段(U-Boot SPL)
  - 内部RAM加载，执行SPL程序，主要用于初始化外部DRAM，加载U-Boot程序到外部DRAM中，并跳转执行。
  - 可选的安全和固件签名认证功能
- 引导加载程序启动(U-Boot)
  - 初始化硬件设备，包括CPU、内存、中断控制器、定时器等
  - 加载内核镜像到DRAM中，跳转执行
- 内核启动(Kernel)
  - 初始化需求的硬件设备
  - 挂载根文件系统，并启动起始进程
- 文件系统挂载和执行，以SystemV系统为例(Rootfs)
  - 内核挂载文件系统后，后首先执行init进程
  - init进程会加载系统的基本服务(列在/etc/inittab中)，如网络服务，文件系统服务，进程管理等，以及初始化脚本(/etc/init.d/rcS)

对于启动流程优化，一般指从系统上电，到正式程序加载的时间，包含U-Boot的启动时间，内核启动时间，必要服务的加载时间，然后开始执行应用，对于启动时间的优化，主要在于分析主要的耗时点，对应解决。

1. U-Boot的加载时间很短，不过在结束时有bootdelay，会进行延时等待，可以在正式版编译时U-Boot时修改为0
2. Kernel的加载时间主要是必要的组件和驱动加载，其中必要的组件因为功能需求很难优化，可以优化驱动的加载，裁剪未使用的模块，例如如果内核未使用rtc，can这类解决，都可以在configs/中注释，并将对应的设备树状态设置为disable
3. 文件系统的优化，对于QT构成的应用，可以直接使用buildroot这类简单的文件系统(不要选则debain，ubuntu这类复杂系统)，在必要的服务和驱动加载后，就可以执行程序

### interview-032

有没有做过芯片bringup，说说主要做了什么工作?

关于u-boot的适配，主要进行了官方SDK对产品硬件的适配，主要工作内容。

1. 确定官方SDK对应开发板和产品硬件的差异，确定需要修改的内容
2. 修改arch/arm/dts下的设备树文件，来适配硬件内容
3. 在board/目录下仿照官方的硬件平台，实现对应自己的平台的硬件文件
4. 修改configs/目录下的默认配置文件，定义u-boot支持的功能
5. 通过make menuconfig，在UI界面中配置和裁剪需要的功能，编译下载既是适配后的U-Boot

关于kernel的适配，和u-boot流程类似。

1. 确定官方SDK对应开发板和产品硬件的差异，确定需要修改的内容
2. 修改arch/arm/boot/dts下的设备树文件，来适配硬件内容
3. 修改arch/arm/configs下的配置文件，定义kernel支持的功能
4. 通过make menuconfig，在UI界面中配置和裁剪需要的功能，编译下载既是适配后的kernel

### interview-033

有没有修改过系统启动中的Logo，实现过开机进度条，简述下实现方法。

系统启动过程中，对于U-Boot，Kernel，以及文件系统都可以实现添加自己的Logo，实现的方法如下所示。

- U-Boot中的logo是在编译过程中链接到固件中，运行时从指定地址加载的
  - 在U-Boot环境变量中通过splashimage变量指定地址
  - 在编译U-Boot时通过tools/bmp_logo工具生成include/bmp_logo_data.h，启动时复制到splashimage地址，之后加载显示
  - 修改图片则需要替换tools/logos/*下对应图片，即可在运行时替换显示
- kernel中的logo也是直接链接到固件中，定义在drivers/video/logo目录下
  - 选中和频率分辨率一致的图片，通过命令行转换成ppm格式
  - 添加到drivers/video/logo目录下，并添加同目录下的logo.c代码中，并修改配置项和Makefile，选中添加的logo
  - 重启设备，此时添加的logo就会显示
- 文件系统时应用已经执行，这个步骤一般加载的是启动动作条，而非logo，可以通过如下方式实现
  - 移植splash开机进度条软件，通过命令实现启动动作条
  - 使用QT通过logo图片加动作进度的方式实现开机进度条

上面只是对logo的修改大致方法进行说明，详细的实现还需要根据版本进行调整，但原理都是一致的。

### interview-034

内核的核心组成部分有哪些，简要描述各模块功能

Linux内核的核心组成部分包括进程调度，内存管理，虚拟文件系统，网络协议栈、进程间通讯IPC机制，以及设备驱动等。

1. 进程调度，负责管理系统中的进程，根据进程的优先级和资源需求，分配CPU时间片，实现进程的切换和调度
2. 内存管理，负责管理系统中的内存资源，包括内存分配，内存回收，内存保护等
3. 虚拟文件系统，负责管理文件系统，提供统一的接口，支持多种文件系统，如ext4，fat32等
4. 网络协议栈，负责管理网络协议，实现网络通信
5. 进程间通讯IPC机制，负责进程间的通信，包括管道，消息队列，信号量，共享内存等
6. 设备驱动，负责管理硬件设备，实现对硬件设备的访问和控制
