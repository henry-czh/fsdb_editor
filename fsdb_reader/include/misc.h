
#ifndef __MSIC_H
#define __MISC_H

#include<map>
#include<stdio.h>
#include<iostream>
#include<string.h>
#include<string>
#include<fstream>
using namespace std;

#define MAX_KV 64
#define MAX_LINE 1024

//#define CHI_SIG_NUM 2

typedef struct sigInfo_t
{
	char name[MAX_KV];
	int idcode;
	int lbitnum;
} sigInfo;

typedef map<string,map<string,sigInfo>> SIGNAL_MAP;

SIGNAL_MAP read_config();

bool path_compare(string a_str, string b_str);

void path_push(string& base_str, string new_str);

string path_pop(string base_str);

void AddPath(char* str, char* s);

void DelPath(char* str);

#endif
