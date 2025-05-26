import argparse
import os

def replace_in_file(file_path, old_str, new_str):
    try:
        # 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
        
        # 执行替换操作
        new_content = content.replace(old_str, new_str)
        
        # 如果内容有变化则写入文件
        if new_content != content:
            with open(file_path, 'w', encoding='utf-8') as file:
                file.write(new_content)
            print(f"成功替换文件：{file_path}")
        else:
            print(f"未找到匹配内容：{file_path}")
            
    except Exception as e:
        print(f"处理文件 {file_path} 时出错：{str(e)}")

def main():
    parser = argparse.ArgumentParser(
        description="批量文件字符替换工具",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="使用示例：\n"
               "  python replace.py hello.c aaa bb\n"
               "  python replace.py *.txt old_text new_text"
    )
    parser.add_argument('files', nargs='+', help="要处理的文件列表（支持通配符）")
    parser.add_argument('old_str', help="需要被替换的旧字符串")
    parser.add_argument('new_str', help="替换后的新字符串")
    
    args = parser.parse_args()
    
    # 处理每个文件
    for file_path in args.files:
        if os.path.isfile(file_path):
            replace_in_file(file_path, args.old_str, args.new_str)
        else:
            print(f"文件不存在或不是普通文件：{file_path}")

if __name__ == "__main__":
    main()

