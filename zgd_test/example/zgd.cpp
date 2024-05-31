/* *****************************************************************************
// [bus.cpp]
//
//  Copyright 2000-2009 SPRINGSOFT. All Rights Reserved.
//
// Except as specified in the license terms of SPRINGSOFT, this material may not be copied, modified,
// re-published, uploaded, executed, or distributed in any way, in any medium,
// in whole or in part, without prior written permission from SPRINGSOFT.
// ****************************************************************************/
//
// Program Name : bus.cpp
// Purpose      : Demostrate how to create bus signal and vlaue
// Description  : In this file, it create several bus signals
//                and their value change.
//

#undef NOVAS_FSDB

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "ffwAPI.h"
#include "sqlhelper.h"
#include "zgd.h"

BusSignal txnid_sig = {
    (str_T)"txnid",
    FSDB_VT_VCD_TRIREG,
    0,
    3,
    FSDB_BYTES_PER_BIT_1B,
    NULL
};

BusSignal srcid_sig = {
    (str_T)"srcid",
    FSDB_VT_VCD_TRIREG,
    0,
    3,
    FSDB_BYTES_PER_BIT_1B,
    NULL
};


#define ARR_DIM(a)                (sizeof(a)/sizeof(a[0]))



//
// bit_size     = ABS(lbitnum - rbitnum) + 1, the maximum bit size is FSDB_MAX_BIT_SIZE
// byte_count   = bit_size * bytes_per_bit
//
static byte_T* AllocateMemory(ushort_T bit_size, fsdbBytesPerBit bpb, uint_T &byte_count)
{
    byte_T*   ptr;

    switch(bpb) {
    case FSDB_BYTES_PER_BIT_1B:
        byte_count = 1*bit_size;
        break;
    case FSDB_BYTES_PER_BIT_2B:
        byte_count = 2*bit_size;
        break;
    case FSDB_BYTES_PER_BIT_4B:
        byte_count = 4*bit_size;
        break;
    case FSDB_BYTES_PER_BIT_8B:
        byte_count = 8*bit_size;
        break;
    default:
        printf("unknown bpb.\n");
        exit(-1);
    }
    ptr = (byte_T*)calloc(1, byte_count);
    if(NULL == ptr) {
        printf("Memory resources exhausted.\n");
        exit(-1);
    }
    return ptr;
}

void SetValue(byte_T* value, int n, int v)
{
    while(n--) {
        value[n] = v & 1; //FSDB_BT_VCD_1
        v = v >> 1;
    }
}

void SetSig(ffwObject* fsdb_obj, BusSignal* sig, fsdbTag64 time, int value)
{
    ffw_CreateXCoorByHnL(fsdb_obj, time.H, time.L);
    SetValue(sig->value, sig->byte_count, value);
    ffw_CreateVarValueByHandle(fsdb_obj, sig, sig->value);
}

ffwObject   *fsdb_obj;
fsdbTag64   time;

int main(int argc, str_T argv[])
{
    int         bus_vc_count = 36;
    str_T       env_ptr;
    int         i, j;
    ffwVarMapId vm_id;

    env_ptr = getenv("BUS_VC_COUNT");
    if (NULL != env_ptr)
        bus_vc_count = atoi(env_ptr);

    fsdb_obj = ffw_Open((str_T)"zgd.fsdb", FSDB_FT_VERILOG);
    if (NULL == fsdb_obj) {
        fprintf(stdout, "failed to create a fsdb file.\n");
        exit(-1);
    }
    ffw_SetAnnotation(fsdb_obj, (str_T)"my annotation: test vc creation of bus.");
    fprintf(stdout, "fsdb version: %s\n", ffw_GetFsdbVersion());

    ffw_CreateTreeByHandleScheme(fsdb_obj);
    ffw_SetScaleUnit(fsdb_obj, (str_T)"0.01n");

    //
    // Tree1
    //
    ffw_BeginTree(fsdb_obj);
    ffw_CreateScope(fsdb_obj, FSDB_ST_VCD_MODULE, (str_T)"top");
    ffw_CreateScope(fsdb_obj, FSDB_ST_VCD_MODULE, (str_T)"scope"); //用 xxx.yyy 并没有用

    vm_id = ffw_CreateVarByHandle(fsdb_obj, txnid_sig.type,
                        FSDB_VD_OUTPUT, FSDB_DT_HANDLE_VERILOG_STANDARD,
                        txnid_sig.lbitnum, txnid_sig.rbitnum, &txnid_sig,
                        txnid_sig.name, (fsdbBytesPerBit)txnid_sig.bpb);

    if (NULL == vm_id) {
        printf("failed to create a var(%s)\n", txnid_sig.name);
        exit(0);
    }
    //
    // allocate memory for storing value change and set up initial value change.
    //
    txnid_sig.value = AllocateMemory(vm_id->bitSize,
        (fsdbBytesPerBit)txnid_sig.bpb, txnid_sig.byte_count);
    for (i = 0; i < txnid_sig.byte_count; i++)
         txnid_sig.value[i] = i & 0x03;
    //
    // For verilog vcd, the value change can be 0, 1, x and z, we define it as
    // fsdbBitType(enum type) in fsdbShr.h header file, the value is from 0 to 3,
    // hence we perform "i & 0x03" to make the value within [0, 3].
    //
    // Since the bytes per bit of txnid_sig is 1 byte, so we assign each
    // bit value based on byte unit; if its bytes per bit is 4 bytes, then we
    // have to assign each bit based on 4 bytes unit.
    //

    //
    // allocate memory for storing value change and set up initial value change.
    //

    printf("bitSize[%d] byte_count[%d]\n", vm_id->bitSize, txnid_sig.byte_count);

    vm_id = ffw_CreateVarByHandle(fsdb_obj, srcid_sig.type,
                        FSDB_VD_OUTPUT, FSDB_DT_HANDLE_VERILOG_STANDARD,
                        srcid_sig.lbitnum, srcid_sig.rbitnum, &srcid_sig,
                        srcid_sig.name, (fsdbBytesPerBit)srcid_sig.bpb);

    if (NULL == vm_id) {
        printf("failed to create a var(%s)\n", srcid_sig.name);
        exit(0);
    }
    //
    // allocate memory for storing value change and set up initial value change.
    //
    srcid_sig.value = AllocateMemory(vm_id->bitSize,
        (fsdbBytesPerBit)srcid_sig.bpb, srcid_sig.byte_count);
    for (i = 0; i < srcid_sig.byte_count; i++)
         srcid_sig.value[i] = i & 0x03;

    ffw_EndTree(fsdb_obj);

    ReadSig();

    /*
    //
    // create initial value change for each var
    //
    time.L++;
    SetSig(fsdb_obj, time, 0);

    //
    // create the time(xtag)
    //
    time.L++;
    SetSig(fsdb_obj, time, 1);

    time.L++;
    SetSig(fsdb_obj, time, 2);

    time.L++;
    SetSig(fsdb_obj, time, 3);

    time.L++;
    SetSig(fsdb_obj, time, 4);

    time.L++;
    SetSig(fsdb_obj, time, 5);

    time.L++;
    SetSig(fsdb_obj, time, 6);
    */


    ffw_Close(fsdb_obj);
    return 0;
}
