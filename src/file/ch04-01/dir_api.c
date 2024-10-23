/*
dir接口
opendir 打开指定目录，获取目录指针
mkdir   创建目录
readdir 读取子目录的句柄
closedir 关闭打开的目录
*/
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#define TEST_DIR "test"

int process_dir(void)
{
    DIR *pDir;
    struct dirent *pdirent;

    pDir = opendir(TEST_DIR);
    if(pDir == NULL)
    {
        printf("dir %s not exist, create!", TEST_DIR);
        mkdir(TEST_DIR, 0755);
        pDir = opendir(TEST_DIR);
        if(pDir == NULL)
        {
            printf("dir %s not exist, failed!", TEST_DIR);
            return -1;
        }
    } 
    else
    {
        printf("dir exist, info:%s\n", TEST_DIR);
    }

    while((pdirent = readdir(pDir)) != NULL)
    {
        printf("%s\n", pdirent->d_name);
    }
    closedir(pDir);

    return 0;
}