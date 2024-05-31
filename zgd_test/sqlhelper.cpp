#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>

#include <sqlite3.h>
#include "ffwAPI.h"
#include "zgd.h"

using namespace std;

extern ffwObject* fsdb_obj;
extern fsdbTag64  g_time;
extern map<string, BusSignal*> sigs;
extern BusSignal  txnid_sig;
extern BusSignal  srcid_sig;

static int callback(void *data, int col_count, char** col_values, char** col_names)
{
    // fprintf(stderr, "data zzz: %s\n", (const char *)data);
    for (int i = 0; i < col_count; i++) {
        const char* col_value = col_values[i] ? col_values[i] : "NULL";
        // printf("%s = %s\n", col_names[i], col_value);
        if (col_value && !strcmp("time", col_names[i])) {
            g_time.L = atoi(col_value);
            // printf("%s = %d\n", col_names[i], g_time.L);
        }

        if (col_value && !strcmp("txnid", col_names[i])) {
            // printf("%s = %s\n", col_names[i], col_value);
            // SetSig(fsdb_obj, sigs["txnid"], g_time, atoi(col_value));
            SetSig(fsdb_obj, &txnid_sig, g_time, atoi(col_value));
        }

        // if (col_value && !strcmp("srcid", col_names[i])) {
        //     // printf("%s = %s\n", col_names[i], col_value);
        //     SetSig(fsdb_obj, &srcid_sig, g_time, atoi(col_value));
        // }
    }
    // printf("\n");
    return 0;
}

int ReadSig()
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    const char *sql;
    const char *data = "Callback function called";

    /* Open database */
    rc = sqlite3_open("../fsdb_writer/chi_analyzer.sqlite", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sql = "SELECT * from rspFlit order by time";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void *)data, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);
    return 0;
}
