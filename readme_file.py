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

        for file in filelist:
            with open(file, 'r', encoding='utf-8') as file:
                for line in file:
                    matches = re.findall(r'\((.*?)\)', line)
                    for match in matches:
                        if match.startswith("./ch"):
                            match_src = match[:2] + 'src/' + match[2:]
                            if not os.path.exists(file_name):
                                print(f"文件{file}:{match_src} 不存在。")

    except FileNotFoundError:   
        print(f"文件 {file_path} 未找到。")
    except Exception as e:
        print(f"读取文件时发生错误: {e}")

if __name__ == "__main__":
    target_file_path = 'README.md'
    find_content_in_parentheses(target_file_path)