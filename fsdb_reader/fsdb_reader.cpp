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

    fprintf(stderr, "马上进入map遍历.\n");
    // 遍历整个配置map项
    for(t = signal_map.begin(); t != signal_map.end(); t++)
    {
        //如果instance对应的map不为空，则为抓取到了对应信号的idcode
        if(t->second["info"].idcode == 20230706)
        {
            if(strcmp(t->second["info"].name,(char*)"chi") == 0)
            {
                fprintf(stderr, "正在遍历的是CHI协议, 层次是: %s.\n", t->first.c_str());
                dumpCHIData(fsdb_obj, t->second);
            }
        }
    }

}

void
dumpCHIData(ffrObject* fsdb_obj, map<string,sigInfo> chi_sig_map)
{
    map<string, sigInfo>::iterator signal_it;
    map<int, sigInfo> chi_map;
    sigInfo sig_info;
    fsdbVarIdcode chi_sig_arr[CHI_SIG_NUM];
    int i = 0;


    chi_sig_map.erase("info");
    for(signal_it = chi_sig_map.begin(); signal_it != chi_sig_map.end(); signal_it++)
    {
        // 判断是否有未找到idcode的signal
        assert(signal_it->second.idcode);

        fsdb_obj->ffrAddToSignalList(signal_it->second.idcode);
        //fprintf(stderr, "add signal %s, idcode %u to list.\n", signal_it->first.c_str(), (unsigned int)signal_it->second.idcode);

        strcpy(sig_info.name,signal_it->second.name);
        sig_info.lbitnum = signal_it->second.lbitnum;

        chi_map[signal_it->second.idcode] = sig_info;
        chi_sig_arr[i] = signal_it->second.idcode;
        i++;
    }

    //
    // Load the value changes of the selected vars onto memory. Note 
    // that value changes of unselected vars are not loaded.
    //
    fsdb_obj->ffrLoadSignals();

    // 设置一个以time-base的value change抓取
    ffrTimeBasedVCTrvsHdl tb_vc_trvs_hdl;
    byte_T *vc_ptr;
    fsdbVarIdcode var_idcode;
    fsdbXTag time;
    fsdbSeqNum seq_num;
    CHIChannel channel = TXREQ;

	unsigned int time_dec[2];

    int txreq_len = 0;
    int txrsp_len = 0;
    int txdat_len = 0;
    int rxrsp_len = 0;
    int rxdat_len = 0;
    int rxsnp_len = 0;


    FILE *chi_dump_file = NULL;
    chi_dump_file = fopen("chi_analyzer.dat", "wb+");
    // 定义chi数据暂存空间
    chi_sig_t chi_sig_record = {.rstn = 0,
                                .txreqflitv = 0,
                                .txrspflitv = 0,
                                .txdatflitv = 0,
                                .rxrspflitv = 0,
                                .rxdatflitv = 0,
                                .rxsnpflitv = 0};

    tb_vc_trvs_hdl = fsdb_obj->ffrCreateTimeBasedVCTrvsHdl(CHI_SIG_NUM,chi_sig_arr);

    if (NULL == tb_vc_trvs_hdl) 
    {
        fprintf(stderr, "Fail to create time-based value change trvs hdl!\n");
    }

    //if (FSDB_RC_SUCCESS == tb_vc_trvs_hdl->ffrGetVC(&vc_ptr)) {
    //    tb_vc_trvs_hdl->ffrGetVarIdcode(&var_idcode);
    //    tb_vc_trvs_hdl->ffrGetXTag(&time);
    //    tb_vc_trvs_hdl->ffrGetSeqNum(&seq_num);

    //    writeCHIdat(chi_map, vc_ptr, time, var_idcode, &chi_sig_record);
    //}

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
        
		time_dec[1] = time.hltag.H;
		time_dec[0] = time.hltag.L;

        if(memcmp(chi_map[var_idcode].name, (char*)"clk", strlen("clk")) == 0) 
        {
            if(*vc_ptr == 0) 
            {
                fprintf(stdout, "@posedge clk; (%u %u) \n", time.hltag.H, time.hltag.L);

                if(chi_sig_record.txreqflitv)
                {
                    fprintf(stdout, "store found txreqflit : %x %x %x %x\n",
					                                         chi_sig_record.txreqflit[3],
					                                         chi_sig_record.txreqflit[2],
					                                         chi_sig_record.txreqflit[1],
					                                         chi_sig_record.txreqflit[0]
															 );


                    channel = TXREQ;
                    fwrite(&time_dec, 2 * sizeof(unsigned int), 1, chi_dump_file);
                    fwrite(&channel, sizeof(unsigned int), 1, chi_dump_file);
                    fwrite(&chi_sig_record.txreqflit, txreq_len * sizeof(byte_T), 1, chi_dump_file);
                }
            }
        }

        if(memcmp(chi_map[var_idcode].name, (char*)"rstn", strlen("rstn")) == 0)
        {
            //fprintf(stdout, "vc size: %ld\n", sizeof(*vc_ptr));
            chi_sig_record.rstn = *vc_ptr;
            fprintf(stdout, "(%u %u) => var(%u): val: ",
                time.hltag.H, time.hltag.L, (unsigned int)var_idcode);
            PrintAsVerilog(vc_ptr, 1);
        }


        if(memcmp(chi_map[var_idcode].name, (char*)"txreqflitpend", strlen("txreqflitpend")) == 0)
        {
            fprintf(stdout, "(%u %u) => var(%u): val: ",
                time.hltag.H, time.hltag.L, (unsigned int)var_idcode);
            PrintAsVerilog(vc_ptr, 1);
        }
        else if(memcmp(chi_map[var_idcode].name, (char*)"txreqflitv", strlen("txreqflitv")) == 0)
        {
            chi_sig_record.txreqflitv = *vc_ptr;
            fprintf(stdout, "(%u %u) => var(%u): val: ",
                time.hltag.H, time.hltag.L, (unsigned int)var_idcode);
            PrintAsVerilog(vc_ptr, 1);
        }
        else if(memcmp(chi_map[var_idcode].name, (char*)"txreqflit", strlen("txreqflit")) == 0)
        {
            fprintf(stdout, "(%u %u) => var(%u): val: \n",
                time.hltag.H, time.hltag.L, (unsigned int)var_idcode);
            PrintAsVerilogHex(vc_ptr, var_idcode, chi_map, chi_sig_record.txreqflit);

            if((chi_map[var_idcode].lbitnum+1) % 8 == 0)
                txreq_len = (chi_map[var_idcode].lbitnum+1)/8;
            else
                txreq_len = (chi_map[var_idcode].lbitnum+1)/8 + 1;
        }

        //if(strcmp(chi_map[var_idcode].name, (char*)"txrspflitv") == 0)
        //    chi_sig_record.txrspflitv = *vc_ptr;

        //if(strcmp(chi_map[var_idcode].name, (char*)"txrspflit") == 0)
        //{
        //    for(i=0; i<chi_map[var_idcode].lbitnum + 1; i++) {
        //        chi_sig_record.txrspflit[i] = *(vc_ptr+i);
        //    }
        //}

        //if(strcmp(chi_map[var_idcode].name, (char*)"txdatflitv") == 0)
        //    chi_sig_record.txdatflitv = *vc_ptr;

        //if(strcmp(chi_map[var_idcode].name, (char*)"txdatflit") == 0)
        //{
        //    for(i=0; i<chi_map[var_idcode].lbitnum + 1; i++) {
        //        chi_sig_record.txdatflit[i] = *(vc_ptr+i);
        //    }
        //}

        //if(strcmp(chi_map[var_idcode].name, (char*)"rxrsplitv") == 0)
        //    chi_sig_record.rxrspflitv = *vc_ptr;

        //if(strcmp(chi_map[var_idcode].name, (char*)"rxrspflit") == 0)
        //{
        //    for(i=0; i<chi_map[var_idcode].lbitnum + 1; i++) {
        //        chi_sig_record.rxrspflit[i] = *(vc_ptr+i);
        //    }
        //}

        //if(strcmp(chi_map[var_idcode].name ,(char*)"rxdatflitv") == 0)
        //    chi_sig_record.rxdatflitv = *vc_ptr;

        //if(strcmp(chi_map[var_idcode].name, (char*)"rxdatflit") == 0)
        //{
        //    for(i=0; i<chi_map[var_idcode].lbitnum + 1; i++) {
        //        chi_sig_record.rxdatflit[i] = *(vc_ptr+i);
        //    }
        //}

        //if(strcmp(chi_map[var_idcode].name, (char*)"rxsnpflitv") == 0)
        //    chi_sig_record.rxsnpflitv = *vc_ptr;

        //if(strcmp(chi_map[var_idcode].name, (char*)"rxsnpflit") == 0)
        //{
        //    for(i=0; i<chi_map[var_idcode].lbitnum + 1; i++) {
        //        chi_sig_record.rxsnpflit[i] = *(vc_ptr+i);
        //    }
        //}

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
** Print as Verilog standard values in hex format.
** x0z1-z10z 0x0x-1011...
*/
void
PrintAsVerilogHex(byte_T *ptr, fsdbVarIdcode var_idcode, map<int, sigInfo> chi_map, byte_T* flit)
{
    const int VALUES_IN_A_SEG = 8;
    const int VALUES_DUMPED_IN_A_LINE = 40;
    int i, j, end_idx;
    byte_T a_byte;
    char val_tbl[] = "01xz";
    byte_T hex_byte;
    byte_T hex_byte_h;

    uint_T size;
    size = chi_map[var_idcode].lbitnum + 1;

    int high_null_bit;
    if(size % 8)
        high_null_bit = 8 - size % 8;
    else
        high_null_bit = 0;

    end_idx = (size + high_null_bit)/8;

    for(i = 0; i < size + high_null_bit; i++) {
        //未对齐，在高位补0
        if(i < high_null_bit)
        {
            a_byte = 0;
        }
        else {
            a_byte = *(ptr + i - high_null_bit);
        }

        j = i % 4;        

        if (a_byte < 2)
            if(j == 0)
                hex_byte = a_byte << 3;
            else
            {
                hex_byte = hex_byte + (a_byte << (3 - j));
            }
        else {
            //fprintf(stderr, "found unkonw sigal ?\n");
            hex_byte = 0;
        }

        if (3 == (i % VALUES_IN_A_SEG) && (i < size+high_null_bit-1))
        {
            //fprintf(stdout, "%x", hex_byte);
            //fprintf(stdout, "-");
            hex_byte_h = hex_byte << 4;
        }
        
        if ((VALUES_IN_A_SEG - 1) == (i % VALUES_IN_A_SEG)) {
            fprintf(stdout, "%x", hex_byte+hex_byte_h);

            flit[end_idx - i/VALUES_IN_A_SEG - 1] = hex_byte + hex_byte_h;

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

/*
void
writeCHIdat(map<int, string> chi_map, byte_T* vc_ptr, fsdbXTag time, fsdbVarIdcode var_idcode, chi_sig_t* chi_sig_value)
{
    if(chi_map[var_idcode].compare("rstn") == 0)
    {
        if(*vc_ptr == 1)
        {
            fprintf(stdout, "vc size: %ld\n", sizeof(*vc_ptr));
            chi_sig_value->rstn = 1;
            fprintf(stdout, "(%u %u) => var(%u): val: ",
                time.hltag.H, time.hltag.L, (unsigned int)var_idcode);
            PrintAsVerilog(vc_ptr, 1);
        }
    }
    if(chi_sig_value->rstn) 
    {
        if(chi_sig_value->txreqflitv)
            fwrite(chi_sig_value->txreqflit, 6 * sizeof(long long), 1, );
    }
    //if(chi_map[var_idcode].compare("txreqflit") == 0)
    //{
    //    fprintf(stdout, "(%u %u) => var(%u): val: \n",
    //        time.hltag.H, time.hltag.L, (unsigned int)var_idcode);
    //    PrintAsVerilog(vc_ptr, 127);
    //}
    //else if(chi_map[var_idcode].compare("txreqflitv") == 0)
    //{
    //    fprintf(stdout, "(%u %u) => var(%u): val: ",
    //        time.hltag.H, time.hltag.L, (unsigned int)var_idcode);
    //    PrintAsVerilog(vc_ptr, 1);
    //}
}*/

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

    int high_null_bit;
    high_null_bit = 4 - size % 4;

    for(i = 0; i < size + high_null_bit; i++) {
        //未对齐，在高位补0
        if(i < high_null_bit)
        {
            a_byte = 0;
        }
        else {
            a_byte = *(ptr + i - high_null_bit);
        }
        //a_byte = *(ptr+i);
        if (a_byte < 4)
            fprintf(stdout, "%c", val_tbl[a_byte]);
        else
            fprintf(stdout, "?");

        if (3 == (i % VALUES_IN_A_SEG) && (i < size+high_null_bit-1))
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
        regex pattern("\[[0-9]+:[0-9]\\]");
        string pure_sig_name = regex_replace(var->name, pattern, "");

        t = signal_map_local[matched_path].find(pure_sig_name);
        if(t != signal_map_local[matched_path].end())
        {
            signal_map_local[matched_path][info].idcode = 20230706;
            signal_map_local[matched_path][pure_sig_name].idcode = (int)var->u.idcode;
            signal_map_local[matched_path][pure_sig_name].lbitnum= (int)var->lbitnum;

            printf("找到匹配的信号:%s -> %u \n",
            pure_sig_name.c_str(), (unsigned int)signal_map_local[matched_path][pure_sig_name].idcode);
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
