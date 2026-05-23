# 拷贝到根目录去然后直接 python GB2312转UTF8.py
# 舒服死了，再也不用一个个转了，该死的 Keil
import os
import codecs

def convert_files_to_utf8(directory, extensions=('.c', '.h')):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(extensions):
                file_path = os.path.join(root, file)
                try:
                    # 尝试用GB2312读取
                    with codecs.open(file_path, 'r', 'gb2312') as f:
                        content = f.read()
                    # 用UTF-8写入
                    with codecs.open(file_path, 'w', 'utf-8') as f:
                        f.write(content)
                    print(f"转换成功: {file_path}")
                except Exception as e:
                    print(f"转换失败 {file_path}: {e}")

# 使用当前目录
convert_files_to_utf8('.')
