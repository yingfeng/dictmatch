#ifndef SDSL_CST_dt
#define SDSL_CST_dt

#include <sdsl/suffixtrees.hpp> // for CSTs
#include <sdsl/util.hpp>
#include <iostream>
#include <string>

using namespace sdsl;
using namespace std;

//can use one of the following statements to define data type for CST, or others that are compatible with the SDSL

//typedef  cst_sada<csa_sada<>, lcp_dac<> > tCST;
//typedef  cst_sada<csa_sada<>, lcp_support_tree2<> > tCST;
//typedef  cst_sada<csa_wt<wt_huff<> >, lcp_support_tree2<> > tCST;
typedef  cst_sada<csa_wt<wt_huff<> >, lcp_dac<> > tCST;
//typedef cst_sada<csa_uncompressed, lcp_uncompressed<> > tCST;


#endif

