import os
import re
from os import walk

# 标签貌似是不区分大小写的
# 标签log_br只在文本出现两次，有一次携带了文本，目前暂时不知道有没有用
skip_tag_list = ['<VOICE', '<WIPE', '<BOX', '<face', '<clear', '<se', '<BACKGROUND', '<box', '<LAYER',
                 '<BUSTUP', '<background', '<bgm', '<wait', '<FLASH', '<A', '<quake', '<bgmovie', '<MARKER',
                 '<MOVIE', '<CALC', '<SE', '<CLEAR', '<LIST_CHOICE', '<system', '<scroll',
                 '<SYSTEM', '<SAVE', '<コマンド', '<BGM', '<WAIT', '<SUSPEND_BGM', '<SELECT', '<layer',
                 '<!', '<INCLUDE', '<LOAD', '<END', '<SCROLL', '<end', '<log_br', '<RESUME_BGM',
                 '<CHOICE_DETAIL']

# '//' 注释也要排除
# <LOG_BR> 特殊！！！！！
# 包含文本的标签
tag_text = ['<CHOICE', '<R', ]

# 这里的路径写死
path = '/Users/madongyu/Downloads/QQ File/nps'  # 原先的shift_jis 编码的脚本文件，即script.npk解包出来的文件
out_dic = os.path.join(path, '../format')  # utf8 格式的脚本文件，用于等下提取文本
chs_dic = os.path.join(path, '../chs')  # 需要翻译的文件，将utf8 格式的脚本文件中的日文提取得到
out_chs_dic = os.path.join(path, '../format_chs')  # 将译文填入的脚本文件的输出文件列表


# 将脚本文件转为utf8
def transform_utf8():
    dic = walk(path)
    for myPath, dir_list, file_list in dic:
        if not os.path.exists(out_dic):
            os.mkdir(out_dic)
        for filename in file_list:
            if filename.endswith('.txt') or filename.endswith('.nps'):
                try:
                    with open(os.path.join(path, filename), 'r', encoding='shift_jis') as f1, open(
                            os.path.join(out_dic, filename), 'w+', encoding='utf-8') as f2:
                        f2.write(f1.read())
                except UnicodeDecodeError:
                    print(filename + 'encode error')
            else:
                print(filename + ' is not script')


# 统计标签出现次数
def cal_tag():
    tags = {}
    dic = walk(out_dic)
    for myPath, dir_list, file_list in dic:
        for filename in file_list:
            with open(os.path.join(myPath, filename), 'r', encoding='utf-8') as f1:
                line = f1.readline()
                while line:
                    line.strip()
                    if line.startswith('<'):
                        lines = line.split(' ', 1)
                        if len(lines) == 1:
                            key = lines[0].split('>')[0]
                        else:
                            key = lines[0]
                        key = key + '>'
                        key = key.strip()
                        if key in tags:
                            tags[key] += 1
                        else:
                            tags[key] = 1
                    line = f1.readline()
    temp = []
    for i in sorted(tags.items(), key=lambda x: x[1], reverse=True):
        temp.append(i[0])
    print(temp)
    print(len(temp))
    print(tags)


