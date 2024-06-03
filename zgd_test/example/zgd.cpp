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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <map>

#include "ffwAPI.h"
#include "sqlhelper.h"
#include "zgd.h"

using namespace std;
map<string, BusSignal*> sig_map;

ffwVarMapId vm_id;
ffwObject   *fsdb_obj;
fsdbTag64   g_time;
int         sig_count;

BusSignal sig_arr[] = {
    {(str_T)"tgtid",            FSDB_VT_VCD_WIRE, 4,  10},
    {(str_T)"srcid",            FSDB_VT_VCD_WIRE, 11, 17},
    {(str_T)"txnid",            FSDB_VT_VCD_WIRE, 18, 25},
    // {(str_T)"opcode",        FSDB_VT_VCD_WIRE, 26, 29},
    {(str_T)"resperr",          FSDB_VT_VCD_WIRE, 30, 31},
    {(str_T)"resp",             FSDB_VT_VCD_WIRE, 32, 34},
    {(str_T)"datapull_data",    FSDB_VT_VCD_WIRE, 35, 37},
    {(str_T)"dbid",             FSDB_VT_VCD_WIRE, 38, 45},
    {(str_T)"pcrdtype",         FSDB_VT_VCD_WIRE, 46, 49},
};

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

void AddWireSig(BusSignal* sig)
{
    sig_map[sig->name] = sig;

    vm_id = ffw_CreateVarByHandle(fsdb_obj, sig->type,
                        FSDB_VD_OUTPUT, FSDB_DT_HANDLE_VERILOG_STANDARD,
                        sig->lbitnum, sig->rbitnum, sig,
                        sig->name, (fsdbBytesPerBit)sig->bpb);
    if (NULL == vm_id) {
        printf("failed to create a var(%s)\n", sig->name);
        exit(0);
    }
    sig->value = AllocateMemory(vm_id->bitSize, (fsdbBytesPerBit)sig->bpb, sig->byte_count);
    printf("bitSize[%d] byte_count[%d]\n", vm_id->bitSize, sig->byte_count);
}

void SetSig(ffwObject* fsdb_obj, BusSignal* sig, fsdbTag64 g_time, int value)
{
    ffw_CreateXCoorByHnL(fsdb_obj, g_time.H, g_time.L);
    SetValue(sig->value, sig->byte_count, value);
    ffw_CreateVarValueByHandle(fsdb_obj, sig, sig->value);
}

int main(int argc, str_T argv[])
{
    // sig_map.clear();

    int         bus_vc_count = 36;
    str_T       env_ptr;
    int         i, j;

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
    ffw_CreateScope(fsdb_obj, FSDB_ST_VCD_MODULE, (str_T)"reqFlit");
    ffw_CreateUpscope(fsdb_obj);
    ffw_CreateScope(fsdb_obj, FSDB_ST_VCD_MODULE, (str_T)"rspFlit");

    sig_count = sizeof(sig_arr) / sizeof(BusSignal);
    for (int i = 0; i < sig_count; i++) {
        printf("sig: %s\n", sig_arr[i].name);
        AddWireSig(&sig_arr[i]);
    }

    ffw_EndTree(fsdb_obj);

    ReadSig();

    ffw_Close(fsdb_obj);
    return 0;
}
