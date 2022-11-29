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
#include <set>
#include "pin.H"
using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;

// Define a shorthand type for a set of addresses
typedef std::set<ADDRINT> COVERED_BLOCKS_T;

// Define a set of addresses as a global variable
COVERED_BLOCKS_T covered_blocks;

// Define an output file stream as a global variable
ofstream OutFile;

// This is the analysis routine actually instrumented into the target code
VOID handle_basic_block(ADDRINT address_bb) {
    covered_blocks.insert(address_bb);
}

// Instrumentation routine
// Pin calls this function every time a new basic block is encountered
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to handle_basic_block before every bbl, passing the address of the block
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)handle_basic_block, 
                       IARG_ADDRINT, BBL_Address(bbl), IARG_END);
    }
    
}

// Add another command line option to specify the output filename
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "coverage.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr may be closed by the application
    OutFile.setf(ios::showbase);
    
    // Dump the set to the file
    for(COVERED_BLOCKS_T::const_iterator it = covered_blocks.begin(); it != covered_blocks.end(); ++it) {
        OutFile << *it << endl;
    }

    // Close the output file stream
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
