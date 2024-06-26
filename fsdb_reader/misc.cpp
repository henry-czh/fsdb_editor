
#include "misc.h"

SIGNAL_MAP
read_config()
{
    char line[MAX_LINE];
    char scope[MAX_LINE];
    char protocol[MAX_KV];
    char sigType[MAX_KV];
    char sigName[MAX_KV];
    SIGNAL_MAP cfgs;
    sigInfo si;

    FILE *fp = NULL;
    fp = fopen("config", "r");

    while(fgets(line, MAX_LINE, fp)) {
        if (line[0] != ' ') {
            *strchr(line, ':') = ' ';
            sscanf(line, "%s %s", scope, protocol);
            map<string, sigInfo> cfg;
	        strcpy(si.name, protocol);
	        si.idcode = 0;
            cfg["info"] = si;
            cfgs[scope] = cfg;
        } else {
            *strchr(line, ':') = ' ';
            sscanf(line, "%s %s", sigType, sigName);
            //printf("k[%s] v[%s]\n", k, v);
	        si = {.idcode = 0};
            // 以type为name，实际信号名为key
	        strcpy(si.name, sigType);
            cfgs[scope][sigName] = si;

        }
    }

    fclose(fp);

    return cfgs;
}

bool
path_compare(string a_str, string b_str)
{
	return b_str == a_str;
}

void
path_push(string& base_str, string new_str)
{
	base_str = base_str + "." + new_str;
}

string
path_pop(string base_str)
{
	int pos;
	pos = base_str.find_last_of(".",base_str.size());

	return base_str.erase(pos,base_str.size());
}

void
AddPath(char* str, char* s)
{
    int l = strlen(str);
    if(l)
    {
        str[l] = '.';
        strcpy(str + l + 1, s);
    }
    else
    {
        strcpy(str, s);
    }
}

void
DelPath(char* str)
{
    char* p = strrchr(str, '.');
    if(p)
        *p = 0;
    else
        str[0] = 0;
}

/*
int
main(int argc, char *argv[])
{
	SIGNAL_MAP signal_map;
	signal_map = read_config();

	string hdl_path;
	hdl_path = "chip_tb";

	string new_str = "skt0";

	hdl_path = path_push(hdl_path,new_str);

	new_str = "top.apu0";
	hdl_path = path_push(hdl_path,new_str);

	cout<<hdl_path << "\n";
	cout<<"hello world!\n";

	hdl_path = path_pop(hdl_path);
	cout<<hdl_path << "\n";

	string cmp_str = "chip_tb.skt0.top";
	bool match = path_compare(cmp_str,hdl_path);
	cout<<match<< "\n";
}
*/

