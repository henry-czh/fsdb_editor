/* *****************************************************************************
// [bus_tr_recording.cpp]
//
//  Copyright 2004-2009 SPRINGSOFT. All Rights Reserved.
//
// Except as specified in the license terms of SPRINGSOFT, this material may not be copied, modified,
// re-published, uploaded, executed, or distributed in any way, in any medium,
// in whole or in part, without prior written permission from SPRINGSOFT.
// ****************************************************************************/
//
// Program Name : bus_tr_recording.cpp
// Purpose      : Demonstrate the use of bus information recording APIs.
// Description  : In this file, there are three stream type signals, at each time 
//                use ffw_BeginTransaction and ffw_EndTransaction to write attr, 
//                then switch to next swignal   .                     
//
#ifdef NOVAS_FSDB
    #undef NOVAS_FSDB
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ffwAPI.h>

//
// Constants
//
#ifndef TRUE
    const int TRUE = 1;
#endif

#ifndef FALSE
    const int FALSE = 0;
#endif

//
// Types And Typedefs
//

//
// Global variable definition and declaration
//
fsdbStreamHdl   aphase_stream;
fsdbStreamHdl   dphase_stream;
fsdbStreamHdl   transfer_stream;
fsdbAttrHdl     addr_attr;
fsdbAttrHdl     data_attr;
fsdbAttrHdl     cmd_attr;
fsdbAttrHdl     master_attr;
fsdbAttrHdl     slave_attr;
fsdbAttrHdl     size_attr;
fsdbAttrHdl     resp_attr;

//
// Function Declaration
//
static void
__CreateTree(ffwObject* ffw_obj);

static void
__CreateVC(ffwObject* ffw_obj);

static void
__OpenFsdbFileForWrite(ffwObject **fsdb_obj, char fname[],
    fsdbFileType file_type);

static void
__CreateBusInfo(ffwObject* ffw_obj);

static fsdbAttrHdl
__CreateAttr(ffwObject* ffw_obj, ffwObject *obj, str_T name,
             fsdbAttrDataType data_type, int larridx, int rarridx,
             bool_T hidden, fsdbBusDataType bus_data_type);

//
// Main Program
//
int
main(int argc, char *argv[])
{
    ffwObject   *ffw_obj = NULL;
    char        fname[] = "bus_tr.fsdb";

    //
    // Write to the fsdb file
    //
    __OpenFsdbFileForWrite(&ffw_obj, fname, FSDB_FT_VERILOG);
    __CreateTree(ffw_obj);
    __CreateVC(ffw_obj);
    __CreateBusInfo(ffw_obj);
    ffw_Close(ffw_obj);

    exit(0);
}

fsdbAttrHdl
__CreateAttr(ffwObject* ffw_obj, str_T name, fsdbAttrDataType data_type,
             int larridx, int rarridx, bool_T hidden,
             fsdbBusDataType bus_data_type)
{
    ffwAttrArg attr_arg;

    memset ((void*)&attr_arg, 0, sizeof(ffwAttrArg));
    attr_arg.size = sizeof(ffwAttrArg);
    attr_arg.name = name;
    attr_arg.data_type = data_type;
    attr_arg.larridx = larridx;
    attr_arg.rarridx = rarridx;
    attr_arg.hidden = hidden;
    attr_arg.meta_info = bus_data_type;     // set bus data type as meta_info

    return ffw_CreateAttr2(ffw_obj, &attr_arg);
}

