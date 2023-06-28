#include <stdio.h>
#include <string.h>
#include <string>
#include <map>

using namespace std;

int read_config()
{
    int max_line = 1024;
    char line[max_line];
    char key[max_line];
    int max_kv = 64;
    char k[max_kv];
    char v[max_kv];
    map<string, map<string, string>> cfgs;

    FILE *fp = NULL;
    fp = fopen("config", "r");

    while(fgets(line, max_line, fp)) {
        if (line[0] != ' ') {
            sscanf(line, "%s", key);
            printf("key[%s]\n", key);
            map<string, string> cfg;
            cfgs[key] = cfg;
        } else {
            *strchr(line, ':') = ' ';
            sscanf(line, "%s %s", k, v);
            printf("k[%s] v[%s]\n", k, v);
            cfgs[key][k] = v;
        }
    }

    fclose(fp);

    printf("\ncfgs[chip_tb.skt1.top.apu0][psel] = %s\n", cfgs["chip_tb.skt1.top.apu0"]["psel"].c_str());
    return 0;
}
