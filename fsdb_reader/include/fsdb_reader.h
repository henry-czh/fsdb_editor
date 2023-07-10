/* *****************************************************************************
// [fsdb_reader.h]
//
//  Copyright 2023 Chaozhanghu. All Rights Reserved.
//
// ****************************************************************************/
//
// Program Name	: fsdb_reader.h
//


//
// FSDB_READER is internally used in NOVAS
//
#ifndef __FSDB_READER_H
#define __FSDB_READER_H

#include "ffrAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <regex>
#include "misc.h"

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

const unsigned int CHI_SIG_NUM = 20;

typedef struct
{
    int rstn;
    int txreqflitpend_pre;
    int txreqflitpend;
    int txreqflitv;
    byte_T txreqflit[20];

    int txrspflitpend_pre;
    int txrspflitpend;
    int txrspflitv;
    byte_T txrspflit[20];

    int txdatflitpend_pre;
    int txdatflitpend;
    int txdatflitv;
    byte_T txdatflit[75];

    int rxrspflitpend_pre;
    int rxrspflitpend;
    int rxrspflitv;
    byte_T rxrspflit[20];

    int rxdatflitpend_pre;
    int rxdatflitpend;
    int rxdatflitv;
    byte_T rxdatflit[75];

    int rxsnpflitpend_pre;
    int rxsnpflitpend;
    int rxsnpflitv;
    byte_T rxsnpflit[20];
} chi_sig_t;

enum CHIChannel {TXREQ, TXRSP, TXDAT, RXRSP, RXDAT, RXSNP};

//
// The tree callback function, it's used to traverse the design 
// hierarchies. 
//
//static bool_T __MyTreeCB(fsdbTreeCBType cb_type, 
//			 void *client_data, void *tree_cb_data);
bool_T __MyTreeCB(fsdbTreeCBType, void*, void*);


//
// dump scope definition
//
static void 
__DumpScope(fsdbTreeCBDataScope *scope, char* match_path);


//
// dump var definition 
// 
static void 
__DumpVar(fsdbTreeCBDataVar *var, SIGNAL_MAP* signal_map);


static void 
__PrintTimeValChng(ffrVCTrvsHdl vc_trvs_hdl, 
		   fsdbTag64 *time, byte_T *vc_ptr);

void
loadSignals(ffrObject* fsdb_obj, SIGNAL_MAP signal_map);

void
dumpCHIData(ffrObject* fsdb_obj, map<string,sigInfo> chi_sig_map);

void
writeCHIdat(map<int, string>, byte_T*, fsdbXTag, fsdbVarIdcode, chi_sig_t*);

static void
PrintAsVerilog(byte_T *ptr, uint_T size);

static void
PrintAsVerilogHex(byte_T *ptr, fsdbVarIdcode, map<int, sigInfo>, byte_T*);

#endif