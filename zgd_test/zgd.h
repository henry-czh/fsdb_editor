typedef struct {
    str_T               name;           // signal name
    fsdbVarType         type;           // signal type
    ushort_T            lbitnum;        // signal left bit number
    ushort_T            rbitnum;        // signal right bit number
    fsdbBytesPerBit     bpb;            // signal bytes per bit 用几个软件的byte表示硬件的一个bit

    byte_T              *value;         // signal value
    uint_T              byte_count;     // byte count of signal value
} BusSignal;

void SetSig(ffwObject* fsdb_obj, BusSignal* sig, fsdbTag64 time, int value);
