# step1: make查找all标签，并执行target.
all : target                                                  

# step2: 查找target标签，执行hello/hello.o，main.o.
target:hello/hello.o main.o                                 
# step5: 链接文件，生成可执行文件
	gcc -o target hello/hello.o main.o 

# step3: 执行hello.o标签，查找hello.c文件，执行下述命令
hello/hello.o:hello/hello.c                                 
	gcc -I"hello/" -c hello/hello.c -o hello/hello.o

# step4: 执行main.o标签，查找main.c文件,执行下述命令，生成链接文件
main.o:main.c                                               
	gcc -I"hello/" -c main.c -o main.o

clean:
	rm -rf target
	rm -rf hello/hello.o main.o