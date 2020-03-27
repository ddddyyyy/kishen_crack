#include "pack.h"


/// 封包函数
/* 封包流程
	1. 设置aes256加密的iv向量
	2. 遍历要封包的文件夹（注意，由于游戏内容的资源文件使用的路径都是写死的，所以封包时候需要保证写入的路径和封包前相同，形如media/xxx）
	3. 解析文件，生成文件索引信息，需要进行切片或压缩，加密操作，将文件切片数据写入临时文件
	4. 记录文件总数
	5. 遍历生成的索引信息，加密并写入文件
	6. 将文件切片数据追加写入
*/
int pack(string npk_name, string root_path) {
	//replace(npk_name.begin(), npk_name.end(), '\\', '/');
	//replace(root_path.begin(), root_path.end(), '\\', '/');
	if (root_path[root_path.length() - 1] != '\\' || root_path[root_path.length() - 1] != '/') {
		//必须为文件夹
		root_path.append("/");
	}

	//设置编码
	system("chcp 65001");
	locale loc = boost::locale::generator().generate("ja_JP.UTF-8");

	boost::filesystem::path myPath(root_path);
	boost::filesystem::recursive_directory_iterator endIter;

	ifstream file_opened;//打开的文件





	ofstream packed_file;
	ofstream temp_file;
	packed_file.open(npk_name, ios::out | ios::binary);
	temp_file.open(".tmp", ios::out | ios::binary);
	if (!temp_file.is_open()) {
		cout << "fail to open file .tmp" << endl;
		return OPEN_FILE_FAIL;
	}
	if (!packed_file.is_open()) {
		cout << "fail to open file .npk" << endl;
		return OPEN_FILE_FAIL;
	}

	char header[8] = {
		0x4E ,0x50 ,0x4B ,0x32 ,0x02 ,0x00 ,0x00 ,0x00
	};//NPK2头
	char iv[16] = {
		0x4E ,0x50 ,0x4B ,0x32 ,0x02 ,0x00 ,0x00 ,0x00,
		0x4E ,0x50 ,0x4B ,0x32 ,0x02 ,0x00 ,0x00 ,0x00
	};//iv

	uint32_t file_count = 0;//读取到的文件个数

	vector<File*> files;//所有的文件索引

	unsigned char *buffer = new unsigned char[SEGMENT_SIZE]; //缓存
	unsigned char *index = new unsigned char[SEGMENT_SIZE];

	for (boost::filesystem::recursive_directory_iterator item(myPath); item != endIter; item++) {
		if (!boost::filesystem::is_directory(*item)) {
			file_count++;

			File* file = new File();//用于存放文件结构体信息

			//解析路径，需要提取出 media/xxxx 的路径
			string filename = item->path().string().substr(root_path.length(), item->path().string().length() - root_path.length());


			replace(filename.begin(), filename.end(), '\\', '/');
			string path = boost::locale::conv::to_utf<char>(filename, "GBK");
			file->name = new char[path.length()];
			for (int j = 0; j < path.length(); ++j) {
				file->name[j] = path.c_str()[j];//文件名
			}
			file->name_len = path.length();//文件名长度

			//获得文件的sha256
			SHA256Util::FileSHA256(item->path().string().c_str(), file->check);


			//读取文件
			file_opened.open(item->path().string(), ios::in | ios::binary);
			if (!file_opened.is_open()) {
				cout << "fail to open file" << endl;
				return OPEN_FILE_FAIL;
			}
			else {
				cout << "[" << file_count << "]:" << path << endl;
			}
			//获得文件大小
			file_opened.seekg(0, ios::end);
			file->uncomp_size = file_opened.tellg();//为压缩前大小
			file_opened.seekg(0, ios::beg);//恢复指针位置
			//计算文件分段数
			int count = file->uncomp_size / SEGMENT_SIZE;
			bool single = (count == 0);//单个分片
			if (file->uncomp_size % SEGMENT_SIZE != 0) {
				count += 1;
			}
			file->seg_count = count;
			file->segments = new Segment[count];

			unsigned char *t_index,*t_buffer;


			//切片文件
			if (!single) {
				//这里由于使用的是分段，所以index的大小固定
				for (int i = 0; i < count; ++i) {
					file->segments[i].offset = temp_file.tellp();//文件内容的偏移量，这里使用临时文件代替，之后再加上缺失的偏移量
					if (i == count - 1) {
						file->segments[i].seg_ucmp_size = file->uncomp_size % MAX_SEGMENT_FILE_SIZE;
					}
					else {
						file->segments[i].seg_ucmp_size = MAX_SEGMENT_FILE_SIZE;
					}

					//分配缓存区
					if (file->segments[i].seg_ucmp_size > SEGMENT_SIZE) {
						t_index = new unsigned char[file->segments[i].seg_ucmp_size];
						t_buffer = new unsigned char[file->segments[i].seg_ucmp_size];
					}
					else {
						t_index = index;
						t_buffer = buffer;
					}

					//读取文件内容
					file_opened.read((char*)t_index, file->segments[i].seg_ucmp_size); //这里使用index，因为AES。。。。
					file->segments[i].seg_ualg_size = file->segments[i].seg_ucmp_size;//未对齐的大小，这里暂时使用文件压缩前的大小
					//分段的文件不压缩
					//加密
					AesUtil::encode(key, (unsigned char*)iv, t_index, t_buffer, file->segments[i].seg_ualg_size, &file->segments[i].seg_size);
					temp_file.write((char*)t_buffer, file->segments[i].seg_size);


					//释放内存
					if (t_index != index) {
						delete[]t_index;
						delete[]t_buffer;
					}

				}
			}
			else {


				//不分段的文件要压缩
				file->segments[0].seg_ucmp_size = file->uncomp_size;
				file->segments[0].offset = temp_file.tellp();//文件内容的偏移量，这里使用临时文件代替，之后再加上缺失的偏移量


				//分配缓存区
				if (file->segments[0].seg_ucmp_size > SEGMENT_SIZE) {
					t_buffer = new unsigned char[file->segments[0].seg_ucmp_size];
				}
				else {
					t_buffer = buffer;
				}


				//读取文件内容
				file_opened.read((char*)t_buffer, file->segments[0].seg_ucmp_size);
				//压缩,未对齐的大小即为压缩后的大小
				unsigned char* comp_data = CompressUtil::cmp_deflate_without_header(t_buffer, file->segments[0].seg_ucmp_size, &file->segments[0].seg_ualg_size);
				//加密
				AesUtil::encode(key, (unsigned char*)iv, comp_data, t_buffer, file->segments[0].seg_ualg_size, &file->segments[0].seg_size);
				temp_file.write((char*)t_buffer, file->segments[0].seg_size);

				delete[]comp_data;
				//释放内存
				if (t_buffer != buffer) {
					delete[]t_buffer;
				}
			}


			file_opened.close();

			files.push_back(file);
		}
	}

	delete[]index;
	//文件索引解析结束

	///开始封包

	//将封包的header和iv写入
	packed_file.write(header, 8);
	packed_file.write(iv, 16);
	//写入文件个数
	packed_file.write(uint32t2byte(file_count), 4);

	uint32_t index_size = 0;//加密后的文件索引大小


	for (auto it = files.begin(); it != files.end(); it++) {
		File file = **it;
		index_size += (3 + file.name_len + 40 + file.seg_count * 20);
	}
	//AES加密会补齐字节
	index_size += (16 - index_size % 16);

	//写入索引大小
	packed_file.write(uint32t2byte(index_size), 4);

	//遍历文件索引，解析
	uint32_t pos = 0;

	//为索引分配空间
	index = new unsigned char[index_size];

	for (auto it = files.begin(); it != files.end(); it++) {
		File file = **it;
		index[pos] = 0x00; pos += 1;//写入头个空白
		char* temp = uint16t2byte(file.name_len);
		for (int i = 0; i < 2; i++) {
			index[pos + i] = temp[i];
		}
		pos += 2;//写入文件名长度
		delete[]temp;


		for (int i = 0; i < file.name_len; ++i) {
			index[pos + i] = file.name[i];
		}
		pos += file.name_len;//写入文件名

		temp = uint32t2byte(file.uncomp_size);
		for (int i = 0; i < 4; i++) {
			index[pos + i] = temp[i];
		}
		pos += 4;//写入文件大小
		delete[]temp;

		for (int i = 0; i < 32; ++i) {
			index[pos + i] = file.check[i];
		}
		pos += 32;//写入check sum md5

		temp = uint32t2byte(file.seg_count);
		for (int i = 0; i < 4; i++) {
			index[pos + i] = temp[i];
		}
		pos += 4;//写入分段数
		delete[]temp;


		for (int i = 0; i < file.seg_count; ++i) {
			temp = uint64t2byte(file.segments[i].offset + index_size + 32);//这里的32为文件开头的32个字节
			for (int i = 0; i < 8; i++) {
				index[pos + i] = temp[i];
			}
			pos += 8;//写入分段文件的偏移地址
			delete[]temp;

			temp = uint32t2byte(file.segments[i].seg_size);
			for (int i = 0; i < 4; i++) {
				index[pos + i] = temp[i];
			}
			pos += 4;//写入分段文件大小
			delete[]temp;

			temp = uint32t2byte(file.segments[i].seg_ualg_size);
			for (int i = 0; i < 4; i++) {
				index[pos + i] = temp[i];
			}
			pos += 4;//写入分段未对齐前的大小
			delete[]temp;

			temp = uint32t2byte(file.segments[i].seg_ucmp_size);
			for (int i = 0; i < 4; i++) {
				index[pos + i] = temp[i];
			}
			pos += 4;//写入分段未压缩前的大小
			delete[]temp;

		}
	}

	unsigned char* output = new unsigned char[index_size];
	uint32_t temp_index_size = 0;

	AesUtil::encode(key, (unsigned char*)iv, index, output, pos, &temp_index_size);
	if (temp_index_size != index_size) {
		cout << "this expected index's size:  " << index_size << " is no correct . It should not be " << temp_index_size << endl;
		return UNPACK_ERROR_SIZE;
	}
	packed_file.write((char*)output, index_size);
	delete[]output;

	temp_file.close();

	ifstream temp_file_read;
	temp_file_read.open(".tmp", ios::in | ios::binary);
	temp_file_read.seekg(0, ios::end);
	uint64_t size = temp_file_read.tellg();

	temp_file_read.seekg(0, ios::beg);
	temp_file_read.read((char*)buffer, SEGMENT_SIZE);

	while (!temp_file_read.eof()) {
		packed_file.write((char*)buffer, SEGMENT_SIZE);
		temp_file_read.read((char*)buffer, SEGMENT_SIZE);
	}

	if (size % 0x10010 != 0) {
		temp_file_read.read((char*)buffer, size % SEGMENT_SIZE);
		packed_file.write((char*)buffer, size % SEGMENT_SIZE);
	}

	temp_file_read.close();
	remove("./.tmp");
	packed_file.close();

	return OK;
}