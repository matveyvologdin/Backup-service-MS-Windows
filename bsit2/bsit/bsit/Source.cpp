#define _CRT_SECURE_NO_WARNINGS
#include "Header.h"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <zip.h>
#include <Windows.h>
#include <atlstr.h>
#include <tchar.h>

using namespace std;

int arch_main(int mode)
{
		ofstream log("C:/Users/volog/Downloads/log.txt");
		ifstream config("C:/Users/volog/Downloads/config.txt");
		string source_path;
		string destination_file;
		config >> source_path;
		config >> destination_file;
		vector<string> masks;
		string tmp;
		while (!config.eof())
		{
			config >> tmp;
			masks.push_back(tmp);
		}
		config.close();
		multimap<string, string> files;//<путь до файла><имя файла>
		multimap<string, string> arch_files;
		recursive_file_search((source_path + "\\*").c_str(), &files);

		multimap<string, string>::iterator it;
		vector<string>::iterator vect_it;
		for (it = files.begin(); it != files.end(); ++it)
		{
			for (vect_it = masks.begin(); vect_it != masks.end(); ++vect_it)
			{
				if (check((char*)it->second.c_str(), (char*)vect_it->c_str()) == true)
				{
					arch_files.insert({ it->first, it->second });
					break;
				}
			}
		}

		int error = 0;
		zip* archive;
		if (mode == 0)
		{
			archive = zip_open(destination_file.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
		}
		else
		{
			archive = zip_open(destination_file.c_str(), 0, &error);
		}
		if (error)
		{
			log << "could not open or create archive" << endl;
			return false;
		}

		for (it = arch_files.begin(); it != arch_files.end(); ++it)
		{
			zip_source* source = zip_source_file(archive, (it->first + it->second).c_str(), 0, 0);
			if (source == NULL)
			{
				log << "failed to create source buffer. " << zip_strerror(archive) << endl;
				return false;
			}

			int index = zip_file_add(archive, (it->first + it->second).c_str() + source_path.size() + 1, source, ZIP_FL_OVERWRITE);
			if (index < 0)
			{
				log << "failed to add file to archive. " << zip_strerror(archive) << endl;
				return false;
			}
		}
		int count = zip_get_num_entries(archive, 0);
		zip_stat_t stat;
		int flag = 0;
		for (int i = 0; i < count; i++)
		{
			zip_stat_index(archive, i, 0, &stat);
			if (strstr(stat.name, "~$") != NULL)
			{
				zip_delete(archive, i);
				continue;
			}
			vector<string>::iterator vect_it;
			for (vect_it = masks.begin(); vect_it != masks.end(); ++vect_it)
				if (check((char*)(string(stat.name).c_str()), (char*)vect_it->c_str()) == true)
					flag = 1;
			if (!flag)
				zip_delete(archive, i);
			flag = 0;
		}
		zip_close(archive);
		log.close();
		Sleep(1000);
	return true;
}

void recursive_file_search(const char* dir, multimap<string, string>* files)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char buff[MAX_PATH] = { 0 };
	char* pch;
	char tmp[200];	
	char path[200];
	strcpy(path, dir);
	path[strlen(path) - 1] = '\0';

	hFind = FindFirstFile(CA2W(dir), &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Invalid file handle. Error is %u.\n", GetLastError());
	}
	else
	{
		if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY
			&& wcscmp(FindFileData.cFileName, L".") != 0 && wcscmp(FindFileData.cFileName, L"..") != 0)
		{
			strcpy(buff, dir);
			size_t i = strlen(buff) - 1;
			buff[i] = '\0';
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%ws", FindFileData.cFileName);
			strcat(buff, tmp);
			strcat(buff, "\\");
			strcat(buff, "*");
			recursive_file_search(buff, files);
		}
		///////////////////////////////////////////////////////
		//////////////////////////////////////////////////////
		//вот здесь поиск файлов по маске
		if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%ws", FindFileData.cFileName);

			//cout << path << tmp << endl;
			(*files).insert({ path, tmp });

		}
		// List all the other files in the directory.
		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%ws", FindFileData.cFileName);
			if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY
				&& strcmp(tmp, ".") != 0 && strcmp(tmp, "..") != 0)
			{

				strcpy(buff, dir);
				size_t i = strlen(buff) - 1;
				buff[i] = '\0';
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "%ws", FindFileData.cFileName);
				strcat(buff, tmp);
				strcat(buff, "\\");
				strcat(buff, "*");
				recursive_file_search(buff, files);
			}
			///////////////////////////////////////////////////////
				//////////////////////////////////////////////////////
				//вот здесь поиск файлов по маске
			if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "%ws", FindFileData.cFileName);

				//cout << path << tmp << endl;
				(*files).insert({ path, tmp });

			}
		}
		FindClose(hFind);
	}

}

bool check(char* s, char* p)
{
	size_t tmp = string(s).rfind('\\');
	s += tmp + 1;
	char* rs = 0, * rp = 0;
	while (1)
		if (*p == '*')
			rs = s, rp = ++p;
		else if (!*s)
			return !*p;
		else if (*s == *p || *p == '?')
			++s, ++p;
		else if (rs)
			s = ++rs, p = rp;
		else
			return false;
}