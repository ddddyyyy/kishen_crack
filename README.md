# kishen_crack
機神咆吼デモンベイン 汉化工具

-----

- 游戏引擎为nitroplus
- 资源包的格式为npk2
- 资源包内的数据采用aes256加密和deflate无头压缩

-----

### .npk文件格式

1. NPK2字符串开头（8个字节）（0x00补齐）
2. iv向量（16个字节）
3. 文件个数（4个字节）
4. 索引区域大小（4个字节）
5. 索引区域（由上面的区域大小决定读取的字节数）（使用aes256加密）
6. 文件数据

-----

### 索引数据结构

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
