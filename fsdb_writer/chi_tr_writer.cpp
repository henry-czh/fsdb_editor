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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ffwAPI.h>
#include <string>
#include <map>

#include <sqlite3.h>

using namespace std;

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
typedef struct ReqFlitT
{
    ulong_T    time;
    uint_T     qos;
    uint_T    tgtid;
    uint_T    srcid;
    uint_T    txnid;
    union{
    uint_T    returnnid;
    uint_T    stashnid;
    }return_stashnid_cf;
    union{
    bool_T        stashnidvalid;
    bool_T        endian;
    }stashnidvalid_endian_cf;
    union {
    uint_T     returntxnid;
    uint_T     stashlpidvalid;
    uint_T     stashlpid;
    } returntxnid_stashlpid_cf;
    uint_T     opcode;
    uint_T     size;
    ulong_T    addr;
    bool_T        ns;
    bool_T        likelyshared;
    bool_T        allowretry;
    uint_T     order;
    bool_T        allocate;
    bool_T        cacheable;
    bool_T        device;
    bool_T        ewa;
    uint_T     snpattr;
    uint_T     lpid;
    union {
    bool_T        excl;
    bool_T        snoopme;
    } excl_snpme_cf;
    bool_T        expcompack;
} reqFlit;

typedef struct RspFlitT
{
    ulong_T    time;
    uint_T    tgtid;
    uint_T    srcid;
    //uint/6_t    txnid;
    uint_T 	txnid;
    uint_T     opcode;
    uint_T     resperr;
    uint_T     resp;
    union{
    uint_T     fwdstate;
    uint_T     datapull;
    uint_T     datasource;
    }fwdstate_datapull_datasource_cf;
    uint_T     dbid;
    uint_T     pcrdtype;
} rspFlit;

typedef struct DatFlitT
{
    ulong_T    time;
    uint_T    tgtid;
    uint_T    srcid;
    uint_T    txnid;
    uint_T    homenid;
    uint_T     opcode;
    uint_T     resperr;
    uint_T     resp;
    union{
    uint_T     fwdstate;
    uint_T     datapull;
    uint_T     datasource;
    }fwdstate_datapull_datasource_cf;
    uint_T     dbid;
    uint_T     ccid;
    uint_T     dataid;
    ulong_T    be;
    ulong_T    data[8];
} datFlit;

typedef struct SnpFlitT
{
    ulong_T    time;
    uint_T    srcid;
    uint_T    txnid;
    uint_T    fwdnid;
    uint_T    fwdtxnid;
    uint_T     opcode;
    ulong_T    addr;
    bool_T        ns;
    bool_T        donotgotosd;
    bool_T        rettosrc;
    bool_T        tracetag;
    uint_T    mpam;
} snpFlit;


//
// CHI Protocal variable definition and delcaration
//
fsdbStreamHdl   reqtracker_stream;
fsdbStreamHdl   snptracker_stream;
fsdbStreamHdl   req_stream;
fsdbStreamHdl   rxrsp_stream;
fsdbStreamHdl   rxdat_stream;
fsdbStreamHdl   txrsp_stream;
fsdbStreamHdl   txdat_stream;
fsdbStreamHdl   rxsnp_stream;

