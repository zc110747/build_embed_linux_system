# /usr/bin/env python3
# -*- coding: utf-8 -*

import os
import re

# detect file wheather exists
def find_content_in_parentheses(file_path):
    try:
        all_files_exist = True

        filelist = []
        with open(file_path, 'r', encoding='utf-8') as file:
            for line in file:
                matches = re.findall(r'\((.*?)\)', line)
                for match in matches:
                    if match.startswith("./src/"):
                        file_name = match[2:]
                        if not os.path.exists(file_name):
                            all_files_exist = False
                            print(f"文件 {file_name} 不存在。")
                        else:
                            filelist.append(match)

        print(f"共找到 {len(filelist)} 个文件。")

        for file in filelist:
            with open(file, 'r', encoding='utf-8') as file:
                for line in file:
                    matches = re.findall(r'\((.*?)\)', line)
                    
                    for match in matches:

                        # 检查引用的主文件 ./ch0x-xx.xx.md
                        if match.startswith("./ch"):
                            # 去除#号后的标题, 获取文件名
                            if '#' in match:
                                match = match.split('#')[0]

                            # print(match)
                            match_src = match[:2] + 'src/' + match[2:]
                            if not os.path.exists(match_src):
                                print(f"文件{file}:{match_src} 不存在。")
                                all_files_exist = False

                        # 检查引用的图片文件，格式为./image/xx.png  
                        if match.startswith("./image"):
                            match_src = match[:2] +'src/' + match[2:]
                            if not os.path.exists(match_src):
                                print(f"文件{file}:{match_src} 不存在。")
                                all_files_exist = False

                        # 检查引用的工程文件 ./file/ch0x-xx/[directory，file]
                        if match.startswith("./file"):
                            match_src = match[:2] +'src/' + match[2:]

                            if not os.path.exists(match_src):
                                print(f"文件{file}:{match_src} 不存在。")
                                all_files_exist = False
        if all_files_exist == True:
            print("所有文件都存在! ")
        else:
            print("存在文件不存在，请检查!。")
    
    except FileNotFoundError:   
        print(f"文件 {file_path} 未找到。")
    except Exception as e:
        print(f"读取文件时发生错误: {e}")

if __name__ == "__main__":
    target_file_path = 'README.md'
    find_content_in_parentheses(target_file_path)