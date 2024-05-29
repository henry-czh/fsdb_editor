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
typedef struct ReqFlitT
{
    uint64_t    time;
    uint8_t     qos;
    uint16_t    tgtid;
    uint16_t    srcid;
    uint16_t    txnid;
    union{
    uint16_t    returnnid;
    uint16_t    stashnid;
    }return_stashnid_cf;
    union{
    bool        stashnidvalid;
    bool        endian;
    }stashnidvalid_endian_cf;
    union {
    uint8_t     returntxnid;
    uint8_t     stashlpidvalid;
    uint8_t     stashlpid;
    } returntxnid_stashlpid_cf;
    uint8_t     opcode;
    uint8_t     size;
    uint64_t    addr;
    bool        ns;
    bool        likelyshared;
    bool        allowretry;
    uint8_t     order;
    bool        allocate;
    bool        cacheable;
    bool        device;
    bool        ewa;
    uint8_t     snpattr;
    uint8_t     lpid;
    union {
    bool        excl;
    bool        snoopme;
    } excl_snpme_cf;
    bool        expcompack;
} reqFlit;

typedef struct RspFlitT
{
    uint64_t    time;
    uint16_t    tgtid;
    uint16_t    srcid;
    uint16_t    txnid;
    uint8_t     opcode;
    uint8_t     resperr;
    uint8_t     resp;
    union{
    uint8_t     fwdstate;
    uint8_t     datapull;
    uint8_t     datasource;
    }fwdstate_datapull_datasource_cf;
    uint8_t     dbid;
    uint8_t     pcrdtype;
} rspFlit;

typedef struct DatFlitT
{
    uint64_t    time;
    uint16_t    tgtid;
    uint16_t    srcid;
    uint16_t    txnid;
    uint16_t    homenid;
    uint8_t     opcode;
    uint8_t     resperr;
    uint8_t     resp;
    union{
    uint8_t     fwdstate;
    uint8_t     datapull;
    uint8_t     datasource;
    }fwdstate_datapull_datasource_cf;
    uint16_t     dbid;
    uint8_t     ccid;
    uint8_t     dataid;
    uint64_t    be;
    uint64_t    data[8];
} datFlit;

typedef struct SnpFlitT
{
    uint64_t    time;
    uint16_t    srcid;
    uint16_t    txnid;
    uint16_t    fwdnid;
    uint16_t    fwdtxnid;
    uint8_t     opcode;
    uint64_t    addr;
    bool        ns;
    bool        donotgotosd;
    bool        rettosrc;
    bool        tracetag;
    uint16_t    mpam;
} snpFlit;


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

// CHI Protocal variable definition and delcaration
fsdbStreamHdl   reqtracker_stream;
fsdbStreamHdl   snptracker_stream;
fsdbStreamHdl   req_stream;
fsdbStreamHdl   rxrsp_stream;
fsdbStreamHdl   rxdat_stream;
fsdbStreamHdl   txrsp_stream;
fsdbStreamHdl   txrsp_stream;
fsdbStreamHdl   txdat_stream;
fsdbStreamHdl   rxsnp_stream;

