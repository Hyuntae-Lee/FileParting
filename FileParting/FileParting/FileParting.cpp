// FileParting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <tuple>

using namespace std;

bool partFile(string strPath, int partSize);
bool mergeFile(string dstFileName, vector<string> srcPathList);
bool readFileBy(vector<pair<char*, int>>& out_buffItemList, string strPath, int partSize);
bool saveBuffersToFile(vector<pair<char*, int>> buffItemList, string dataDir, string fileNamePrefix, string fileExt);
long getFileSize(string path);
auto parsePath(string strPath)->tuple<string, string, string>;

int main()
{
	const string kFilePath = "D:/projects/etc/fileParting/sample/angio2.dat";

	const auto kPartSize = (int)pow(2, 26);

	partFile(kFilePath, kPartSize);

	vector<string> srcFileList = {
		"D:/projects/etc/fileParting/sample/angio2_0.dat",
		"D:/projects/etc/fileParting/sample/angio2_1.dat",
		"D:/projects/etc/fileParting/sample/angio2_2.dat",
		"D:/projects/etc/fileParting/sample/angio2_3.dat",
	};
	mergeFile(
		"D:/projects/etc/fileParting/sample/angio2_res.dat",
		srcFileList
	);

    return 0;
}

auto parsePath(string strPath)->tuple<string, string, string>
{
	auto posEnd = strPath.length();
	auto posExt = strPath.rfind('.');
	auto posDir = strPath.rfind('/');

	auto strExt = strPath.substr(posExt + 1, posEnd - (posExt + 1));
	auto strDir = strPath.substr(0, posDir);
	auto strName = strPath.substr(posDir + 1, posExt - posDir - 1);

	return make_tuple(strDir, strName, strExt);
}

long getFileSize(string path)
{
	ifstream istream(path, ifstream::binary);
	if (!istream.is_open()) {
		return false;
	}

	istream.seekg(0, std::ios::end);

	auto fileSize = (long)istream.tellg();
	istream.close();

	return fileSize;
}

bool readFileBy(vector<pair<char*, int>>& out_buffItemList, string strPath, int partSize)
{
	ifstream istream(strPath, ifstream::binary);
	if (!istream.is_open()) {
		return false;
	}

	// read file
	auto totalSize = getFileSize(strPath);
	while (!istream.eof()) {
		char* buff = new char[partSize + 1];
		memset(buff, 0, partSize + 1);

		istream.read(buff, partSize);

		// file size
		int nContentsSize = 0;
		{
			if (istream.tellg() > 0) {
				nContentsSize = partSize;
			}
			else {
				nContentsSize = totalSize % partSize;
			}
		}

		out_buffItemList.push_back(make_pair(buff, nContentsSize));
	};
	istream.close();

	return true;
}

bool saveBuffersToFile(vector<pair<char*, int>> buffItemList, string dataDir, string fileNamePrefix, string fileExt)
{
	const int kListSize = (int)buffItemList.size();
	for (int idx = 0; idx < kListSize; idx++) {
		// determine file name
		stringbuf strBuf;
		ostream ostream(&strBuf, std::ostream::binary);

		ostream << fileNamePrefix;
		ostream << "_";
		ostream << idx;
		ostream << ".";
		ostream << fileExt;

		auto fileName = strBuf.str();
		auto filePath = dataDir + "/" + fileName;

		//  save file
		ofstream fstream(filePath, std::ostream::binary);
		if (!fstream.is_open()) {
			continue;
		}

		auto buff = get<0>(buffItemList[idx]);
		auto buffSize = get<1>(buffItemList[idx]);

		fstream.write(buff, buffSize);

		//
		fstream.flush();
		fstream.close();
	}

	return true;
}

bool mergeFile(string dstFileName, vector<string> srcPathList)
{
	bool ret = false;

	// file to write
	ofstream ostream(dstFileName, std::ofstream::out | std::ofstream::app | std::ostream::binary);
	if (!ostream.is_open()) {
		goto FINISH;
	}

	//
	for (auto path : srcPathList) {
		// read
		ifstream istream(path, std::ostream::binary);
		if (!istream.is_open()) {
			goto FINISH;
		}

		auto fileSize = getFileSize(path);
		char* buff = new char[fileSize + 1];
		memset(buff, 0, fileSize + 1);

		istream.read(buff, fileSize);

		// merge
		ostream.write(buff, fileSize);
	}

	ret = true;

FINISH:
	if (ostream.is_open()) {
		ostream.close();
	}

	return ret;
}

bool partFile(string strPath, int partSize)
{
	vector<pair<char*, int>> buffItemList;

	// read file
	if (!readFileBy(buffItemList, strPath, partSize)) {
		return false;
	}

	// write files
	auto pathItem = parsePath(strPath);
	auto dataDir = get<0>(pathItem);
	auto fileName = get<1>(pathItem);
	auto fileExt = get<2>(pathItem);
	if (!saveBuffersToFile(buffItemList, dataDir, fileName, fileExt)) {
		return false;
	}

	// clear buffers
	for (auto buffItem : buffItemList) {
		delete[] get<0>(buffItem);
	}
}

