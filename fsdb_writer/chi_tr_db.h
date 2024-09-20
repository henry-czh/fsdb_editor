
typedef struct {
    int db_id;
    const char* tb_name;
    fsdbStreamHdl fsdb_stream;
} FieldInfo;

int ReadTracker(FieldInfo*);
int ReadReqFlit(FieldInfo*);
void OpenSqliteDB();
void CloseSqliteDB();