fsdbAttrHdl     qos_attr;
fsdbAttrHdl     tgtid_attr;
fsdbAttrHdl     srcid_attr;
fsdbAttrHdl     txnid_attr;
fsdbAttrHdl     fwdnid_attr;
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
fsdbAttrHdl     rettosrc_attr;
fsdbAttrHdl     homenid_attr;
fsdbAttrHdl     ccid_attr;
fsdbAttrHdl     be_attr;
fsdbAttrHdl     data_attr;
fsdbAttrHdl     data_check_attr;
fsdbAttrHdl     poison_attr;
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

    //`
    // Create tree
    //
    ffw_BeginTree(ffw_obj);
    ffw_CreateScope(ffw_obj, FSDB_ST_VCD_MODULE, "CHI-Analyzer");

    req_stream = ffw_CreateStream(ffw_obj, "req_flit");
    txrsp_stream = ffw_CreateStream(ffw_obj, "txrsp_flit");
    txdat_stream = ffw_CreateStream(ffw_obj, "txdat_flit");
    rxrsp_stream = ffw_CreateStream(ffw_obj, "rxrsp_flit");
    rxdat_stream = ffw_CreateStream(ffw_obj, "rxdat_flit");
    rxsnp_stream = ffw_CreateStream(ffw_obj, "rxsnp_flit");

    qos_attr                                    = __CreateAttr(ffw_obj, "qos_attr                                 ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    tgtid_attr                                  = __CreateAttr(ffw_obj, "tgtid_attr                               ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    srcid_attr                                  = __CreateAttr(ffw_obj, "srcid_attr                               ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    txnid_attr                                  = __CreateAttr(ffw_obj, "txnid_attr                               ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    fwdnid_attr                                 = __CreateAttr(ffw_obj, "fwdnid_attr                              ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    returnnid_stashnid_attr                     = __CreateAttr(ffw_obj, "returnnid_stashnid_attr                  ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    stashnidvalid_endian_attr                   = __CreateAttr(ffw_obj, "stashnidvalid_endian_attr                ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    returntxnid_stashlpidvalid_stashlpid_attr   = __CreateAttr(ffw_obj, "returntxnid_stashlpidvalid_stashlpid_attr", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    opcode_attr                                 = __CreateAttr(ffw_obj, "opcode_attr                              ", FSDB_ATTR_DT_INT32, 0, 0, FALSE, FSDB_DBT_DATA);
    size_attr                                   = __CreateAttr(ffw_obj, "size_attr                                ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    addr_attr                                   = __CreateAttr(ffw_obj, "addr_attr                                ", FSDB_ATTR_DT_INT64, 0, 0, FALSE, FSDB_DBT_DATA);
    ns_attr                                     = __CreateAttr(ffw_obj, "ns_attr                                  ", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_DBT_DATA);
    likelyshared_attr                           = __CreateAttr(ffw_obj, "likelyshared_attr                        ", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_DBT_DATA);
    order_attr                                  = __CreateAttr(ffw_obj, "order_attr                               ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    pcrdtype_attr                               = __CreateAttr(ffw_obj, "pcrdtype_attr                            ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    memattr_attr                                = __CreateAttr(ffw_obj, "memattr_attr                             ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    snpattr_attr                                = __CreateAttr(ffw_obj, "snpattr_attr                             ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    lpid_attr                                   = __CreateAttr(ffw_obj, "lpid_attr                                ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    excl_snoopme_attr                           = __CreateAttr(ffw_obj, "excl_snoopme_attr                        ", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_DBT_DATA);
    expcompack_attr                             = __CreateAttr(ffw_obj, "expcompack_attr                          ", FSDB_ATTR_DT_BOOL, 0, 0, FALSE, FSDB_DBT_DATA);
    tracetag_attr                               = __CreateAttr(ffw_obj, "tracetag_attr                            ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    resperr_attr                                = __CreateAttr(ffw_obj, "resperr_attr                             ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    resp_attr                                   = __CreateAttr(ffw_obj, "resp_attr                                ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    fwdstate_datapull_datasource_attr           = __CreateAttr(ffw_obj, "fwdstate_datapull_datasource_attr        ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    dbid_attr                                   = __CreateAttr(ffw_obj, "dbid_attr                                ", FSDB_ATTR_DT_INT32, 0, 0, FALSE, FSDB_DBT_DATA);
    donotgotosd_donotdatapull_attr              = __CreateAttr(ffw_obj, "donotgotosd_donotdatapull_attr           ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    rettosrc_attr                               = __CreateAttr(ffw_obj, "rettosrc_attr                            ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    homenid_attr                                = __CreateAttr(ffw_obj, "homenid_attr                             ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    ccid_attr                                   = __CreateAttr(ffw_obj, "ccid_attr                                ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);
    be_attr                                     = __CreateAttr(ffw_obj, "be_attr                                  ", FSDB_ATTR_DT_INT64, 0, 0, FALSE, FSDB_DBT_DATA);
    data_attr                                   = __CreateAttr(ffw_obj, "data_attr                                ", FSDB_ATTR_DT_INT64, 0, 0, FALSE, FSDB_DBT_DATA);
    data_check_attr                             = __CreateAttr(ffw_obj, "data_check_attr                          ", FSDB_ATTR_DT_INT64, 0, 0, FALSE, FSDB_DBT_DATA);
    poison_attr                                 = __CreateAttr(ffw_obj, "poison_attr                              ", FSDB_ATTR_DT_CHAR, 0, 0, FALSE, FSDB_DBT_DATA);

    ffw_EndTree(ffw_obj);
}

fsdbTransId
__WriteOneRspFlit(ffwObject* ffw_obj, *rspFlit rspflit, *fsdbStreamHdl rsp_stream)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  aphase_val[9];
    fsdbTransId     aphase_trans;

    aphase_val[0].hdl   = tgtid_attr;
    aphase_val[0].value = (byte_T *)&(rspflit->tgtid);
    aphase_val[1].hdl   = srcid_attr;
    aphase_val[1].value = (byte_T *)&(rspflit->srcid);
    aphase_val[2].hdl   = txnid_attr;
    aphase_val[2].value = (byte_T *)&(rspflt->txnid);
    aphase_val[3].hdl   = opcode_attr;
    aphase_val[3].value = (byte_T *)&(rspflit->opcode);
    aphase_val[4].hdl   = resperr_attr;
    aphase_val[4].value = (byte_T *)&(rspflit->resperr);
    aphase_val[5].hdl   = resp;
    aphase_val[5].value = (byte_T *)&(rspflit->resp);
    aphase_val[6].hdl   = fwdstate_datapull_datasource_attr;
    aphase_val[6].value = (byte_T *)&(rspflit->fwdstate_datafull_datasouce_cf);
    aphase_val[7].hdl   = dbid_attr;
    aphase_val[7].value = (byte_T *)&(rspflit->dbid);
    aphase_val[8].hdl   = pcrdtype_attr;
    aphase_val[8].value = (byte_T *)&(rspflit->pcrdtype);

    btime.hltag.H = 0;
    btime.hltag.L = rspflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = rspflit->time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    rsp_trans = ffw_BeginTransaction(ffw_obj, rsp_stream, btime,
                                        "aphase", NULL, 0);
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
__WriteOneDatFlit(ffwObject* ffw_obj, *datFlit datflit, *fsdbStreamHdl dat_stream)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  aphase_val[13];
    fsdbTransId     aphase_trans;

    aphase_val[0].hdl   = tgtid_attr;
    aphase_val[0].value = (byte_T *)&(datflit->tgtid);
    aphase_val[1].hdl   = srcid_attr;
    aphase_val[1].value = (byte_T *)&(datflit->srcid);
    aphase_val[2].hdl   = txnid_attr;
    aphase_val[2].value = (byte_T *)&(rspflt->txnid);
    aphase_val[3].hdl   = homenid_attr;
    aphase_val[3].value = (byte_T *)&(datflit->homenid);
    aphase_val[3].hdl   = opcode_attr;
    aphase_val[3].value = (byte_T *)&(datflit->opcode);
    aphase_val[4].hdl   = resperr_attr;
    aphase_val[4].value = (byte_T *)&(datflit->resperr);
    aphase_val[5].hdl   = resp;
    aphase_val[5].value = (byte_T *)&(datflit->resp);
    aphase_val[6].hdl   = fwdstate_datapull_datasource_attr;
    aphase_val[6].value = (byte_T *)&(datflit->fwdstate_datafull_datasouce_cf);
    aphase_val[7].hdl   = dbid_attr;
    aphase_val[7].value = (byte_T *)&(datflit->dbid);
    aphase_val[7].hdl   = ccid_attr;
    aphase_val[7].value = (byte_T *)&(datflit->ccid);
    aphase_val[7].hdl   = dataid_attr;
    aphase_val[7].value = (byte_T *)&(datflit->dataid);
    aphase_val[8].hdl   = be_attr;
    aphase_val[8].value = (byte_T *)&(datflit->be);
    aphase_val[8].hdl   = data_attr;
    aphase_val[8].value = (byte_T *)&(datflit->data;

    btime.hltag.H = 0;
    btime.hltag.L = datflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = datflit->time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    dat_trans = ffw_BeginTransaction(ffw_obj, dat_stream, btime,
                                        "aphase", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == dat_trans) {
        fprintf(stderr, "dat fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, dat_trans, etime, aphase_val, 13)) {
        fprintf(stderr, "aphase fails during end!\n");
    }

    return dat_trans;
}

fsdbTransId
__WriteOneSnpFlit(ffwObject* ffw_obj, *snpFlit snpflit, *fsdbStreamHdl snp_stream)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  aphase_val[11];
    fsdbTransId     aphase_trans;

    uint16_t    srcid;
    uint16_t    txnid;
    uint16_t    fwdnid;
    uint16_t    fwdtxnid;
    uint8_t     opcode;
    uint64_t    addr;
    bool        ns;
    bool        donotgotosd;
    bool        rettosrc;
    bool        tracetag;
    uint16_t    mpam;

    aphase_val[1].hdl   = srcid_attr;
    aphase_val[1].value = (byte_T *)&(snpflit->srcid);
    aphase_val[2].hdl   = txnid_attr;
    aphase_val[2].value = (byte_T *)&(rspflt->txnid);
    aphase_val[3].hdl   = fwdnid_attr;
    aphase_val[3].value = (byte_T *)&(snpflit->fwdnid);
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
    aphase_val[7].hdl   = rettosrc_attr;
    aphase_val[7].value = (byte_T *)&(snpflit->rettosrc);
    aphase_val[7].hdl   = tracetag_attr;
    aphase_val[7].value = (byte_T *)&(snpflit->tracetag);
    aphase_val[8].hdl   = mpam_attr;
    aphase_val[8].value = (byte_T *)&(snpflit->mpam);

    btime.hltag.H = 0;
    btime.hltag.L = snpflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = snpflit->time + 1;

    xtag.H = btime.hltag.H;
    xtag.L = btime.hltag.L;
    ffw_CreateXCoorByHnL(ffw_obj, xtag.H, xtag.L);
    snp_trans = ffw_BeginTransaction(ffw_obj, snp_stream, btime,
                                        "aphase", NULL, 0);
    if (FSDB_INVALID_TRANS_ID == snp_trans) {
        fprintf(stderr, "snp fails during begin!\n");
    }

    if (FSDB_RC_SUCCESS !=
        ffw_EndTransaction(ffw_obj, snp_trans, etime, aphase_val, 11)) {
        fprintf(stderr, "aphase fails during end!\n");
    }

    return snp_trans;
}

fsdbTransId
__WriteOneReqFlit(ffwObject* ffw_obj, reqFlit *reqflit)
{
    fsdbTag64       xtag;
    fsdbAttrHdlVal  dphase_val[22];
    fsdbTransId     req_trans;

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
    dphase_val[22].hdl   = returnnid_stashnid_attr;
    dphase_val[22].value = (byte_T *)&(reqflit->return_stashnid_cf);

    fsdbXTag btime;
    fsdbXTag etime;

    btime.hltag.H = 0;
    btime.hltag.L = reqflit->time;
    etime.hltag.H = 0;
    etime.hltag.L = reqflit->time + 1;

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
__WriteOneReqTracker(ffwObject* ffw_obj, fsdbXTag btime, fsdbXTag etime,
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
