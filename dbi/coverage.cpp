/*
 * Copyright 2002-2019 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 * 
 * Modifications by Johannes Kinder, incorporating code from Axel Souchet
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;

typedef std::map<ADDRINT, UINT32> BASIC_BLOCKS_INFO_T;
BASIC_BLOCKS_INFO_T basic_blocks_info;

ofstream OutFile;

VOID handle_basic_block(UINT32 bb_insn_count, ADDRINT address_bb) {
    basic_blocks_info[address_bb] = bb_insn_count;
}
    
// Pin calls this function every time a new basic block is encountered
// It inserts a call to handle_basic_block
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount before every bbl, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)handle_basic_block, 
                       IARG_UINT32, BBL_NumIns(bbl), IARG_ADDRINT, BBL_Address(bbl), IARG_END);
    }
    
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "coverage.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr may be closed by the application
    OutFile.setf(ios::showbase);
    
    for(BASIC_BLOCKS_INFO_T::const_iterator it = basic_blocks_info.begin(); it != basic_blocks_info.end(); ++it) {
        OutFile << it->first << endl;
    }

    OutFile.close();

}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool records all basic blocks executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