fsdbAttrHdl     qos_attr;
fsdbAttrHdl     tgtid_attr;
fsdbAttrHdl     srcid_attr;
fsdbAttrHdl     txnid_attr;
fsdbAttrHdl     fwdnid_attr;
fsdbAttrHdl     fwdtxnid_attr;
fsdbAttrHdl     returnnid_stashnid_attr;
fsdbAttrHdl     stashnidvalid_endian_attr;
fsdbAttrHdl     returntxnid_stashlpidvalid_stashlpid_attr;
fsdbAttrHdl     opcode_attr;
fsdbAttrHdl     size_attr;
fsdbAttrHdl     addr_attr;
fsdbAttrHdl     ns_attr;
fsdbAttrHdl     likelyshared_attr;
fsdbAttrHdl     allowretry_attr;
fsdbAttrHdl     order_attr;
fsdbAttrHdl     pcrdtype_attr;
fsdbAttrHdl     memattr_attr;
fsdbAttrHdl     cacheable_attr;
fsdbAttrHdl     allocate_attr;
fsdbAttrHdl     device_attr;
fsdbAttrHdl     ewa_attr;
fsdbAttrHdl     snpattr_attr;
fsdbAttrHdl     lpid_attr;
fsdbAttrHdl     excl_snoopme_attr;
fsdbAttrHdl     expcompack_attr;
fsdbAttrHdl     tracetag_attr;
fsdbAttrHdl     resperr_attr;
fsdbAttrHdl     resp_attr;
fsdbAttrHdl     fwdstate_datapull_datasource_attr;
fsdbAttrHdl     dbid_attr;
fsdbAttrHdl     donotgotosd_donotdatapull_attr;
fsdbAttrHdl     donotgotosd_attr;
fsdbAttrHdl     rettosrc_attr;
fsdbAttrHdl     homenid_attr;
fsdbAttrHdl     ccid_attr;
fsdbAttrHdl     dataid_attr;
fsdbAttrHdl     be_attr;
fsdbAttrHdl     data_attr;
fsdbAttrHdl     data_check_attr;
fsdbAttrHdl     poison_attr;
fsdbAttrHdl     mpam_attr;
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
__CreateAttr(ffwObject* ffw_obj, ffwObject *obj, const char* name,
             fsdbAttrDataType data_type, int larridx, int rarridx,
             bool_T hidden, fsdbBusDataType bus_data_type);

int ReadTracker();
void OpenSqliteDB();

//
// Main Program
//
sqlite3 *db = NULL;
ffwObject *ffw_obj = NULL;
int main(int argc, char *argv[])
{
    char        fname[] = "bus_tr.fsdb";

    //
    // Write to the fsdb file
    //
    __OpenFsdbFileForWrite(&ffw_obj, fname, FSDB_FT_VERILOG);
    __CreateTree(ffw_obj);
    OpenSqliteDB();
    ReadTracker();
    sqlite3_close(db);
    __CreateBusInfo(ffw_obj);
    ffw_Close(ffw_obj);

    exit(0);
}