//
// __CreateTree()
//
void
__CreateTree(ffwObject* ffw_obj)
{
    //
    // Call ffw_GetDataTypeCreationReady() and prepare to create
    // transaction attributes.
    //
    ffw_GetDataTypeCreationReady(ffw_obj);

    //
    // Create tree
    //
    ffw_BeginTree(ffw_obj);
    ffw_CreateScope(ffw_obj, FSDB_ST_VCD_MODULE, "Top");

    aphase_stream = ffw_CreateStream(ffw_obj, "addr_phase");
    dphase_stream = ffw_CreateStream(ffw_obj, "data_phase");
    transfer_stream = ffw_CreateStream(ffw_obj, "transfer");

    cmd_attr = __CreateAttr(ffw_obj, "Command", FSDB_ATTR_DT_STRING,
                            0, 0, FALSE, FSDB_BDT_COMMAND);
    addr_attr = __CreateAttr(ffw_obj, "address", FSDB_ATTR_DT_UINT32, 0, 0,
                             FALSE, FSDB_BDT_ADDRESS);
    data_attr = __CreateAttr(ffw_obj, "data", FSDB_ATTR_DT_UINT32, 0, 0,
                             FALSE, FSDB_BDT_DATA);
    master_attr = __CreateAttr(ffw_obj, "Master", FSDB_ATTR_DT_STRING, 0, 0,
                               FALSE, FSDB_BDT_MASTER);
    slave_attr = __CreateAttr(ffw_obj, "Slave", FSDB_ATTR_DT_STRING, 0, 0,
                              FALSE, FSDB_BDT_SLAVE);
    size_attr = __CreateAttr(ffw_obj, "Size", FSDB_ATTR_DT_UINT32, 0, 0,
                             FALSE, FSDB_BDT_GENERIC);
    resp_attr = __CreateAttr(ffw_obj, "Response", FSDB_ATTR_DT_STRING, 0, 0,
                             FALSE, FSDB_BDT_GENERIC);

    ffw_EndTree(ffw_obj);
}

fsdbTransId
__WriteOneAphase(ffwObject* ffw_obj, fsdbXTag btime, fsdbXTag etime,
                 str_T cmd, uint_T addr, uint_T size,
                 str_T master, str_T slave)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  aphase_val[5];
    fsdbTransId     aphase_trans;

    aphase_val[0].hdl   = cmd_attr;
    aphase_val[0].value = (byte_T *)&cmd;
    aphase_val[1].hdl   = addr_attr;
    aphase_val[1].value = (byte_T *)&addr;
    aphase_val[2].hdl   = size_attr;
    aphase_val[2].value = (byte_T *)&size;
    aphase_val[3].hdl   = master_attr;
    aphase_val[3].value = (byte_T *)&master;
    aphase_val[4].hdl   = slave_attr;
    aphase_val[4].value = (byte_T *)&slave;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    aphase_trans = ffw_BeginTransaction(ffw_obj, aphase_stream, btime,
                                        "aphase", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == aphase_trans) {
        fprintf(stderr, "aphase fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, aphase_trans, etime, aphase_val, 5)) {
        fprintf(stderr, "aphase fails during end!\n");
    }

    return aphase_trans;
}

fsdbTransId
__WriteOneDphase(ffwObject* ffw_obj, fsdbXTag btime, fsdbXTag etime,
                 uint_T data, uint_T size, str_T resp, str_T master,
                 str_T slave)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  dphase_val[5];
    fsdbTransId     dphase_trans;

    dphase_val[0].hdl   = data_attr;
    dphase_val[0].value = (byte_T *)&data;
    dphase_val[1].hdl   = size_attr;
    dphase_val[1].value = (byte_T *)&size;
    dphase_val[2].hdl   = resp_attr;
    dphase_val[2].value = (byte_T *)&resp;
    dphase_val[3].hdl   = master_attr;
    dphase_val[3].value = (byte_T *)&master;
    dphase_val[4].hdl   = slave_attr;
    dphase_val[4].value = (byte_T *)&slave;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    dphase_trans = ffw_BeginTransaction(ffw_obj, dphase_stream, btime,
                                        "dphase", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == dphase_trans) {
        fprintf(stderr, "dphase fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, dphase_trans, etime, dphase_val, 5)) {
        fprintf(stderr, "dphase fails during end!\n");
    }

    return dphase_trans;
}

