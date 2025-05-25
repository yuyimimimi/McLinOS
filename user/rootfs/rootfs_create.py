#!/usr/bin/env python3
import os
import random
import string
import shutil

def generate_random_name(filename):
    """生成随机前缀+原文件名的名称"""
    rand_str = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
    return f"{rand_str}_{os.path.basename(filename)}.c"

def file_to_hex_array(filepath):
    """将文件内容转换为十六进制数组字符串"""
    with open(filepath, 'rb') as f:
        content = f.read()
    
    hex_str = ','.join([f"0x{byte:02x}" for byte in content])
    return hex_str, len(content)

def process_file(input_path, output_dir, root_dir):
    """处理单个文件"""
    # 生成输出文件名
    output_filename = generate_random_name(input_path)
    output_path = os.path.join(output_dir, output_filename)
    
    # 计算文件系统中的绝对路径
    rel_path = os.path.relpath(input_path, root_dir)
    fs_path = f"/{rel_path.replace(os.sep, '/')}"  # 转换为Linux路径格式
    
    # 转换文件内容为十六进制数组
    hex_data, data_len = file_to_hex_array(input_path)
    
    # 生成C文件内容
    c_content = f"""#include <linux/initramfs.h>

static const char filedata[] = {{
    {hex_data}
}};

register_file("{fs_path}", {data_len}, filedata, READONLY);
"""
    
    # 写入输出文件
    with open(output_path, 'w') as f:
        f.write(c_content)

def main():
    # 设置输入输出目录
    current_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(current_dir, "../rootfs_file_out")
    
    # 清空并创建输出目录
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)
    
    # 遍历当前目录及其子目录
    for root, dirs, files in os.walk(current_dir):
        # 跳过输出目录
        if os.path.abspath(root).startswith(os.path.abspath(output_dir)):
            continue
            
        for filename in files:
            # 跳过Python脚本本身
            if filename.endswith('.py'):
                continue
                
            input_path = os.path.join(root, filename)
            process_file(input_path, output_dir, current_dir)
    
    print(f"save into {output_dir}")

if __name__ == "__main__":
    main()

