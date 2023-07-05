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


#include "fsdb_reader.h"
#include <assert.h>

void
loadSignals(ffrObject* fsdb_obj, SIGNAL_MAP signal_map)
{
    //
    // In order to load value changes of vars onto memory, application
    // has to tell fsdb reader about what vars it's interested in. 
    // Application selects the interested vars by giving their idcodes
    // to fsdb reader, this is done by the following API:
    //
    //		ffrAddToSignalList()
    // 

    map<string,map<string,sigInfo>>::iterator t;
    map<string, sigInfo>::iterator signal_it;
    map<string, int> chi_map;

    int i = 0;

    fprintf(stderr, "马上进入map遍历.\n");
    // 遍历整个配置map项
    for(t = signal_map.begin(); t != signal_map.end(); t++)
    {
        //如果instance对应的map不为空，则为抓取到了对应信号的idcode
        if(t->second["info"].idcode == 20230706)
        {
            fprintf(stderr, "正在进行遍历的层次是: %s.\n", t->first.c_str());
            t->second.erase("info");
            for(signal_it = t->second.begin(); signal_it != t->second.end(); signal_it++)
            {
                // 判断是否有未找到idcode的signal
                assert(signal_it->second.idcode);

                fsdb_obj->ffrAddToSignalList(signal_it->second.idcode);
                fprintf(stderr, "add signal %s, idcode %u to list.\n", signal_it->first.c_str(), (unsigned int)signal_it->second.idcode);

                chi_map[signal_it->second.name] = signal_it->second.idcode;
                chi_sig_arr[i] = signal_it->second.idcode;
                i++;
            }
        }
    }

    //
    // Load the value changes of the selected vars onto memory. Note 
    // that value changes of unselected vars are not loaded.
    //
    fsdb_obj->ffrLoadSignals();

}

void
DumpData(ffrObject* fsdb_obj, FILE* dump_file)
{
    // 设置一个以time-base的value change抓取
    ffrTimeBasedVCTrvsHdl tb_vc_trvs_hdl;
    byte_T *vc_ptr;
    fsdbVarIdcode var_idcode;
    fsdbXTag time;
    fsdbSeqNum seq_num;

    tb_vc_trvs_hdl = fsdb_obj->ffrCreateTimeBasedVCTrvsHdl(CHI_SIG_NUM,chi_sig_arr);

    if (NULL == tb_vc_trvs_hdl) 
    {
        fprintf(stderr, "Fail to create time-based value change trvs hdl!\n");
    }

    if (FSDB_RC_SUCCESS == tb_vc_trvs_hdl->ffrGetVC(&vc_ptr)) {
        tb_vc_trvs_hdl->ffrGetVarIdcode(&var_idcode);
        tb_vc_trvs_hdl->ffrGetXTag(&time);
        tb_vc_trvs_hdl->ffrGetSeqNum(&seq_num);
        fprintf(stdout, "(%u %u) => var(%u): seq_num: %u val: ",
            time.hltag.H, time.hltag.L, (unsigned int)var_idcode, seq_num);
        PrintAsVerilog(vc_ptr, 1);
    }

    //
    // Iterate until no more value change
    //
    // The order of the value change at the same time step are
    // returned according to their sequence number. If sequence
    // number is not turned on, the order will be undetermined.
    //
    while(FSDB_RC_SUCCESS == tb_vc_trvs_hdl->ffrGotoNextVC()) {
        tb_vc_trvs_hdl->ffrGetVarIdcode(&var_idcode);
        tb_vc_trvs_hdl->ffrGetXTag(&time);
        tb_vc_trvs_hdl->ffrGetSeqNum(&seq_num);
        tb_vc_trvs_hdl->ffrGetVC(&vc_ptr);
        fprintf(stdout, "(%u %u) => var(%u): seq_num: %u val: ",
        time.hltag.H, time.hltag.L, (unsigned int)var_idcode, seq_num);
        PrintAsVerilog(vc_ptr, 1);
    }
    //
    // Remember to call ffrFree() to free the memory occupied by
    // this time-based value change trvs hdl
    //
    tb_vc_trvs_hdl->ffrFree();

    //
    // Remember to call ffrUnloadSignals() to unload value change
    // memory.
    //
    fsdb_obj->ffrUnloadSignals();

}