fsdbTransId
__WriteOneTransfer(ffwObject* ffw_obj, fsdbXTag btime, fsdbXTag etime,
                   str_T cmd, uint_T addr, uint_T data, uint_T size,
                   str_T resp, str_T master, str_T slave)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  tr_val[7];
    fsdbTransId     tr_trans;

    tr_val[0].hdl   = cmd_attr;
    tr_val[0].value = (byte_T *)&cmd;
    tr_val[1].hdl   = addr_attr;
    tr_val[1].value = (byte_T *)&addr;
    tr_val[2].hdl   = data_attr;
    tr_val[2].value = (byte_T *)&data;
    tr_val[3].hdl   = size_attr;
    tr_val[3].value = (byte_T *)&size;
    tr_val[4].hdl   = resp_attr;
    tr_val[4].value = (byte_T *)&resp;
    tr_val[5].hdl   = master_attr;
    tr_val[5].value = (byte_T *)&master;
    tr_val[6].hdl   = slave_attr;
    tr_val[6].value = (byte_T *)&slave;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    tr_trans = ffw_BeginTransaction(ffw_obj, transfer_stream, btime,
                                        "transfer", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == tr_trans) {
        fprintf(stderr, "transfer fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, tr_trans, etime, tr_val, 7)) {
        fprintf(stderr, "transfer fails during end!\n");
    }

    return tr_trans;
}
//
// __CreateVC()
//
void
__CreateVC(ffwObject* ffw_obj)
{
    fsdbTag64       xtag;
    fsdbXTag        time;
    fsdbXTag        btime;
    fsdbXTag        etime;
    fsdbAttrHdlVal  aphase_val[5];
    fsdbAttrHdlVal  dphase_val[5];

    str_T           cmd;
    str_T           master, slave, resp;
    uint_T          addr, data, size;

    fsdbTransId     aphase_trans, dphase_trans, tr_trans;
    fsdbRelationHdl relation1 = ffw_CreateRelation(ffw_obj, "parent-child");

    // transfer 1
    cmd = "burst read(noseq)";
    addr = 0x20;
    data = 0x11223344;
    size = 4;
    master = "CPU";
    slave = "MEM_CTRL";
    resp = "OKAY";

    btime.hltag.H       = 0;
    btime.hltag.L       = 100;
    etime.hltag.H       = 0;
    etime.hltag.L       = 200;
    aphase_trans = __WriteOneAphase(ffw_obj, btime, etime, cmd, addr, size, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 200;
    etime.hltag.H       = 0;
    etime.hltag.L       = 300;
    dphase_trans = __WriteOneDphase(ffw_obj, btime, etime, data, size, resp, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 100;
    etime.hltag.H       = 0;
    etime.hltag.L       = 300;
    tr_trans = __WriteOneTransfer(ffw_obj, btime, etime, cmd, addr, data,
                                  size, resp, master, slave);

    ffw_AddRelation(ffw_obj, relation1, aphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, dphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, aphase_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, dphase_trans);

    // transfer 2
    cmd = "burst read(seq)";
    addr = 0x24;
    data = 0x55667788;
    size = 4;
    master = "CPU";
    slave = "MEM_CTRL";
    resp = "OKAY";

    btime.hltag.H       = 0;
    btime.hltag.L       = 200;
    etime.hltag.H       = 0;
    etime.hltag.L       = 300;
    aphase_trans = __WriteOneAphase(ffw_obj, btime, etime, cmd, addr, size, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 300;
    etime.hltag.H       = 0;
    etime.hltag.L       = 400;
    dphase_trans = __WriteOneDphase(ffw_obj, btime, etime, data, size, resp, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 200;
    etime.hltag.H       = 0;
    etime.hltag.L       = 400;
    tr_trans = __WriteOneTransfer(ffw_obj, btime, etime, cmd, addr, data,
                                  size, resp, master, slave);

    ffw_AddRelation(ffw_obj, relation1, aphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, dphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, aphase_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, dphase_trans);

    // transfer 3
    cmd = "burst read(seq)";
    addr = 0x28;
    data = 0x99aabbcc;
    size = 4;
    master = "CPU";
    slave = "MEM_CTRL";
    resp = "OKAY";

    btime.hltag.H       = 0;
    btime.hltag.L       = 300;
    etime.hltag.H       = 0;
    etime.hltag.L       = 400;
    aphase_trans = __WriteOneAphase(ffw_obj, btime, etime, cmd, addr, size, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 400;
    etime.hltag.H       = 0;
    etime.hltag.L       = 800;
    dphase_trans = __WriteOneDphase(ffw_obj, btime, etime, data, size, resp, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 300;
    etime.hltag.H       = 0;
    etime.hltag.L       = 800;
    tr_trans = __WriteOneTransfer(ffw_obj, btime, etime, cmd, addr, data,
                                  size, resp, master, slave);

    ffw_AddRelation(ffw_obj, relation1, aphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, dphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, aphase_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, dphase_trans);

    // transfer 4
    cmd = "burst read(seq)";
    addr = 0x2c;
    data = 0xddeeff00;
    size = 4;
    master = "CPU";
    slave = "MEM_CTRL";
    resp = "OKAY";

    btime.hltag.H       = 0;
    btime.hltag.L       = 400;
    etime.hltag.H       = 0;
    etime.hltag.L       = 800;
    aphase_trans = __WriteOneAphase(ffw_obj, btime, etime, cmd, addr, size, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 800;
    etime.hltag.H       = 0;
    etime.hltag.L       = 900;
    dphase_trans = __WriteOneDphase(ffw_obj, btime, etime, data, size, resp, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 400;
    etime.hltag.H       = 0;
    etime.hltag.L       = 900;
    tr_trans = __WriteOneTransfer(ffw_obj, btime, etime, cmd, addr, data,
                                  size, resp, master, slave);

    ffw_AddRelation(ffw_obj, relation1, aphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, dphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, aphase_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, dphase_trans);

    // transfer 5
    cmd = "single write";
    addr = 0x80000000;
    data = 0xaabbccdd;
    size = 4;
    master = "CPU";
    slave = "APB_BRIDGE";
    resp = "OKAY";

    btime.hltag.H       = 0;
    btime.hltag.L       = 1200;
    etime.hltag.H       = 0;
    etime.hltag.L       = 1300;
    aphase_trans = __WriteOneAphase(ffw_obj, btime, etime, cmd, addr, size, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 1300;
    etime.hltag.H       = 0;
    etime.hltag.L       = 1500;
    dphase_trans = __WriteOneDphase(ffw_obj, btime, etime, data, size, resp, master, slave);

    btime.hltag.H       = 0;
    btime.hltag.L       = 1200;
    etime.hltag.H       = 0;
    etime.hltag.L       = 1500;
    tr_trans = __WriteOneTransfer(ffw_obj, btime, etime, cmd, addr, data,
                                  size, resp, master, slave);

    ffw_AddRelation(ffw_obj, relation1, aphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, dphase_trans, tr_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, aphase_trans);
    ffw_AddRelation(ffw_obj, relation1, tr_trans, dphase_trans);
}

//
// __OpenFsdbFileForWrite()
//
void
__OpenFsdbFileForWrite(ffwObject **fsdb_obj, char fname[],
    fsdbFileType file_type)
{
    // Open fsdb file
    *fsdb_obj = ffw_Open(fname, file_type);
    if (NULL == *fsdb_obj) {
        printf("Fail to open a fast file object! File name = %s.\n", fname);
        exit(FSDB_RC_OBJECT_CREATION_FAILED);
    }

    // Use handle scheme for tree creation
    ffw_CreateTreeByHandleScheme(*fsdb_obj);
    ffw_SetScaleUnit(*fsdb_obj, "1p");
    ffw_WarnSuppress(TRUE);
}

//
// __CreateBusInfo()
//
void
__CreateBusInfo(ffwObject* ffw_obj)
{
    fsdbBusHdl bus = ffw_CreateBus(ffw_obj, "ahb1", AHB_PROTOCOL);

    // Setting bus clock information
    ffw_SetBusClock(ffw_obj, bus, "/system/hclk", FSDB_CLOCK_POSEDGE);

    // Add streams to bus
    ffw_AddBusStream(ffw_obj, bus, aphase_stream);
    ffw_AddBusStream(ffw_obj, bus, dphase_stream);
    ffw_AddBusStream(ffw_obj, bus, transfer_stream);

    // Set bus parameters
    ffw_AddBusParameter(ffw_obj, bus, "ADDR_WIDTH", "32");
    ffw_AddBusParameter(ffw_obj, bus, "DATA_WIDTH", "32");
    ffw_AddBusParameter(ffw_obj, bus, "MASTER_COUNT", "2");
    ffw_AddBusParameter(ffw_obj, bus, "SLAVE_COUNT", "3");
}
