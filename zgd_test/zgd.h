typedef struct {
    str_T               name;           // signal name
    fsdbVarType         type;           // signal type
    ushort_T            lbitnum;        // signal left bit number
    ushort_T            rbitnum;        // signal right bit number
    fsdbBytesPerBit     bpb;            // signal bytes per bit

    byte_T              *value;         // signal value
    uint_T              byte_count;     // byte count of signal value
} BusSignal;

void SetSig(ffwObject* fsdb_obj, BusSignal* sig, fsdbTag64 time, int value);
