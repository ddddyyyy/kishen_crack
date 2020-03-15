#ifndef PACK_H
#define PACK_H

#include <iostream>
#include <fstream>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include "ByteUtil.h"
#include "AesUtil.h"
#include "CompressUtil.h"
#include "Constant.h"
#include "SHA256Util.h"

using namespace std;

int pack(string npk_name, string root_path);

#endif // !PACK_H
