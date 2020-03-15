#include "unpack.h"

int unpack(string pac_file_path, string output_path) {
	if (output_path[output_path.length() - 1] != '\\' || output_path[output_path.length() - 1] != '/') {
		output_path.append("/");
	}
	/*replace(pac_file_path.begin(), pac_file_path.end(), '\\', '/');
	replace(output_path.begin(), output_path.end(), '\\', '/');*/

	system("chcp 65001");
	locale loc = boost::locale::generator().generate("ja_JP.UTF-8");


	ifstream is;
	is.open(pac_file_path, ios::binary);
	if (!is.is_open()) {
		cout << "fail to open file" << endl;
		return OPEN_FILE_FAIL;
	}
	string root = output_path;

	//����NPK2�ļ���־
	is.seekg(8);
	//��ȡiv
	unsigned char iv[16];
	is.read((char*)iv, 16);
	//��ȡ�ļ���������������
	unsigned char temp[8];
	is.read((char*)temp, 8);
	//��ȡ�ļ�����
	uint32_t file_count = byte2uint32(temp);
	//��ȡ��������
	uint32_t table_size = byte2uint32(temp + 4);

	unsigned char* index = new unsigned char[table_size];
	unsigned char* dec_index = new unsigned char[table_size];

	
	//��ȡ����
	is.read((char*)index, table_size);
	//��������
	AesUtil::decode(key, iv, index, dec_index, table_size);
	delete[]index;

	int pos = 0;
	File file;

	//�ļ���дʹ�õĻ������
	index = new unsigned char[SEGMENT_SIZE];
	unsigned char buffer[SEGMENT_SIZE];
	unsigned char* t_index, * t_buffer;


	for (uint32_t i = 0; i < file_count; ++i) {
		pos += 1;//����һ��0x00�Ŀհ��ֽ�
		uint16_t file_name_len = byte2uint16(dec_index + pos); pos += 2;
		//��ȡ�ļ�·����
		file.name = new char[file_name_len + 1];
		uint32_t j = 0;
		for (; j < file_name_len; ++j) {
			file.name[j] = dec_index[pos + j];
		}
		file.name[j] = '\0'; pos += file_name_len;
		//�ļ�δѹ��ǰ�Ĵ�С
		file.uncomp_size = byte2uint32(dec_index + pos); pos += 4;
		pos += 32; //����32λ��sumcheck
		file.seg_count = byte2uint32(dec_index + pos); pos += 4;

		//�����ļ�
		wstring path;
		path = boost::locale::conv::to_utf<wchar_t>(root + file.name, loc).c_str();
		int dict_name_index = path.find_last_of('/');
		wstring dict_name = path.substr(0, dict_name_index);
		if (!boost::filesystem::exists(dict_name))
		{
			if (!boost::filesystem::create_directories(dict_name)) {
				return CREATE_DICT_FAIL;
			}
		}

		ofstream out;
		out.open(path, ios::out | ios::binary);
		if (!out.is_open()) {
			wcout << path << " fail to open" << endl;
			return OPEN_FILE_FAIL;
		}
		wcout << path << endl;

		file.segments = new Segment[file.seg_count];
		for (j = 0; j < file.seg_count; ++j) {
			file.segments[j].offset = byte2uint64(dec_index + pos); pos += 8;
			file.segments[j].seg_size = byte2uint32(dec_index + pos); pos += 4;
			file.segments[j].seg_ualg_size = byte2uint32(dec_index + pos); pos += 4;
			file.segments[j].seg_ucmp_size = byte2uint32(dec_index + pos); pos += 4;

			//���仺����
			if (file.segments[j].seg_size > SEGMENT_SIZE) {
				t_index = new unsigned char[file.segments[j].seg_size];
				t_buffer = new unsigned char[file.segments[j].seg_size];
			}
			else {
				t_index = index;
				t_buffer = buffer;
			}

			is.seekg(file.segments[j].offset, ios::beg);
			is.read((char*)t_index, file.segments[j].seg_size);


			//�����ļ�����
			AesUtil::decode(key, iv, t_index, t_buffer, file.segments[j].seg_size);


			//�ж��Ƿ���Ҫ��ѹ
			if (file.segments[j].seg_ucmp_size > file.segments[j].seg_ualg_size) {

				unsigned char* temp = CompressUtil::uncmp_deflate_without_header(t_buffer, file.segments[j].seg_size, file.segments[j].seg_ucmp_size);
				if (temp == NULL) {
					cout << "uncompress error" << endl;
					return COMPRESS_FAIL;
				}
				else {
					out.write((char*)temp, file.segments[j].seg_ucmp_size);
					delete[]temp;
				}
			}
			else {
				out.write((char*)t_buffer, file.segments[j].seg_ucmp_size);
			}


			//�ͷ��ڴ�
			if (t_index != index) {
				delete[]t_index;
				delete[]t_buffer;
			}

		}
		if (out.tellp() != file.uncomp_size) {
			cout << "the file size should be " << file.uncomp_size << " but the real size is " << out.tellp() << endl;
			cout << "position in index is " << pos << endl;
		}

		out.close();
		delete[]file.name;
	}


	return OK;
}