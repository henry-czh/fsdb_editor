/* *****************************************************************************
// [fsdb_reader.cpp]
//
//  Copyright 2023 Chaozhanghu. All Rights Reserved.
//
// ****************************************************************************/
//
// Program Name	: fsdb_reader.cpp
//
// Purpose	: Demonstrate how to call fsdb reader APIs to access 
//		  the value changes of verilog type fsdb.
//


#include "ffrAPI.h"
#include <stdio.h>
#include <stdlib.h>

#include "fsdb_reader.h"

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

int 
main(int argc, char *argv[])
{
    if (2 != argc) {
	fprintf(stderr, "usage: read_verilog verilog_type_fsdb\n");
	return FSDB_RC_FAILURE;
    }

	//
	// 读取配置文件，指定哪些s信号将从fsdb中读出
	//
	SIGNAL_MAP signal_map;
	signal_map = read_config();

    // 
    // check the file to see if it's a fsdb file or not.
    //
    if (FALSE == ffrObject::ffrIsFSDB(argv[1])) {
	fprintf(stderr, "%s is not an fsdb file.\n", argv[1]);
	return FSDB_RC_FAILURE;
    }

    ffrFSDBInfo fsdb_info;

    ffrObject::ffrGetFSDBInfo(argv[1], fsdb_info);
    if (FSDB_FT_VERILOG != fsdb_info.file_type) {
  	fprintf(stderr, "file type is not verilog.\n");
	return FSDB_RC_FAILURE;
    }

    //
    // Open the fsdb file.
    //
    // From fsdb v2.0(Debussy 5.0), there are two APIs to open a 
    // fsdb file: ffrOpen() and ffrOpen2(). Both APIs take three 
    // parameters, the first one is the fsdb file name, the second 
    // one is a tree callback function written by application, the 
    // last one is the client data that application would like 
    // fsdb reader to pass it back in tree callback function.
    //
    // Open a fsdb file with ffrOpen(), the tree callback function
    // will be activated many times during open session; open a fsdb
    // file with ffrOpen2(), the tree callback function will not be
    // activated during open session, applicaiton has to call an API
    // called "ffrReadScopeVarTree()" to activate the tree callback
    // function. 
    // 
    // In tree callback function, application can tell what the
    // callback data is, based on the callback type. For example, if 
    // the callback type is scope(FFR_TREE_CBT_SCOPE), then 
    // applicaiton knows that it has to perform (fsdbTreeCBDataScope*) 
    // type case on the callback data so that it can read the scope 
    // defition.
    //
    ffrObject *fsdb_obj =
	ffrObject::ffrOpen3(argv[1]);
    if (NULL == fsdb_obj) {
	fprintf(stderr, "ffrObject::ffrOpen() failed.\n");
	exit(FSDB_RC_OBJECT_CREATION_FAILED);
    }
    fsdb_obj->ffrSetTreeCBFunc(__MyTreeCB, &signal_map);

    if (FSDB_FT_VERILOG != fsdb_obj->ffrGetFileType()) {
	fprintf(stderr, 
		"%s is not verilog type fsdb, just return.\n", argv[1]);
	fsdb_obj->ffrClose();
	return FSDB_RC_SUCCESS;
    }

    //
    // Activate the tree callback funciton, read the design 
    // hierarchies. Application has to perform proper type case 
    // on tree callback data based on the callback type, then uses 
    // the type case structure view to access the wanted data.
    //
    fsdb_obj->ffrReadScopeVarTree();

    //
    // Each unique var is represented by a unique idcode in fsdb 
    // file, these idcodes are positive integer and continuous from 
    // the smallest to the biggest one. So the maximum idcode also 
    // means that how many unique vars are there in this fsdb file. 
    //
    // Application can know the maximum var idcode by the following
    // API:
    //
    //		ffrGetMaxVarIdcode()
    //

    /*
    * 遍历配置信息中列出的信号，将信号的idcode加入signalList，load相关信号
    * 变化到内存中。
    */
    loadSignals(fsdb_obj, signal_map);

    FILE *chi_dump_file = NULL;
    chi_dump_file = fopen("chi_analyzer.dat", "wb+");

    DumpData(fsdb_obj, chi_dump_file);
/* czh  

    //
    // In order to traverse the value changes of a specific var,
    // application must create a value change traverse handle for 
    // that sepcific var. Once the value change traverse handle is 
    // created successfully, there are lots of traverse functions 
    // available to traverse the value changes backward and forward, 
    // or jump to a sepcific time, etc.
    //
    ffrVCTrvsHdl vc_trvs_hdl = 
	fsdb_obj->ffrCreateVCTraverseHandle(max_var_idcode); 
    if (NULL == vc_trvs_hdl) {
	fprintf(stderr, "Failed to create a traverse handle(%u)\n", 
		max_var_idcode);
	exit(FSDB_RC_OBJECT_CREATION_FAILED);
    }


    fsdbTag64 time;
    int	      glitch_num;
    byte_T    *vc_ptr;

    //
    // Check to see if this var has value changes or not.
    //
    if (FALSE == vc_trvs_hdl->ffrHasIncoreVC()) {
        fprintf(stderr, 
	        "This var(%u) has no value change at all.\n", 
		max_var_idcode);
    }
    else {
        //
        // Get the maximum time(xtag) where has value change. 
        //
        if (FSDB_RC_SUCCESS != 
	    vc_trvs_hdl->ffrGetMaxXTag((void*)&time)) {
	    fprintf(stderr, "should not happen.\n");
	    exit(FSDB_RC_FAILURE);
 	}
       	fprintf(stderr, "trvs hdl(%u): maximum time is (%u %u).\n", 
            	max_var_idcode, time.H, time.L);
            
        //
        // Get the minimum time(xtag) where has value change. 
        // 
        if (FSDB_RC_SUCCESS != 
	    vc_trvs_hdl->ffrGetMinXTag((void*)&time)) {
	    fprintf(stderr, "should not happen.\n");
	    exit(FSDB_RC_FAILURE);
 	}
       	fprintf(stderr, "trvs hdl(%u): minimum time is (%u %u).\n", 
            	max_var_idcode, time.H, time.L);
    
        //
        // Jump to the specific time specified by the parameter of 
	// ffrGotoXTag(). The specified time may have or have not 
	// value change; if it has value change, then the return time 
	// is exactly the same as the specified time; if it has not 
	// value change, then the return time will be aligned forward
	// (toward smaller time direction). 
        //
        // There is an exception for the jump alignment: If the 
	// specified time is smaller than the minimum time where has 
	// value changes, then the return time will be aligned to the 
	// minimum time.
        //
        if (FSDB_RC_SUCCESS != vc_trvs_hdl->ffrGotoXTag((void*)&time)) {
	    fprintf(stderr, "should not happen.\n");
	    exit(FSDB_RC_FAILURE);
        }	
    
        //
        // Get the value change. 
        //
        if (FSDB_RC_SUCCESS == vc_trvs_hdl->ffrGetVC(&vc_ptr))
            __PrintTimeValChng(vc_trvs_hdl, &time, vc_ptr);
         
    
        //
        // Value change traverse handle keeps an internal index
        // which points to the current time and value change; each
        // traverse API may move that internal index backward or
        // forward.
        // 
        // ffrGotoNextVC() moves the internal index backward so
        // that it points to the next value change and the time
        // where the next value change happened.
        //  
        for ( ; FSDB_RC_SUCCESS == vc_trvs_hdl->ffrGotoNextVC(); ) {
            vc_trvs_hdl->ffrGetXTag(&time);
      	    vc_trvs_hdl->ffrGetVC(&vc_ptr);
      	    __PrintTimeValChng(vc_trvs_hdl, &time, vc_ptr);
        }
            
        // 
        // ffrGotoPrevVC() moves the internal index forward so
        // that it points to the previous value change and the time
        // where the previous value change happened.
        //  
        for ( ; FSDB_RC_SUCCESS == vc_trvs_hdl->ffrGotoPrevVC(); ) {
            vc_trvs_hdl->ffrGetXTag(&time);
      	    vc_trvs_hdl->ffrGetVC(&vc_ptr);
      	    __PrintTimeValChng(vc_trvs_hdl, &time, vc_ptr);
        }
    }
    // 
    // free this value change traverse handle 
    //
    vc_trvs_hdl->ffrFree();

    //
    // We have traversed the value changes associated with the var 
    // whose idcode is max_var_idcode, now we are going to traverse
    // the value changes of the other vars. The idcode of the other
    // vars is from FSDB_MIN_VAR_IDCODE, which is 1, to 
    // (max_var_idcode - 1)  
    //
    for (i = FSDB_MIN_VAR_IDCODE; i < max_var_idcode; i++) {
   	//
        // create a value change traverse handle associated with
  	// current traversed var.
 	// 
 	vc_trvs_hdl = fsdb_obj->ffrCreateVCTraverseHandle(i);
        if (NULL == vc_trvs_hdl) {
	    fprintf(stderr, "Failed to create a traverse handle(%u)\n", 
		    max_var_idcode);
	    exit(FSDB_RC_OBJECT_CREATION_FAILED);
        }
  	fprintf(stderr, "\n");
	fprintf(stderr, "Current traversed var idcode is %u\n", i);

	//
	// Check to see if this var has value changes or not.
	//
	if (FALSE == vc_trvs_hdl->ffrHasIncoreVC()) {
	    fprintf(stderr, 
		"This var(%u) has no value change at all.\n", i);
	    vc_trvs_hdl->ffrFree();
	    continue;
	}

        //
        // Get the minimum time(xtag) where has value change. 
        // 
	vc_trvs_hdl->ffrGetMinXTag((void*)&time);

	//
	// Jump to the minimum time(xtag). 
	// 
	vc_trvs_hdl->ffrGotoXTag((void*)&time);

	//
	// Traverse all the value changes from the minimum time
	// to the maximum time.
	//
	do {
	    vc_trvs_hdl->ffrGetXTag(&time);
	    vc_trvs_hdl->ffrGetVC(&vc_ptr);
	    __PrintTimeValChng(vc_trvs_hdl, &time, vc_ptr);
	} while(FSDB_RC_SUCCESS == vc_trvs_hdl->ffrGotoNextVC());

        // 
        // free this value change traverse handle
        //
        vc_trvs_hdl->ffrFree();
    } 
    fprintf(stderr, "Watch Out Here!\n");
    fprintf(stderr, "We are going to reset the signal list.\n");
    fprintf(stderr, "Press enter to continue running.");
    getchar();

    fsdb_obj->ffrResetSignalList();
    for (i = FSDB_MIN_VAR_IDCODE; i <= max_var_idcode; i++) {
    	if (TRUE == fsdb_obj->ffrIsInSignalList(i)) 
	    fprintf(stderr, "var idcode %d is in signal list.\n", i);
	else
	    fprintf(stderr, "var idcode %d is not in signal list.\n", i);
    }
    fsdb_obj->ffrUnloadSignals();

*/

    fsdb_obj->ffrClose();
    return 0;
}
