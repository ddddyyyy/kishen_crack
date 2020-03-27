// kishen_crack.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "pack.h"
#include "unpack.h"

const int MAX_FILENAME_LEN = 100;

//选择操作
int run(int i,char* arg) {
	switch (i)
	{
	case 1:
	{
		string input;
		string output;
		if (arg == NULL) {
			cout << "请输入要解包的文件路径（只能是.npk结尾的文件）:" << endl;
			cin >> input;
		}
		else {
			input = arg;
		}
		cout << "请输入解包后的文件的存放路径:（请使用全英文路径）" << endl;
		cin >> output;
		return unpack(input, output);
	}
	break;
	case 2:
	{
		string input;
		string output;
		if (arg == NULL) {
			cout << "请输入要封包的文件夹路径:（请使用全英文路径）" << endl;
			cin >> input;
		}
		else {
			input = arg;
		}
		
		cout << "请输入解包后的文件名:（请使用全英文路径，例子：./xxx.npk）" << endl;
		cin >> output;
		if (output.length() == 0) {
			output = "./kishen.npk";
		}
		else {
			return pack(output,input);
		}
	}
		break;
	case 3:
	{
		return unpack("D:/GalGame/機神咆吼デモンベイン/backup/script.npk", "C:/Users/21156/Desktop/kishen");
	}
	break;
	case 4:
	{
		return pack("D:/GalGame/機神咆吼デモンベイン/script.npk","C:/Users/21156/Desktop/kishen");
	}
	break;
	default:
		cout << "输入错误" << endl;
		break;
	};
	return 0;
}

int main(int argc,char* argv[])
{
	if (argc == 2) {
		cout << "输入的路径为：" << argv[1] << endl;
		cout << "请输入操作（1为解包，2为封包）:" << endl;
		int i;
		cin >> i;
		cout << run(i, argv[1]) << endl;
	}
	else {
		cout << "请输入操作（1为解包，2为封包）:" << endl;
		int i;
		cin >> i;
		cout << run(i, NULL) << endl;
	}
	system("pause");
}