/*
** Print as Verilog standard values.
** x0z1-z10z 0x0x-1011...
*/
void
PrintAsVerilog(byte_T *ptr, uint_T size)
{
    const int VALUES_IN_A_SEG = 8;
    const int VALUES_DUMPED_IN_A_LINE = 40;
    int i, j, end_idx;
    byte_T a_byte;
    char val_tbl[] = "01xz";

    for(i = 0; i < size; i++) {
        a_byte = *(ptr+i);
        if (a_byte < 4)
            fprintf(stdout, "%c", val_tbl[a_byte]);
        else
            fprintf(stdout, "?");

        if (3 == (i % VALUES_IN_A_SEG) && (i < size-1))
            fprintf(stdout, "-");
        
        if ((VALUES_IN_A_SEG - 1) == (i % VALUES_IN_A_SEG)) {
            if ((VALUES_DUMPED_IN_A_LINE - 1) ==
                (i % VALUES_DUMPED_IN_A_LINE)) {
                fprintf(stdout, "\n");
            }
            else {
                fprintf(stdout, " ");
            }
        }
    }

    if ((VALUES_DUMPED_IN_A_LINE - 1) != (i %
        VALUES_DUMPED_IN_A_LINE)) {
        fprintf(stdout, "\n");
    }
}

static void 
__PrintTimeValChng(ffrVCTrvsHdl vc_trvs_hdl, 
		   fsdbTag64 *time, byte_T *vc_ptr)
{ 
    static byte_T buffer[FSDB_MAX_BIT_SIZE+1];
    byte_T        *ret_vc;
    uint_T        i;
    fsdbVarType   var_type; 
    
    switch (vc_trvs_hdl->ffrGetBytesPerBit()) {
    case FSDB_BYTES_PER_BIT_1B:

	//
 	// Convert each verilog bit type to corresponding
 	// character.
 	//
        for (i = 0; i < vc_trvs_hdl->ffrGetBitSize(); i++) {
	    switch(vc_ptr[i]) {
 	    case FSDB_BT_VCD_0:
	        buffer[i] = '0';
	        break;

	    case FSDB_BT_VCD_1:
	        buffer[i] = '1';
	        break;

	    case FSDB_BT_VCD_X:
	        buffer[i] = 'x';
	        break;

	    case FSDB_BT_VCD_Z:
	        buffer[i] = 'z';
	        break;

	    default:
		//
		// unknown verilog bit type found.
 		//
	        buffer[i] = 'u';
	    }
        }
        buffer[i] = '\0';
	fprintf(stderr, "time: (%u %u)  val chg: %s\n",
		time->H, time->L, buffer);	
	break;

    case FSDB_BYTES_PER_BIT_4B:
	//
	// Not 0, 1, x, z since their bytes per bit is
	// FSDB_BYTES_PER_BIT_1B. 
 	//
	// For verilog type fsdb, there is no array of 
  	// real/float/double so far, so we don't have to
	// care about that kind of case.
	//

        //
        // The var type of memory range variable is
        // FSDB_VT_VCD_MEMORY_DEPTH. This kind of var
        // has two value changes at certain time step.
        // The first value change is the index of the 
        // beginning memory variable which has a value change
        // and the second is the index of the end memory variable
        // which has a value change at this time step. The index
        // is stored as an unsigned integer and its bpb is 4B.
        //
        
        var_type = vc_trvs_hdl->ffrGetVarType();
        switch(var_type){
        case FSDB_VT_VCD_MEMORY_DEPTH:
        case FSDB_VT_VHDL_MEMORY_DEPTH:
	    fprintf(stderr, "time: (%u %u)", time->H, time->L);
            fprintf(stderr, "  begin: %d", *((int*)vc_ptr));
            vc_ptr = vc_ptr + sizeof(uint_T);
            fprintf(stderr, "  end: %d\n", *((int*)vc_ptr));  
            break;
               
        default:    
            vc_trvs_hdl->ffrGetVC(&vc_ptr);
	    fprintf(stderr, "time: (%u %u)  val chg: %f\n",
                    time->H, time->L, *((float*)vc_ptr));
	    break;
        }
        break;

    case FSDB_BYTES_PER_BIT_8B:
	//
	// Not 0, 1, x, z since their bytes per bit is
	// FSDB_BYTES_PER_BIT_1B. 
 	//
	// For verilog type fsdb, there is no array of 
  	// real/float/double so far, so we don't have to
	// care about that kind of case.
	//
	fprintf(stderr, "time: (%u %u)  val chg: %e\n",
		time->H, time->L, *((double*)vc_ptr));
	break;

    default:
	fprintf(stderr, "Control flow should not reach here.\n");
	break;
    }
}

