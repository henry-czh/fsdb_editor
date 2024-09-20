#include <map>
#include <sqlite3.h>
#include <ffwAPI.h>

#include "chi_tr_db.h"

using namespace std;

sqlite3 *db = NULL;
extern ffwObject *ffw_obj;
extern fsdbStreamHdl reqtracker_stream;
extern fsdbStreamHdl snptracker_stream;
extern fsdbStreamHdl req_stream;
extern fsdbStreamHdl rxrsp_stream;
extern fsdbStreamHdl rxdat_stream;
extern fsdbStreamHdl txrsp_stream;
extern fsdbStreamHdl txdat_stream;
extern fsdbStreamHdl rxsnp_stream;
extern fsdbRelationHdl tracker_relation;
extern fsdbTransId tracker_trans_id;

fsdbAttrHdl __CreateAttr(ffwObject* ffw_obj, const char* name, fsdbAttrDataType data_type,
             int larridx, int rarridx, bool_T hidden, fsdbBusDataType bus_data_type);

void Callback(void *data, int col_count, char** col_values, char** col_names, map<string, char*>& values)
{
    for (int i = 0; i < col_count; i++) {
        values[col_names[i]] = col_values[i];
    }
}

fsdbTransId WriteField(int col_count, char** col_values, char** col_names, map<string, char*>& values, const char* end_time, fsdbStreamHdl stream_hdl, str_T sig_name)
{
    fsdbAttrHdlVal tr_val[col_count];

    for (int i = 0; i < col_count; i++) {
        tr_val[i].hdl = __CreateAttr(ffw_obj, col_names[i], FSDB_ATTR_DT_STRING, 0, 0, false, FSDB_BDT_GENERIC);
        tr_val[i].value = col_values[i] ? (byte_T*)(&col_values[i]) : (byte_T*)(&"NULL");
    }

    fsdbXTag btime, etime;
    btime.hltag.H = 0;
    btime.hltag.L = atoi(values["time"]);
    etime.hltag.H = 0;
    etime.hltag.L = atoi(values[end_time]) + 1;

    fsdbTag64 xtag = {btime.hltag.H, btime.hltag.L};
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    fsdbTransId tr_trans = ffw_BeginTransaction(ffw_obj, stream_hdl, btime,
                                                sig_name, NULL, 0);
    if (FSDB_INVALID_TRANS_ID == tr_trans) {
        fprintf(stderr, "reqtracker fails during begin!\n");
    }
    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, tr_trans, etime, tr_val, col_count)) {
        fprintf(stderr, "reqtracker fails during end!\n");
    }
    return tr_trans;
}

static int CallbackTracker(void* data, int col_count, char** col_values, char** col_names)
{
    map<string, char*> values;
    Callback(data, col_count, col_values, col_names, values);
    FieldInfo* field_info = (FieldInfo*)data;
    tracker_trans_id = WriteField(col_count, col_values, col_names, values, "end_time", field_info->fsdb_stream, (str_T)field_info->tb_name);

    FieldInfo field_infos[] = {
        {atoi(values["reqFlit_id"]),    "reqFlit", req_stream},
        {atoi(values["SnpFlitT0_id"]),  "snpFlit", rxsnp_stream},
        {atoi(values["SnpFlitT1_id"]),  "snpFlit", rxsnp_stream},
        {atoi(values["rxrspFlit_id"]),  "rspFlit", rxrsp_stream},
        {atoi(values["txrspFlit_id"]),  "rspFlit", txrsp_stream},
        {atoi(values["rxdatFlit0_id"]), "datFlit", rxdat_stream},
        {atoi(values["txdatFlit1_id"]), "datFlit", txdat_stream},
    };
    for (int i = 0; i < 7; i++) {
        ReadReqFlit(&field_infos[i]);
    }
    return 0;
}

static int CallbackReqFlit(void* data, int col_count, char** col_values, char** col_names)
{
    map<string, char*> values;
    Callback(data, col_count, col_values, col_names, values);
    FieldInfo* field_info = (FieldInfo*)data;
    fsdbTransId trans_id = WriteField(col_count, col_values, col_names, values, "time",
                                      field_info->fsdb_stream, (str_T)field_info->tb_name);
    ffw_AddRelation(ffw_obj, tracker_relation, tracker_trans_id, trans_id);
    return 0;
}

void OpenSqliteDB()
{
    int rc = sqlite3_open("./chi_analyzer-2.sqlite", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
}

void CloseSqliteDB()
{
    sqlite3_close(db);
}

int ReadTracker(FieldInfo* field_info)
{
    char sql[128];
    sprintf(sql, "select * from v_%s order by time", field_info->tb_name);

    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, CallbackTracker, (void*)field_info, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    return 0;
}

int ReadReqFlit(FieldInfo* field_info)
{
    char sql[128];
    sprintf(sql, "select * from v_%s where id = %d", field_info->tb_name, field_info->db_id);

    char* zErrMsg = NULL;
    int rc = sqlite3_exec(db, sql, CallbackReqFlit, (void*)field_info, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return 0;
}
