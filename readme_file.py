# /usr/bin/env python3

import os
import re

# 用于查找README.md中文件是否存在，如果不存在则输出提示
def find_content_in_parentheses(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            for line in file:
                # 使用正则表达式查找括号内的内容
                matches = re.findall(r'\((.*?)\)', line)
                for match in matches:
                    if match.startswith("./src/"):
                        file_name = match[2:]
                        if not os.path.exists(file_name):
                            print(f"文件 {file_name} 不存在。")
    except FileNotFoundError:
        print(f"文件 {file_path} 未找到。")
    except Exception as e:
        print(f"读取文件时发生错误: {e}")

if __name__ == "__main__":
    target_file_path = 'README.md'
    find_content_in_parentheses(target_file_path)