#pragma once
#include <iostream>
#include <map>
using namespace std;

void recursive_file_search(const char* dir, multimap<string, string>* files);
bool check(char* s, char* p);
int arch_main(int mode);