char current_path[MAX_LINE];
char matched_path[MAX_LINE];

bool_T __MyTreeCB(fsdbTreeCBType cb_type, 
			 void *client_data, void *tree_cb_data)
{
    map<string,map<string,sigInfo>>::iterator t;
    SIGNAL_MAP* signal_map_local;
    signal_map_local = (SIGNAL_MAP*)client_data;

    switch (cb_type) {
    case FSDB_TREE_CBT_BEGIN_TREE:
	fprintf(stderr, "<BeginTree>\n");
	break;

    case FSDB_TREE_CBT_SCOPE:
	__DumpScope((fsdbTreeCBDataScope*)tree_cb_data, current_path);

    t = signal_map_local->find(current_path);
    if(t != signal_map_local->end())
    {
        strcpy(matched_path, current_path);
        printf("find matched path: %s\n", matched_path);
    }
    else
    {
        matched_path[0] = 0;
    }
	break;

    case FSDB_TREE_CBT_VAR:
	__DumpVar((fsdbTreeCBDataVar*)tree_cb_data, (SIGNAL_MAP*)client_data);
	break;

    case FSDB_TREE_CBT_UPSCOPE:
	fprintf(stderr, "<Upscope>\n");
    DelPath(current_path);
	break;

    case FSDB_TREE_CBT_END_TREE:
	fprintf(stderr, "<EndTree>\n\n");
	break;

    case FSDB_TREE_CBT_FILE_TYPE:
	break;

    case FSDB_TREE_CBT_SIMULATOR_VERSION:
	break;

    case FSDB_TREE_CBT_SIMULATION_DATE:
	break;

    case FSDB_TREE_CBT_X_AXIS_SCALE:
	break;

    case FSDB_TREE_CBT_END_ALL_TREE:
	break;

    case FSDB_TREE_CBT_ARRAY_BEGIN:
        fprintf(stderr, "<BeginArray>\n");
        break;
        
    case FSDB_TREE_CBT_ARRAY_END:
        fprintf(stderr, "<EndArray>\n\n");
        break;

    case FSDB_TREE_CBT_RECORD_BEGIN:
        fprintf(stderr, "<BeginRecord>\n");
        break;
        
    case FSDB_TREE_CBT_RECORD_END:
        fprintf(stderr, "<EndRecord>\n\n");
        break;
             
    default:
	return FALSE;
    }

    return TRUE;
}

static void 
__DumpScope(fsdbTreeCBDataScope* scope, char* match_path)
{
    str_T type;

    switch (scope->type) {
    case FSDB_ST_VCD_MODULE:
	type = (str_T) "module"; 
    AddPath(match_path, scope->name);
    fprintf(stderr, "<Debug> path:%s \n", match_path);
	break;

    case FSDB_ST_VCD_TASK:
	type = (str_T) "task"; 
	break;

    case FSDB_ST_VCD_FUNCTION:
	type = (str_T) "function"; 
	break;

    case FSDB_ST_VCD_BEGIN:
	type = (str_T) "begin"; 
	break;

    case FSDB_ST_VCD_FORK:
	type = (str_T) "fork"; 
	break;

    default:
	type = (str_T) "unknown_scope_type";
	break;
    }

    fprintf(stderr, "<Scope> name:%s  type:%s\n", 
	    scope->name, type);
}