# 将utf的脚本文件转化为汉化用文件
# 排除不以 d ,ep 开头的文件
def utf82chs():
    size = 0
    dic = walk(out_dic)
    for myPath, dir_list, file_list in dic:
        if not os.path.exists(chs_dic):
            os.mkdir(chs_dic)
        for filename in file_list:

            if filename.startswith('ep') or filename.startswith('d'):
                with open(os.path.join(myPath, filename), 'r', encoding='utf-8') as f1, open(
                        os.path.join(chs_dic, filename), 'w+', encoding='utf-8') as f2:

                    f2.write('//【翻译规范】\n')
                    f2.write('//原文例子 ●●●[19]●●● xxxx\n')
                    f2.write('//译文例子 ○○○[20]○○○ xxxx\n')
                    f2.write('//译文长度没有要求，不需要和原文保持一致\n')
                    f2.write('//原文中出现的<K>特殊标签，其作用是让<K>后面的文本和前面的文本可以在同一个对话框里显示，在不影响语义的情况下建议保留\n')
                    f2.write('//原文中出现的<R>为注音标签，里面的TEXT可以进行翻译，如果觉得没必要的话可以选择删掉，只保留里面的词语。如<R TEXT="ソラ">虚空</R> => 虚空\n')
                    f2.write(
                        '//原文中出现的<CHOICE>为选项，如 <CHOICE HREF="#最初" TEXT="プロローグから始める" OPERATOR=""></A> '
                        '，这里只需要对TEXT进行翻译即可，译文应该为 <CHOICE HREF="#最初" TEXT="从序章开始" OPERATOR=""></A>  \n')
                    f2.write('//翻译过程如发现文本存在问题或者有疑惑的地方，可以联系本人QQ：2115601599，或者发邮件到2115601599@qq.com咨询\n\n\n')

                    line = f1.readline()
                    count = 1  # 第几行
                    while line:

                        line.strip()

                        can_write = True

                        if line == '\n' or line == '　\n':
                            can_write = False
                        elif line.startswith('<'):
                            lines = line.split(' ', 1)
                            if len(lines) == 1:
                                key = lines[0].split('>')[0]
                            else:
                                key = lines[0]
                            key = key
                            key = key.strip()
                            can_write = (key not in skip_tag_list)
                        elif line.startswith('//'):
                            can_write = False
                        if can_write:
                            size += len(line)
                            f2.write('●●●[{0}]●●●{1}'.format(count, line))
                            f2.write('○○○[{0}]○○○\n'.format(count))
                            f2.write('\n')
                        count += 1
                        line = f1.readline()
            else:
                print(filename + ' not transfer')
    print(size)


# 将译文写入脚本文件
def chs2utf8():
    dic = walk(chs_dic)
    for myPath, dir_list, file_list in dic:
        if not os.path.exists(out_chs_dic):
            os.mkdir(out_chs_dic)
        for filename in file_list:
            with open(os.path.join(out_dic, filename), 'r', encoding='utf-8') as origin, open(
                    os.path.join(chs_dic, filename), 'r', encoding='utf-8') as translation, open(
                    os.path.join(out_chs_dic, filename), 'w+', encoding='utf-8') as f3:
                if filename.startswith('ep') or filename.startswith('d'):
                    o_line = origin.readline()
                    count = 1
                    t_line = translation.readline()
                    while t_line:
                        t_line.strip()
                        if not (t_line == '\n' or t_line.startswith('●') or t_line.startswith('//')):
                            res = re.split(r']○○○', t_line, 1)
                            assert len(res) == 2
                            num = int(res[0][4:])
                            # assert num != 1 # 排除
                            text = res[1]
                            while o_line:
                                # 遍历原来的文件，将改行之前的内容全部写入
                                if num == count:
                                    # 这里读取一行意味着舍弃了原文那一行
                                    count += 1
                                    o_line = origin.readline()
                                    break
                                else:
                                    f3.write(o_line)
                                    count += 1
                                    o_line = origin.readline()
                            f3.write(text)
                        t_line = translation.readline()
                    # 继续写文件
                    while o_line:
                        f3.write(o_line)
                        count += 1
                        o_line = origin.readline()
                else:
                    # 不需要翻译的文件直接读取原文
                    f3.write(origin.read())


# 检查文件的行数是否对应成功
def check():
    dic = walk(out_chs_dic)
    for myPath, dir_list, file_list in dic:
        for filename in file_list:
            if filename.startswith('ep') or filename.startswith('d'):
                with open(os.path.join(out_chs_dic, filename), 'r', encoding='utf-8') as f1, open(
                        os.path.join(out_dic, filename), 'r', encoding='utf-8') as f2:
                    o1 = f1.readline()
                    o2 = f2.readline()
                    while o1 and o2:
                        o1 = f1.readline()
                        o2 = f2.readline()
                    assert not o1
                    assert not o2

# transform_utf8()
# cal_tag()
# utf82chs()
# chs2utf8()
# check()
# test = '○○○[123]○○○阿萨德流量卡时间段里看见;la'
# res = re.split(r']○○○', test, 1)
# num = res[0][4:]
# print(num)
# print(res[1])
# print(res)