fsdbAttrHdl
__CreateAttr(ffwObject* ffw_obj, const char* name, fsdbAttrDataType data_type,
             int larridx, int rarridx, bool_T hidden,
             fsdbBusDataType bus_data_type)
{
    ffwAttrArg attr_arg;

    memset ((void*)&attr_arg, 0, sizeof(ffwAttrArg));
    attr_arg.size = sizeof(ffwAttrArg);
    attr_arg.name = (str_T)name;
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

    //`
    // Create tree
    //
    ffw_BeginTree(ffw_obj);
    ffw_CreateScope(ffw_obj, FSDB_ST_VCD_MODULE, (str_T)"CHI-Analyzer");
    ffw_CreateScope(ffw_obj, FSDB_ST_VCD_MODULE, (str_T)"cpu0");
    ffw_CreateScope(ffw_obj, FSDB_ST_VCD_MODULE, (str_T)"core0");

    reqtracker_stream = ffw_CreateStream(ffw_obj, (str_T)"req_tracker");
    snptracker_stream = ffw_CreateStream(ffw_obj, (str_T)"snp_tracker");
    req_stream = ffw_CreateStream(ffw_obj, (str_T)"req_flit");
    txrsp_stream = ffw_CreateStream(ffw_obj, (str_T)"txrsp_flit");
    txdat_stream = ffw_CreateStream(ffw_obj, (str_T)"txdat_flit");
    rxrsp_stream = ffw_CreateStream(ffw_obj, (str_T)"rxrsp_flit");
    rxdat_stream = ffw_CreateStream(ffw_obj, (str_T)"rxdat_flit");
    rxsnp_stream = ffw_CreateStream(ffw_obj, (str_T)"rxsnp_flit");

    qos_attr                                    = __CreateAttr(ffw_obj, "qos", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    tgtid_attr                                  = __CreateAttr(ffw_obj, "tgtid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    srcid_attr                                  = __CreateAttr(ffw_obj, "srcid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    txnid_attr                                  = __CreateAttr(ffw_obj, "txnid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    fwdnid_attr                                 = __CreateAttr(ffw_obj, "fwdnid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    returnnid_stashnid_attr                     = __CreateAttr(ffw_obj, "returnnid_stashnid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    returntxnid_stashlpidvalid_stashlpid_attr   = __CreateAttr(ffw_obj, "returntxnid_stashlpidvalid_stashlpid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    stashnidvalid_endian_attr                   = __CreateAttr(ffw_obj, "stashnidvalid_endian", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    opcode_attr                                 = __CreateAttr(ffw_obj, "opcode", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    size_attr                                   = __CreateAttr(ffw_obj, "size", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    addr_attr                                   = __CreateAttr(ffw_obj, "addr", FSDB_ATTR_DT_UINT64, 0, 0, FALSE, FSDB_BDT_GENERIC);
    ns_attr                                     = __CreateAttr(ffw_obj, "ns", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    likelyshared_attr                           = __CreateAttr(ffw_obj, "likelyshared", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    allowretry_attr                             = __CreateAttr(ffw_obj, "allowretry", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    order_attr                                  = __CreateAttr(ffw_obj, "order", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    pcrdtype_attr                               = __CreateAttr(ffw_obj, "pcrdtype", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    cacheable_attr                              = __CreateAttr(ffw_obj, "cacheable", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    allocate_attr                               = __CreateAttr(ffw_obj, "allocate", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    device_attr                                 = __CreateAttr(ffw_obj, "device", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    ewa_attr                                    = __CreateAttr(ffw_obj, "ewa", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    snpattr_attr                                = __CreateAttr(ffw_obj, "snpattr", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    lpid_attr                                   = __CreateAttr(ffw_obj, "lpid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    excl_snoopme_attr                           = __CreateAttr(ffw_obj, "excl_snoopme", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    expcompack_attr                             = __CreateAttr(ffw_obj, "expcompack", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    tracetag_attr                               = __CreateAttr(ffw_obj, "tracetag", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    resperr_attr                                = __CreateAttr(ffw_obj, "resperr", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    resp_attr                                   = __CreateAttr(ffw_obj, "resp", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    fwdstate_datapull_datasource_attr           = __CreateAttr(ffw_obj, "fwdstate_datapull_datasource", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    dbid_attr                                   = __CreateAttr(ffw_obj, "dbid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    donotgotosd_donotdatapull_attr              = __CreateAttr(ffw_obj, "donotgotosd_donotdatapull", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    donotgotosd_attr                            = __CreateAttr(ffw_obj, "donotgotosd", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    rettosrc_attr                               = __CreateAttr(ffw_obj, "rettosrc", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_BDT_GENERIC);
    homenid_attr                                = __CreateAttr(ffw_obj, "homenid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    ccid_attr                                   = __CreateAttr(ffw_obj, "ccid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    dataid_attr                                 = __CreateAttr(ffw_obj, "dataid", FSDB_ATTR_DT_UINT32, 0, 0, FALSE, FSDB_BDT_GENERIC);
    be_attr                                     = __CreateAttr(ffw_obj, "be", FSDB_ATTR_DT_UINT64, 0, 0, FALSE, FSDB_BDT_GENERIC);
    data_attr                                   = __CreateAttr(ffw_obj, "data", FSDB_ATTR_DT_INT64, 0, 0, FALSE, FSDB_BDT_GENERIC);
    //data_check_attr                             = __CreateAttr(ffw_obj, "data_check", FSDB_ATTR_DT_INT64, 0, 0, FALSE, FSDB_BDT_GENERIC);
    poison_attr                                 = __CreateAttr(ffw_obj, "poison", FSDB_ATTR_DT_INT32, 0, 0, FALSE, FSDB_BDT_GENERIC);

    ffw_EndTree(ffw_obj);
}

fsdbTransId
__WriteOneRspFlit(ffwObject* ffw_obj, rspFlit rspflit, fsdbStreamHdl rsp_stream)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  aphase_val[9];
    fsdbTransId     rsp_trans;
    fsdbXTag btime, etime;

    fprintf(stdout, "rsp txnid = %x!\n", rspflit.txnid);
    aphase_val[0].hdl   = tgtid_attr;
    aphase_val[0].value = (byte_T *)&(rspflit.tgtid);
    aphase_val[1].hdl   = srcid_attr;
    aphase_val[1].value = (byte_T *)&(rspflit.srcid);
    aphase_val[2].hdl   = txnid_attr;
    aphase_val[2].value = (byte_T *)&(rspflit.txnid);
    aphase_val[3].hdl   = opcode_attr;
    aphase_val[3].value = (byte_T *)&(rspflit.opcode);
    aphase_val[4].hdl   = resperr_attr;
    aphase_val[4].value = (byte_T *)&(rspflit.resperr);
    aphase_val[5].hdl   = resp_attr;
    aphase_val[5].value = (byte_T *)&(rspflit.resp);
    aphase_val[6].hdl   = fwdstate_datapull_datasource_attr;
    aphase_val[6].value = (byte_T *)&(rspflit.fwdstate_datapull_datasource_cf);
    aphase_val[7].hdl   = dbid_attr;
    aphase_val[7].value = (byte_T *)&(rspflit.dbid);
    aphase_val[8].hdl   = pcrdtype_attr;
    aphase_val[8].value = (byte_T *)&(rspflit.pcrdtype);

    btime.hltag.H = 0;
    btime.hltag.L = rspflit.time;
    etime.hltag.H = 0;
    etime.hltag.L = rspflit.time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    rsp_trans = ffw_BeginTransaction(ffw_obj, rsp_stream, btime,
                                     (str_T)"rsp", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == rsp_trans) {
        fprintf(stderr, "rsp fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, rsp_trans, etime, aphase_val, 9)) {
        fprintf(stderr, "rsp fails during end!\n");
    }

    return rsp_trans;
}


fsdbTransId
__WriteOneDatFlit(ffwObject* ffw_obj, datFlit *datflit, fsdbStreamHdl dat_stream)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  aphase_val[13];
    fsdbTransId     dat_trans;
    fsdbXTag btime, etime;

    aphase_val[0].hdl   = tgtid_attr;
    aphase_val[0].value = (byte_T *)&(datflit->tgtid);
    aphase_val[1].hdl   = srcid_attr;
    aphase_val[1].value = (byte_T *)&(datflit->srcid);
    aphase_val[2].hdl   = txnid_attr;
    aphase_val[2].value = (byte_T *)&(datflit->txnid);
    aphase_val[3].hdl   = homenid_attr;
    aphase_val[3].value = (byte_T *)&(datflit->homenid);
    aphase_val[4].hdl   = opcode_attr;
    aphase_val[4].value = (byte_T *)&(datflit->opcode);
    aphase_val[5].hdl   = resperr_attr;
    aphase_val[5].value = (byte_T *)&(datflit->resperr);
    aphase_val[6].hdl   = resp_attr;
    aphase_val[6].value = (byte_T *)&(datflit->resp);
    aphase_val[7].hdl   = fwdstate_datapull_datasource_attr;
    aphase_val[7].value = (byte_T *)&(datflit->fwdstate_datapull_datasource_cf);
    aphase_val[8].hdl   = dbid_attr;
    aphase_val[8].value = (byte_T *)&(datflit->dbid);
    aphase_val[9].hdl   = ccid_attr;
    aphase_val[9].value = (byte_T *)&(datflit->ccid);
    aphase_val[10].hdl   = dataid_attr;
    aphase_val[10].value = (byte_T *)&(datflit->dataid);
    aphase_val[11].hdl   = be_attr;
    aphase_val[11].value = (byte_T *)&(datflit->be);
    aphase_val[12].hdl   = data_attr;
    aphase_val[12].value = (byte_T *)&(datflit->data);

    btime.hltag.H = 0;
    btime.hltag.L = datflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = datflit->time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    dat_trans = ffw_BeginTransaction(ffw_obj, dat_stream, btime,
                                     (str_T)"aphase", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == dat_trans) {
        fprintf(stderr, "dat fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, dat_trans, etime, aphase_val, 13)) {
        fprintf(stderr, "dat fails during end!\n");
    }

    return dat_trans;
}

fsdbTransId
__WriteOneSnpFlit(ffwObject* ffw_obj, snpFlit *snpflit, fsdbStreamHdl snp_stream)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  aphase_val[11];
    fsdbTransId     snp_trans;
    fsdbXTag btime, etime;

    aphase_val[0].hdl   = srcid_attr;
    aphase_val[0].value = (byte_T *)&(snpflit->srcid);
    aphase_val[1].hdl   = txnid_attr;
    aphase_val[1].value = (byte_T *)&(snpflit->txnid);
    aphase_val[2].hdl   = fwdnid_attr;
    aphase_val[2].value = (byte_T *)&(snpflit->fwdnid);
    aphase_val[3].hdl   = fwdtxnid_attr;
    aphase_val[3].value = (byte_T *)&(snpflit->fwdtxnid);
    aphase_val[4].hdl   = opcode_attr;
    aphase_val[4].value = (byte_T *)&(snpflit->opcode);
    aphase_val[5].hdl   = addr_attr;
    aphase_val[5].value = (byte_T *)&(snpflit->addr);
    aphase_val[6].hdl   = ns_attr;
    aphase_val[6].value = (byte_T *)&(snpflit->ns);
    aphase_val[7].hdl   = donotgotosd_attr;
    aphase_val[7].value = (byte_T *)&(snpflit->donotgotosd);
    aphase_val[8].hdl   = rettosrc_attr;
    aphase_val[8].value = (byte_T *)&(snpflit->rettosrc);
    aphase_val[9].hdl   = tracetag_attr;
    aphase_val[9].value = (byte_T *)&(snpflit->tracetag);
    aphase_val[10].hdl   = mpam_attr;
    aphase_val[10].value = (byte_T *)&(snpflit->mpam);

    btime.hltag.H = 0;
    btime.hltag.L = snpflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = snpflit->time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    snp_trans = ffw_BeginTransaction(ffw_obj, snp_stream, btime,
                                     (str_T)"snp", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == snp_trans) {
        fprintf(stderr, "snp fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, snp_trans, etime, aphase_val, 11)) {
        fprintf(stderr, "snp during end!\n");
    }

    return snp_trans;
}

fsdbTransId
__WriteOneReqFlit(ffwObject* ffw_obj, reqFlit *reqflit)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  dphase_val[22];
    fsdbTransId     req_trans;
    fsdbXTag btime, etime;


    dphase_val[0].hdl   = qos_attr;
    dphase_val[0].value = (byte_T *)&(reqflit->qos);
    dphase_val[1].hdl   = tgtid_attr;
    dphase_val[1].value = (byte_T *)&(reqflit->tgtid);
    dphase_val[2].hdl   = srcid_attr;
    dphase_val[2].value = (byte_T *)&(reqflit->srcid);
    dphase_val[3].hdl   = txnid_attr;
    dphase_val[3].value = (byte_T *)&(reqflit->txnid);
    dphase_val[4].hdl   = returnnid_stashnid_attr;
    dphase_val[4].value = (byte_T *)&(reqflit->return_stashnid_cf);
    dphase_val[5].hdl   = stashnidvalid_endian_attr;
    dphase_val[5].value = (byte_T *)&(reqflit->stashnidvalid_endian_cf);
    dphase_val[6].hdl   = returntxnid_stashlpidvalid_stashlpid_attr;
    dphase_val[6].value = (byte_T *)&(reqflit->returntxnid_stashlpid_cf);
    dphase_val[7].hdl   = opcode_attr;
    dphase_val[7].value = (byte_T *)&(reqflit->opcode);
    dphase_val[8].hdl   = size_attr;
    dphase_val[8].value = (byte_T *)&(reqflit->size);
    dphase_val[9].hdl   = addr_attr;
    dphase_val[9].value = (byte_T *)&(reqflit->addr);
    dphase_val[10].hdl   = ns_attr;
    dphase_val[10].value = (byte_T *)&(reqflit->ns);
    dphase_val[11].hdl   = likelyshared_attr;
    dphase_val[11].value = (byte_T *)&(reqflit->likelyshared);
    dphase_val[12].hdl   = allowretry_attr;
    dphase_val[12].value = (byte_T *)&(reqflit->allowretry);
    dphase_val[13].hdl   = order_attr;
    dphase_val[13].value = (byte_T *)&(reqflit->order);
    dphase_val[14].hdl   = allocate_attr;
    dphase_val[14].value = (byte_T *)&(reqflit->cacheable);
    dphase_val[15].hdl   = device_attr;
    dphase_val[15].value = (byte_T *)&(reqflit->device);
    dphase_val[16].hdl   = ewa_attr;
    dphase_val[16].value = (byte_T *)&(reqflit->ewa);
    dphase_val[17].hdl   = snpattr_attr;
    dphase_val[17].value = (byte_T *)&(reqflit->snpattr);
    dphase_val[18].hdl   = lpid_attr;
    dphase_val[18].value = (byte_T *)&(reqflit->lpid);
    dphase_val[19].hdl   = excl_snoopme_attr;
    dphase_val[19].value = (byte_T *)&(reqflit->excl_snpme_cf);
    dphase_val[20].hdl   = expcompack_attr;
    dphase_val[20].value = (byte_T *)&(reqflit->expcompack);
    dphase_val[21].hdl   = returnnid_stashnid_attr;
    dphase_val[21].value = (byte_T *)&(reqflit->return_stashnid_cf);

    btime.hltag.H = 0;
    btime.hltag.L = reqflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = reqflit->time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    req_trans = ffw_BeginTransaction(ffw_obj, req_stream, btime,
                                     (str_T)"req", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == req_trans) {
        fprintf(stderr, "req fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, req_trans, etime, dphase_val, 22)) {
        fprintf(stderr, "req fails during end!\n");
    }

    return req_trans;
}

fsdbTransId
__WriteOneReqTracker(ffwObject* ffw_obj, reqFlit *reqflit, rspFlit *rspflit)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  tr_val[5];
    fsdbTransId     tr_trans;
    fsdbXTag btime, etime;

    tr_val[0].hdl   = opcode_attr;
    tr_val[0].value = (byte_T *)&(reqflit->opcode);
    tr_val[1].hdl   = addr_attr;
    tr_val[1].value = (byte_T *)&(reqflit->addr);
    tr_val[2].hdl   = size_attr;
    tr_val[2].value = (byte_T *)&(reqflit->size);
    tr_val[3].hdl   = txnid_attr;
    tr_val[3].value = (byte_T *)&(reqflit->txnid);
    tr_val[4].hdl   = dbid_attr;
    tr_val[4].value = (byte_T *)&(rspflit->dbid);

    btime.hltag.H = 0;
    btime.hltag.L = reqflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = rspflit->time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    tr_trans = ffw_BeginTransaction(ffw_obj, reqtracker_stream, btime,
                                    (str_T)"reqtracker", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == tr_trans) {
        fprintf(stderr, "reqtracker fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, tr_trans, etime, tr_val, 5)) {
        fprintf(stderr, "reqtracker fails during end!\n");
    }

    return tr_trans;
}
//
// __CreateVC()
//
void
__CreateVC(ffwObject* ffw_obj)
{
    reqFlit reqflit;
    rspFlit rspflit;

    fsdbTransId     reqtracker_trans, req_trans, rsp_trans;

    fsdbRelationHdl relation1 = ffw_CreateRelation(ffw_obj, (str_T)"parent-child");


    // transfer 2
    reqflit.time = 7;
    reqflit.opcode = 0x14;
    reqflit.addr = 0x900000000;
    reqflit.txnid = 0x01;

    rspflit.time = 9;
    rspflit.tgtid = 0xb;
    rspflit.srcid = 0x1;
    rspflit.txnid = 0x02;
    rspflit.opcode = 0x15;
    rspflit.resperr = 0x0;
    rspflit.resp= 0x1;
    rspflit.fwdstate_datapull_datasource_cf.fwdstate = 0x0;
    rspflit.dbid = 0x9;
    rspflit.pcrdtype= 0x0;


    req_trans = __WriteOneReqFlit(ffw_obj, &reqflit);

    rsp_trans = __WriteOneRspFlit(ffw_obj, rspflit, rxrsp_stream);

    reqtracker_trans = __WriteOneReqTracker(ffw_obj, &reqflit, &rspflit);

    ffw_AddRelation(ffw_obj, relation1, req_trans, rsp_trans);
    ffw_AddRelation(ffw_obj, relation1, rsp_trans, req_trans);
    ffw_AddRelation(ffw_obj, relation1, req_trans, reqtracker_trans);
    ffw_AddRelation(ffw_obj, relation1, rsp_trans, reqtracker_trans);
    ffw_AddRelation(ffw_obj, relation1, reqtracker_trans, req_trans);
    ffw_AddRelation(ffw_obj, relation1, reqtracker_trans, rsp_trans);

    // transfer 1
    reqflit.time = 5;
    reqflit.opcode = 0x14;
    reqflit.addr = 0x800000000;
    reqflit.txnid = 0x01;

    rspflit.time = 6;
    rspflit.tgtid = 0xa;
    rspflit.srcid = 0x0;
    rspflit.txnid = 0x01;
    rspflit.opcode = 0x14;
    rspflit.resperr = 0x1;
    rspflit.resp= 0x0;
    rspflit.fwdstate_datapull_datasource_cf.fwdstate = 0x01;
    rspflit.dbid = 0x80;
    rspflit.pcrdtype= 0x0;

    rsp_trans = __WriteOneRspFlit(ffw_obj, rspflit, rxrsp_stream);

    reqtracker_trans = __WriteOneReqTracker(ffw_obj, &reqflit, &rspflit);

    req_trans = __WriteOneReqFlit(ffw_obj, &reqflit);

    ffw_AddRelation(ffw_obj, relation1, req_trans, rsp_trans);
    ffw_AddRelation(ffw_obj, relation1, rsp_trans, req_trans);
    ffw_AddRelation(ffw_obj, relation1, req_trans, reqtracker_trans);
    ffw_AddRelation(ffw_obj, relation1, rsp_trans, reqtracker_trans);
    ffw_AddRelation(ffw_obj, relation1, reqtracker_trans, req_trans);
    ffw_AddRelation(ffw_obj, relation1, reqtracker_trans, rsp_trans);

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
    ffw_SetScaleUnit(*fsdb_obj, (str_T)"1p");
    ffw_WarnSuppress(TRUE);
}

//
// __CreateBusInfo()
//
void
__CreateBusInfo(ffwObject* ffw_obj)
{
    fsdbBusHdl bus = ffw_CreateBus(ffw_obj, (str_T)"ahb1", (str_T)AHB_PROTOCOL);

    // Setting bus clock information
    ffw_SetBusClock(ffw_obj, bus, (str_T)"/system/hclk", FSDB_CLOCK_POSEDGE);

    // Add streams to bus
    ffw_AddBusStream(ffw_obj, bus, reqtracker_stream);
    ffw_AddBusStream(ffw_obj, bus, req_stream);
    ffw_AddBusStream(ffw_obj, bus, rxrsp_stream);

    // Set bus parameters
    ffw_AddBusParameter(ffw_obj, bus, (str_T)"ADDR_WIDTH", (str_T)"32");
    ffw_AddBusParameter(ffw_obj, bus, (str_T)"DATA_WIDTH", (str_T)"32");
    ffw_AddBusParameter(ffw_obj, bus, (str_T)"MASTER_COUNT", (str_T)"2");
    ffw_AddBusParameter(ffw_obj, bus, (str_T)"SLAVE_COUNT", (str_T)"3");
}

void Callback(void *data, int col_count, char** col_values, char** col_names, map<string, string>& values)
{
    for (int i = 0; i < col_count; i++) {
        values[col_names[i]] = col_values[i];
    }
}

static int Callback0(void *data, int col_count, char** col_values, char** col_names, const char* col_end_time)
{
    fsdbAttrHdlVal tr_val[col_count];
    fsdbXTag btime, etime;

    for (int i = 0; i < col_count; i++) {
        tr_val[i].hdl = __CreateAttr(ffw_obj, col_names[i], FSDB_ATTR_DT_STRING, 0, 0, false, FSDB_BDT_GENERIC);
        tr_val[i].value = col_values[i] ? (byte_T*)(&col_values[i]) : (byte_T*)(&"NULL");

        if (col_values[i] && !strcmp("time", col_names[i])) {
            btime.hltag.H = 0;
            btime.hltag.L = atoi(col_values[i]);
            continue;
        }
        if (col_values[i] && !strcmp(col_end_time, col_names[i])) {
            etime.hltag.H = 0;
            etime.hltag.L = atoi(col_values[i]) + 1;
            continue;
        }
    }
    fsdbTag64 xtag = {btime.hltag.H, btime.hltag.L};
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    fsdbTransId tr_trans = ffw_BeginTransaction(ffw_obj, reqtracker_stream, btime,
                                        (str_T)"reqtracker", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == tr_trans) {
        fprintf(stderr, "reqtracker fails during begin!\n");
    }
    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, tr_trans, etime, tr_val, col_count)) {
        fprintf(stderr, "reqtracker fails during end!\n");
    }
    return 0;
}

static int CallbackTracker(void *data, int col_count, char** col_values, char** col_names)
{
    map<string, string> values;
    Callback(data, col_count, col_values, col_names, values);

    fsdbAttrHdlVal tr_val[col_count];

    for (int i = 0; i < col_count; i++) {
        tr_val[i].hdl = __CreateAttr(ffw_obj, col_names[i], FSDB_ATTR_DT_STRING, 0, 0, false, FSDB_BDT_GENERIC);
        tr_val[i].value = col_values[i] ? (byte_T*)(&col_values[i]) : (byte_T*)(&"NULL");
    }

    fsdbXTag btime, etime;
    btime.hltag.H = 0;
    btime.hltag.L = atoi(values["time"].c_str());
    etime.hltag.H = 0;
    etime.hltag.L = atoi(values["end_time"].c_str());

    fsdbTag64 xtag = {btime.hltag.H, btime.hltag.L};
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    fsdbTransId tr_trans = ffw_BeginTransaction(ffw_obj, reqtracker_stream, btime,
                                        (str_T)"reqtracker", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == tr_trans) {
        fprintf(stderr, "reqtracker fails during begin!\n");
    }
    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, tr_trans, etime, tr_val, col_count)) {
        fprintf(stderr, "reqtracker fails during end!\n");
    }

    int reqFlit_id = atoi(values["reqFlit_id"].c_str());
    printf("reqFlit_id[%d]\n", reqFlit_id);
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

int ReadTracker()
{
    const char* sql = "select * from v_reqTrackerT order by time";

    char* zErrMsg = 0;
    const char* data = "Callback function called";
    int rc = sqlite3_exec(db, sql, CallbackTracker, (void*)data, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    return 0;
}

