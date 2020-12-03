// FileParting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <tuple>
#include <algorithm>

using namespace std;

static auto partFile(wstring strPath, int partSize)->vector<wstring>;
static bool mergeFiles(wstring dstFileName, vector<wstring> srcFileNameList);
static bool readFileBy(vector<pair<char*, int>>& out_buffItemList, wstring strPath, int partSize);
static auto saveBuffersToFile(vector<pair<char*, int>> buffItemList, wstring dataDir,
	wstring fileNamePrefix, wstring fileExt)->vector<wstring>;
static long getFileSize(wstring path);
static auto parsePath(wstring strPath)->tuple<wstring, wstring, wstring>;

int main()
{
	//const wstring kFilePath = L"D:/projects/etc/fileParting/sample/angio2.dat";
	const wstring kFilePath = L"D:\\projects\\etc\\fileParting\\sample\\angio2.dat";

	const auto kPartSize = (int)pow(2, 26);

	auto fileNameList = partFile(kFilePath, kPartSize);

	mergeFiles(
		L"D:/projects/etc/fileParting/sample/angio2_res.dat",
		fileNameList
	);

    return 0;
}

auto partFile(wstring strPath, int partSize)->vector<wstring>
{
	vector<pair<char*, int>> buffItemList;

	// read file
	if (!readFileBy(buffItemList, strPath, partSize)) {
		return vector<wstring>();
	}

	// write files
	auto pathItem = parsePath(strPath);
	auto dataDir = get<0>(pathItem);
	auto fileName = get<1>(pathItem);
	auto fileExt = get<2>(pathItem);
	auto filePathList = saveBuffersToFile(buffItemList, dataDir, fileName, fileExt);
	if (filePathList.size() == 0) {
		return vector<wstring>();
	}

	// clear buffers
	for (auto buffItem : buffItemList) {
		delete[] get<0>(buffItem);
	}

	return filePathList;
}

bool mergeFiles(wstring dstFileName, vector<wstring> srcFileNameList)
{
	bool ret = false;

	// file to write
	ofstream ostream(dstFileName, std::ofstream::out | std::ofstream::app | std::ostream::binary);
	if (!ostream.is_open()) {
		goto FINISH;
	}

	//
	for (auto path : srcFileNameList) {
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

auto parsePath(wstring strPath)->tuple<wstring, wstring, wstring>
{
	auto posEnd = strPath.length();
	auto posExt = strPath.rfind('.');
	auto posDir0 = strPath.rfind('/');
	auto posDir1 = strPath.rfind('\\');
	size_t posDir = 0;
	if (posDir0 == string::npos && posDir1 == string::npos) {
		return tuple<wstring, wstring, wstring>();
	}
	else if (posDir0 == string::npos) {
		posDir = posDir1;
	}
	else if (posDir1 == string::npos) {
		posDir = posDir0;
	}
	else {
		posDir = max(posDir0, posDir1);
	}

	auto strExt = strPath.substr(posExt + 1, posEnd - (posExt + 1));
	auto strDir = strPath.substr(0, posDir);
	auto strName = strPath.substr(posDir + 1, posExt - posDir - 1);

	return make_tuple(strDir, strName, strExt);
}

long getFileSize(wstring path)
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

bool readFileBy(vector<pair<char*, int>>& out_buffItemList, wstring strPath, int partSize)
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

auto saveBuffersToFile(vector<pair<char*, int>> buffItemList, wstring dataDir, wstring fileNamePrefix, wstring fileExt)->vector<wstring>
{
	vector<wstring> fileNameList;

	const int kListSize = (int)buffItemList.size();
	for (int idx = 0; idx < kListSize; idx++) {
		// determine file name
		wstringbuf strBuf;
		wostream ostream(&strBuf, std::ostream::binary);

		ostream << fileNamePrefix;
		ostream << L"_";
		ostream << idx;
		ostream << L".";
		ostream << fileExt;

		auto fileName = strBuf.str();
		auto filePath = dataDir + L"/" + fileName;

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

		fileNameList.push_back(filePath);
	}

	return fileNameList;
}

