# kishen_crack
機神咆吼デモンベイン Windows移植版 分析工具

-----

- 游戏引擎为nitroplus
- 资源包的格式为.npk
- 资源包采用的格式为NPK2，推测应该是16年之后nitro社采用的新的格式
- 资源包内的数据采用aes256加密和deflate无头压缩
- 脚本文件使用的是shift-jis编码，但是其他的系统文件，使用的是utf8
- 汉化的时候统一使用utf8编码，需要对游戏本体程序进行编码修改，具体则是对字符转换函数（在游戏初始化的时候调用的）[MultiByteToWideChar](https://docs.microsoft.com/zh-cn/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar?redirectedfrom=MSDN)的编码参数：932改成65001，具体编码表可以参考[微软的api文档](https://docs.microsoft.com/zh-cn/windows/win32/intl/code-page-identifiers)
-----

- 用到的c++库：zlib，boost，opensll
- 可以使用vsual studio 的 nuget 包管理工具安装，也可以使用vcpkg工具安装上述的库

-----

# npk文件内容

1. NPK2字符串开头（8个字节）（0x00补齐）
2. iv向量（16个字节）
3. 文件个数（4个字节）
4. 索引区域大小（4个字节）
5. 索引区域（由上面的区域大小决定读取的字节数）（使用aes256加密）
6. 文件数据

-----

# 索引解析

1. 数据结构

	```
	typedef struct segment {
		uint64_t offset;//文件在封包的偏移
		uint32_t seg_size;//区段在封包的大小
		uint32_t seg_ualg_size;//区段未经对齐的大小
		uint32_t seg_ucmp_size;//区段未经压缩的大小
	}Segment;

	typedef struct file
	{
		uint16_t name_len; // 文件名长度
		char* name;// 文件名
		char check[32];//校验值32字节，SHA256校验码
		uint32_t seg_count;//该文件包含多少个区段
		uint32_t uncomp_size;//文件未压缩之前的大小
		Segment* segments;
	} File;

	```
2. 读取出的文件索引数据解析

	- 一个0x00字节
	- 文件名长度（2个字节）
	- 文件名
	- 文件未压缩前的大小（4个字节）
	- 文件的sha256校验码（32个字节）
	- 文件分段的数目（4个字节）

	接下来就是分段内容的解析了，要解析 `文件分段数` 那么多的次数

	- 该分段在npk文件内容的位置（偏移地址）（8个字节）
	- 分段大小（4个字节）
	- 分段未对齐前的大小（4个字节）（aes256加密前需要对齐，补充至可以被16整除，如果本身可以被16整除，则再加16个字节）
	- 分段未压缩前的大小（4个字节）

3. 分段内容的解析
	- 首先是数据用aes256加密了，需要解密
	- 根据对比分段未对齐前的大小和分段未压缩前的大小，来判断是否该分段被压缩了


----

# AES256密钥的获取

- 下面仅提供寻找密钥的思路，我是直接将处理后的密钥找出来，即32个字节的密钥，猜测这32个字节的密钥应该是由其它函数处理得到的。
- 用x32dbg等调试程序，通过对文件读取函数下断点， 定位到读取npk文件的代码段附近
- 读取文件内容之后，会对读到的数据进行处理，这里由于iv是已知的，所以留意一下是否有函数的参数包含iv的地址，有的话则可以猜测该函数用于解密，可以找出对应密钥的地址。

----


# 程序结构
	
- 解封包程序为kishen_crack项目
- 文本提取使用python，脚本放在python_script