static void 
__DumpVar(fsdbTreeCBDataVar* var, SIGNAL_MAP* signal_map)
{
    str_T type;
    str_T bpb;
    str_T direct;
    char* info = (char*)"info";

    map<string,sigInfo>::iterator t;
    SIGNAL_MAP signal_map_local = *signal_map;

    bool type_matched = FALSE;

    switch(var->bytes_per_bit) {
    case FSDB_BYTES_PER_BIT_1B:
	bpb = (str_T) "1B";
	break;

    case FSDB_BYTES_PER_BIT_2B:
	bpb = (str_T) "2B";
	break;

    case FSDB_BYTES_PER_BIT_4B:
	bpb = (str_T) "4B";
	break;

    case FSDB_BYTES_PER_BIT_8B:
	bpb = (str_T) "8B";
	break;

    default:
	bpb = (str_T) "XB";
	break;
    }

    switch (var->direction) {
    case FSDB_VD_INPUT:
    direct = (str_T) "input";
    type_matched = TRUE;
    break;

    case FSDB_VD_OUTPUT:
    direct = (str_T) "output";
    type_matched = TRUE;
    break;
    
    case FSDB_VD_INOUT:
    direct = (str_T) "inout";
    type_matched = TRUE;
    break;
    
    default:
    direct = (str_T) "unkown direction";
    break;

    }
    switch (var->type) {
    case FSDB_VT_VCD_EVENT:
	type = (str_T) "event"; 
  	break;

    case FSDB_VT_VCD_INTEGER:
	type = (str_T) "integer"; 
	break;

    case FSDB_VT_VCD_PARAMETER:
	type = (str_T) "parameter"; 
	break;

    case FSDB_VT_VCD_REAL:
	type = (str_T) "real"; 
	break;

    case FSDB_VT_VCD_REG:
	type = (str_T) "reg"; 
	break;

    case FSDB_VT_VCD_SUPPLY0:
	type = (str_T) "supply0"; 
	break;

    case FSDB_VT_VCD_SUPPLY1:
	type = (str_T) "supply1"; 
	break;

    case FSDB_VT_VCD_TIME:
	type = (str_T) "time";
	break;

    case FSDB_VT_VCD_TRI:
	type = (str_T) "tri";
	break;

    case FSDB_VT_VCD_TRIAND:
	type = (str_T) "triand";
	break;

    case FSDB_VT_VCD_TRIOR:
	type = (str_T) "trior";
	break;

    case FSDB_VT_VCD_TRIREG:
	type = (str_T) "trireg";
	break;

    case FSDB_VT_VCD_TRI0:
	type = (str_T) "tri0";
	break;

    case FSDB_VT_VCD_TRI1:
	type = (str_T) "tri1";
	break;

    case FSDB_VT_VCD_WAND:
	type = (str_T) "wand";
	break;

    case FSDB_VT_VCD_WIRE:
	type = (str_T) "wire";
	break;

    case FSDB_VT_VCD_WOR:
	type = (str_T) "wor";
	break;

    case FSDB_VT_VHDL_SIGNAL:
	type = (str_T) "signal";
	break;

    case FSDB_VT_VHDL_VARIABLE:
	type = (str_T) "variable";
	break;

    case FSDB_VT_VHDL_CONSTANT:
	type = (str_T) "constant";
	break;

    case FSDB_VT_VHDL_FILE:
	type = (str_T) "file";
	break;

    case FSDB_VT_VCD_MEMORY:
        type = (str_T) "vcd_memory";
        break;

    case FSDB_VT_VHDL_MEMORY:
        type = (str_T) "vhdl_memory";
        break;
        
    case FSDB_VT_VCD_MEMORY_DEPTH:
        type = (str_T) "vcd_memory_depth_or_range";
        break;
        
    case FSDB_VT_VHDL_MEMORY_DEPTH:         
        type = (str_T) "vhdl_memory_depth";
        break;

    default:
	type = (str_T) "unknown_var_type";
	break;
    }

    if(type_matched)
    {
        t = signal_map_local[matched_path].find(var->name);
        if(t != signal_map_local[matched_path].end())
        {
            signal_map_local[matched_path][info].idcode = 20230706;
            signal_map_local[matched_path][var->name].idcode = (int)var->u.idcode;
            //printf("找到匹配的信号:%s -> %u \n",
            //var->name, (unsigned int)var->u.idcode);
            printf("找到匹配的信号:%s -> %u \n",
            var->name, (unsigned int)signal_map_local[matched_path][var->name].idcode);
        }

        *signal_map = signal_map_local;

    }

    fprintf(stderr,
	"<Var>  name:%s  l:%u  r:%u  type:%s  ",
	var->name, var->lbitnum, var->rbitnum, type);
    fprintf(stderr,
    "direction:%u direct:%s ",
    var->direction,direct);
    fprintf(stderr,
	"idcode:%u  dtidcode:%u  bpb:%s\n",
	(unsigned int)var->u.idcode, (unsigned int)var->dtidcode, bpb);
}
