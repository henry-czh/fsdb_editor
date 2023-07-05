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
#include "misc.h"

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

const unsigned int CHI_SIG_NUM = 2;

fsdbVarIdcode chi_sig_arr[CHI_SIG_NUM];

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
DumpData(ffrObject* fsdb_obj, FILE* dump_file);

static void
PrintAsVerilog(byte_T *ptr, uint_T size);

#endif