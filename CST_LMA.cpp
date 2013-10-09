#ifndef SDSL_CST_LMA
#define SDSL_CST_LMA

#include <sdsl/suffixtrees.hpp>
#include <sdsl/util.hpp>
#include <iostream>
#include <string>

using namespace sdsl;
using namespace std;

//these two charactesr should not appear in a pattern or in the text
const unsigned char END_OF_STRING_MARKER = '\n'; //to delimit patterns in dictionary file
const unsigned char END_OF_STRING_MARKER2 = '#'; //will be appended to text

typedef tCST::node_type node_type;
typedef tCST::size_type size_type;

typedef bp_support_sada<> bp_support_type;
//typedef bp_support_g<> bp_support_type;
typedef rank_support_v<10,2> rank_support_type;
typedef select_support_mcl<10,2> select_support_type;

node_type findLMA(const tCST &st, const node_type &node, const bit_vector &bv_M, const bit_vector &bv_D);

    //to obtain a character that is indexed by the compressed suffix array that is part of the compressed suffix tree
    template<class Csa>
    static unsigned char extract(const Csa &csa, typename Csa::size_type pos){
        assert( pos <= csa.size() );
        unsigned char c;


        for(typename Csa::size_type i=pos, order = csa(pos); i<=pos; ++i, order =  csa.psi[order]){
            uint16_t c_begin = 1, c_end = 257, mid;
            while( c_begin < c_end ){
                mid = (c_begin+c_end)>>1;
                if( csa.C[mid] <= order ){
                    c_begin = mid+1;
                }else{
                    c_end = mid;
                }
            }
            c = csa.comp2char[c_begin-1];
            return c;
        }
    }

    //retrieves d-th character of tCST's underlying input string, 0-based
    unsigned char getCharAtPatternPos(const tCST & st, int d)
    {
        return extract(st.csa, d);
    }

    //return true if this node has a child beginning with $ and $ is char before path leaves root (unless that char would be before string begins, i.e., this is the first pattern in the dictionary)
    bool nodeIsMarked(tCST &st, node_type node)
    {
        int pos = st.csa[st.lb(node)]-1;
        //last clause: don't mark root!
        if (st.id(st.child(node, END_OF_STRING_MARKER))!=st.id(st.root()) && (pos<0 ||  getCharAtPatternPos(st, pos) == END_OF_STRING_MARKER) && st.id(node)!=st.id(st.root()))
            return true;
        else
            return false;
    }

    //this function processes st for constant-time LMA queries
    void markNodes(tCST &st, bit_vector &bv_M, bit_vector &bv_D)
    {
        bv_M.resize(2*st.nodes());
        bv_D.resize(2*st.nodes());
        util::set_zero_bits(bv_M);

        size_type idx=0;
        size_type idx_d = 0;
        node_type node;

        //perform DFS on st
        //iterator has O(1) time and space complecity for all of its operations for the CSTs in the sdsl
        for (tCST::const_iterator it = st.begin(), end = st.end(); it != end; ++it){
            //visit() returns 1 for the first visit and 2 for the second visit
            //each internal node is visited twice
            //each leaf is visited once
            node = *it;

            if (nodeIsMarked(st, node)) //regardless of whether this is the first or second visit to the node
            {
                bv_M[idx] = 1;
                //add to bv_D
                if (it.visit()==1){ //open parentheses
                    bv_D[idx_d] = 1;
                }
                else{ //close parentheses
                    bv_D[idx_d] = 0;
                }
                idx_d++;
            }

            if( st.is_leaf(*it) )
            {
                ++idx; //leave its closing parentheses as a 0 in the bit array
                bv_M[idx] = bv_M[idx-1];  //mark its closing bit in the M bit array if its opening bit is marked
            }
            ++idx;
        }
        bv_M.resize(idx);
        bv_D.resize(idx_d);

}

   node_type findLMA(const tCST &st, const node_type &node, const bit_vector &bv_M, const bit_vector &bv_D)
   {
		bp_support_type			bp_support_D(&bv_D); // support for the balanced parentheses sequence D
		rank_support_type		rank_M;  // rank_support for marked nodes
		select_support_type		select_M;// select_support for marked nodes
		node_type LMA;

        //let x be the index in M of the node we are interested in
        //If M[x]=1 we are done, since then x is marked itself.
        if (bv_M[node]==1){
            LMA = node;
        }
        else{
            //find y such that M[y]=1 and y is the maximal y < x
            rank_M.init(&bv_M);
            select_M.init(&bv_M);
            size_type preY = rank_M.rank(node+1);

            if (preY ==0 )
                LMA = 0;
            else{
                size_type y = select_M.select(preY)-1; //subtract 1 to compensate for 0-based array

                if(st.bp[y]==1){
                    LMA = y;
                }
                else{ //B[y]=')'
                    size_type y1 = rank_M.rank(y);//coresponding index in D  //subtract 1 to compensate for 0-based array
                    size_type y2 = bp_support_D.find_open(y1);
                    size_type y3 = bp_support_D.enclose(y2); //returns size() if there is no enclosing parentheses
                    size_type y4;
                    if(y3 == bv_D.size()) //no enclosing parentheses
                    {
                        y3 = 0;
                        y4 = 0; //root
                    }
                    else
                        y4 = select_M.select(y3+1)-1;
                    //if no LMA?  return root, node 0
                    LMA = y4;
                }
            }
        }
        return LMA;
   }
#endif

