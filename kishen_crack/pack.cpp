#include "pack.h"


/// �������
/* �������
	1. ����aes256���ܵ�iv����
	2. ����Ҫ������ļ��У�ע�⣬������Ϸ���ݵ���Դ�ļ�ʹ�õ�·������д���ģ����Է��ʱ����Ҫ��֤д���·���ͷ��ǰ��ͬ������media/xxx��
	3. �����ļ��������ļ�������Ϣ����Ҫ������Ƭ��ѹ�������ܲ��������ļ���Ƭ����д����ʱ�ļ�
	4. ��¼�ļ�����
	5. �������ɵ�������Ϣ�����ܲ�д���ļ�
	6. ���ļ���Ƭ����׷��д��
*/
int pack(string npk_name, string root_path) {
	//replace(npk_name.begin(), npk_name.end(), '\\', '/');
	//replace(root_path.begin(), root_path.end(), '\\', '/');
	if (root_path[root_path.length() - 1] != '\\' || root_path[root_path.length() - 1] != '/') {
		//����Ϊ�ļ���
		root_path.append("/");
	}

	//���ñ���
	system("chcp 65001");
	locale loc = boost::locale::generator().generate("ja_JP.UTF-8");

	boost::filesystem::path myPath(root_path);
	boost::filesystem::recursive_directory_iterator endIter;

	ifstream file_opened;//�򿪵��ļ�





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
	};//NPK2ͷ
	char iv[16] = {
		0x4E ,0x50 ,0x4B ,0x32 ,0x02 ,0x00 ,0x00 ,0x00,
		0x4E ,0x50 ,0x4B ,0x32 ,0x02 ,0x00 ,0x00 ,0x00
	};//iv

	uint32_t file_count = 0;//��ȡ�����ļ�����

	vector<File*> files;//���е��ļ�����

	unsigned char *buffer = new unsigned char[SEGMENT_SIZE]; //����
	unsigned char *index = new unsigned char[SEGMENT_SIZE];

	for (boost::filesystem::recursive_directory_iterator item(myPath); item != endIter; item++) {
		if (!boost::filesystem::is_directory(*item)) {
			file_count++;

			File* file = new File();//���ڴ���ļ��ṹ����Ϣ

			//����·������Ҫ��ȡ�� media/xxxx ��·��
			string filename = item->path().string().substr(root_path.length(), item->path().string().length() - root_path.length());


			replace(filename.begin(), filename.end(), '\\', '/');
			string path = boost::locale::conv::to_utf<char>(filename, "GBK");
			file->name = new char[path.length()];
			for (int j = 0; j < path.length(); ++j) {
				file->name[j] = path.c_str()[j];//�ļ���
			}
			file->name_len = path.length();//�ļ�������

			//����ļ���sha256
			SHA256Util::FileSHA256(item->path().string().c_str(), file->check);


			//��ȡ�ļ�
			file_opened.open(item->path().string(), ios::in | ios::binary);
			if (!file_opened.is_open()) {
				cout << "fail to open file" << endl;
				return OPEN_FILE_FAIL;
			}
			else {
				cout << "[" << file_count << "]:" << path << endl;
			}
			//����ļ���С
			file_opened.seekg(0, ios::end);
			file->uncomp_size = file_opened.tellg();//Ϊѹ��ǰ��С
			file_opened.seekg(0, ios::beg);//�ָ�ָ��λ��
			//�����ļ��ֶ���
			int count = file->uncomp_size / SEGMENT_SIZE;
			bool single = (count == 0);//������Ƭ
			if (file->uncomp_size % SEGMENT_SIZE != 0) {
				count += 1;
			}
			file->seg_count = count;
			file->segments = new Segment[count];

			unsigned char *t_index,*t_buffer;


			//��Ƭ�ļ�
			if (!single) {
				//��������ʹ�õ��ǷֶΣ�����index�Ĵ�С�̶�
				for (int i = 0; i < count; ++i) {
					file->segments[i].offset = temp_file.tellp();//�ļ����ݵ�ƫ����������ʹ����ʱ�ļ����棬֮���ټ���ȱʧ��ƫ����
					if (i == count - 1) {
						file->segments[i].seg_ucmp_size = file->uncomp_size % MAX_SEGMENT_FILE_SIZE;
					}
					else {
						file->segments[i].seg_ucmp_size = MAX_SEGMENT_FILE_SIZE;
					}

					//���仺����
					if (file->segments[i].seg_ucmp_size > SEGMENT_SIZE) {
						t_index = new unsigned char[file->segments[i].seg_ucmp_size];
						t_buffer = new unsigned char[file->segments[i].seg_ucmp_size];
					}
					else {
						t_index = index;
						t_buffer = buffer;
					}

					//��ȡ�ļ�����
					file_opened.read((char*)t_index, file->segments[i].seg_ucmp_size); //����ʹ��index����ΪAES��������
					file->segments[i].seg_ualg_size = file->segments[i].seg_ucmp_size;//δ����Ĵ�С��������ʱʹ���ļ�ѹ��ǰ�Ĵ�С
					//�ֶε��ļ���ѹ��
					//����
					AesUtil::encode(key, (unsigned char*)iv, t_index, t_buffer, file->segments[i].seg_ualg_size, &file->segments[i].seg_size);
					temp_file.write((char*)t_buffer, file->segments[i].seg_size);


					//�ͷ��ڴ�
					if (t_index != index) {
						delete[]t_index;
						delete[]t_buffer;
					}

				}
			}
			else {


				//���ֶε��ļ�Ҫѹ��
				file->segments[0].seg_ucmp_size = file->uncomp_size;
				file->segments[0].offset = temp_file.tellp();//�ļ����ݵ�ƫ����������ʹ����ʱ�ļ����棬֮���ټ���ȱʧ��ƫ����


				//���仺����
				if (file->segments[0].seg_ucmp_size > SEGMENT_SIZE) {
					t_buffer = new unsigned char[file->segments[0].seg_ucmp_size];
				}
				else {
					t_buffer = buffer;
				}


				//��ȡ�ļ�����
				file_opened.read((char*)t_buffer, file->segments[0].seg_ucmp_size);
				//ѹ��,δ����Ĵ�С��Ϊѹ����Ĵ�С
				unsigned char* comp_data = CompressUtil::cmp_deflate_without_header(t_buffer, file->segments[0].seg_ucmp_size, &file->segments[0].seg_ualg_size);
				//����
				AesUtil::encode(key, (unsigned char*)iv, comp_data, t_buffer, file->segments[0].seg_ualg_size, &file->segments[0].seg_size);
				temp_file.write((char*)t_buffer, file->segments[0].seg_size);

				delete[]comp_data;
				//�ͷ��ڴ�
				if (t_buffer != buffer) {
					delete[]t_buffer;
				}
			}


			file_opened.close();

			files.push_back(file);
		}
	}

	delete[]index;
	//�ļ�������������

	///��ʼ���

	//�������header��ivд��
	packed_file.write(header, 8);
	packed_file.write(iv, 16);
	//д���ļ�����
	packed_file.write(uint32t2byte(file_count), 4);

	uint32_t index_size = 0;//���ܺ���ļ�������С


	for (auto it = files.begin(); it != files.end(); it++) {
		File file = **it;
		index_size += (3 + file.name_len + 40 + file.seg_count * 20);
	}
	//AES���ܻᲹ���ֽ�
	index_size += (16 - index_size % 16);

	//д��������С
	packed_file.write(uint32t2byte(index_size), 4);

	//�����ļ�����������
	uint32_t pos = 0;

	//Ϊ��������ռ�
	index = new unsigned char[index_size];

	for (auto it = files.begin(); it != files.end(); it++) {
		File file = **it;
		index[pos] = 0x00; pos += 1;//д��ͷ���հ�
		char* temp = uint16t2byte(file.name_len);
		for (int i = 0; i < 2; i++) {
			index[pos + i] = temp[i];
		}
		pos += 2;//д���ļ�������
		delete[]temp;


		for (int i = 0; i < file.name_len; ++i) {
			index[pos + i] = file.name[i];
		}
		pos += file.name_len;//д���ļ���

		temp = uint32t2byte(file.uncomp_size);
		for (int i = 0; i < 4; i++) {
			index[pos + i] = temp[i];
		}
		pos += 4;//д���ļ���С
		delete[]temp;

		for (int i = 0; i < 32; ++i) {
			index[pos + i] = file.check[i];
		}
		pos += 32;//д��check sum md5

		temp = uint32t2byte(file.seg_count);
		for (int i = 0; i < 4; i++) {
			index[pos + i] = temp[i];
		}
		pos += 4;//д��ֶ���
		delete[]temp;


		for (int i = 0; i < file.seg_count; ++i) {
			temp = uint64t2byte(file.segments[i].offset + index_size + 32);//�����32Ϊ�ļ���ͷ��32���ֽ�
			for (int i = 0; i < 8; i++) {
				index[pos + i] = temp[i];
			}
			pos += 8;//д��ֶ��ļ���ƫ�Ƶ�ַ
			delete[]temp;

			temp = uint32t2byte(file.segments[i].seg_size);
			for (int i = 0; i < 4; i++) {
				index[pos + i] = temp[i];
			}
			pos += 4;//д��ֶ��ļ���С
			delete[]temp;

			temp = uint32t2byte(file.segments[i].seg_ualg_size);
			for (int i = 0; i < 4; i++) {
				index[pos + i] = temp[i];
			}
			pos += 4;//д��ֶ�δ����ǰ�Ĵ�С
			delete[]temp;

			temp = uint32t2byte(file.segments[i].seg_ucmp_size);
			for (int i = 0; i < 4; i++) {
				index[pos + i] = temp[i];
			}
			pos += 4;//д��ֶ�δѹ��ǰ�Ĵ�С
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