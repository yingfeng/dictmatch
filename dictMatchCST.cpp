/* command to compile in Linux machines
command to compile this file in Linux:
g++ -O3 -DNDEBUG -funroll-loops -I${HOME}/include/ -L${HOME}/lib/ -o dictMatch dictMatchCST.cpp -lsdsl -ldivsufsort -ldivsufsort64
command to run this file in Linux:
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/users1/sneuburg/CSTmay/libdivsufsort-2.0.1/build/lib ./dictMatch pats.txt text.txt > outputOccs.txt
*/

#include <sdsl/suffixtrees.hpp>
#include <sdsl/util.hpp>
#include <sdsl/algorithms_for_string_matching.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "CST_dt.cpp"
#include "CST_LMA.cpp"

using namespace sdsl;
using namespace std;

long findPatInText(tCST &, string, int, bit_vector &, bit_vector &);
char getCharAtPos(const tCST &, const node_type &, int);
string loadTextFromFile(string);
void announcePatOcc(const tCST &st, const node_type &, int);



int main(int argc, char* argv[]){
    int beginPreTime;
    int endPreTime;
    int endScanTime;

	if( argc < 3 ){
		cout << "Usage " << argv[0] << " pat_file_name text_file_name" << endl;
		cout << "Usage " << argv[0] << " pat_file_name text_file_name" << endl;
		return 1;
	}
	tCST cst;
    bit_vector	 			bv_M;  // M indicates if a node is marked. it is of size 2n where n is the number of nodes in the suffix tree
    bit_vector              bv_D;  // D represents the structure of marked nodes  it is of size 2m where m is the number of marked nodes in the suffix tree.

    beginPreTime = std::time(0);

	construct_cst( argv[1], cst );
    markNodes(cst, bv_M, bv_D); //mark CST nodes for LMA queries

    endPreTime = std::time(0);

	// Output some information about the CST
	cout << "#dictionary consists of " << cst.csa.size() << " bytes" << endl;
	cout << "#cst occupies " << util::get_size_in_bytes(cst) << " bytes" << endl;
	//cout << "csa occupies " << util::get_size_in_bytes(cst.csa) << " bytes" << endl;
	//cout << "lcp occupies " << util::get_size_in_bytes(cst.lcp) << " bytes" << endl;
	cout << "# M occupies " << util::get_size_in_bytes(bv_M) << " bytes" << endl;
    cout << "# D occupies " << util::get_size_in_bytes(bv_D) << " bytes" << endl;

    string text = loadTextFromFile(argv[2]);
    cout<<"text has length "<<text.length()<<endl;

    unsigned char c;

    long numOccs = findPatInText(cst, text, text.length(), bv_M, bv_D);

    endScanTime = std::time(0);
    cout<<"# complete! at "<<endScanTime<<endl;
    cout<<" found "<<numOccs<<" pattern occurrences in text"<<endl;
    int preTime = endPreTime - beginPreTime;
    int scanTime = endScanTime - endPreTime;

    cout << " TotalSpace \t PreprocessingTime \t SearchTime \n" << (util::get_size_in_bytes(cst) + util::get_size_in_bytes(bv_M) + util::get_size_in_bytes(bv_D)) << " \t "<< preTime<< " \t "<< scanTime << endl;
}

    //returns length of edge labeling edge entering node
    int getNodeLabelLength(const tCST &st, const node_type &node)
    {
        return st.depth(node)-st.depth(st.parent(node));
    }


    //returns character on edge entering node, at d-th char along edge, 0-based
    char getCharAtPos(const tCST &st, const node_type &node, int d)
    {
        int cumD; //cumulative depth of d from root = string depth(node) + d
        cumD = st.depth(st.parent(node)) +d;
        return st.edge(node, cumD+1);   //add 1 since edge function uses 1-based indexing
    }


    long findPatInText(tCST &st, string text, int textLen, bit_vector &bv_M, bit_vector &bv_D){
        bool usedSkipCount = false;
		int skipcount = 0;
		int textIndex = 0; //text index
		int curNodeIndex = 0; //index within label of current node
		node_type curNode = st.root();
		node_type lastNode; //last node that was used; in case branching is unsuccessful and need to use its suffix links
        int textPos;

		int patocc;
        int curNodeLength;

        long numOccs=0;

		//begin at root and find first character(s) of text until there is a mismatch
		//loop until reach end of text
        do{
            lastNode = curNode;

            if(!usedSkipCount)  //if used skipcount, continue by comparing characters on current node that used suffix link to get to
            {
                textIndex+=curNodeIndex; //if not first time traversing
                curNodeIndex=0;

                curNode = st.child(curNode, text[textIndex]);
                curNodeLength = getNodeLabelLength(st, curNode);

                if(curNodeLength>0)
                {
                    curNodeIndex++; //already compared the first character on the edge, don't want to recompare
                }

            }
            else{
                usedSkipCount=false;  //reset flag
            }
            //compare text characters to labels of edges in suffix tree
            for(; curNodeIndex<curNodeLength && curNodeIndex+textIndex<textLen ; curNodeIndex++){
                if(text[textIndex+curNodeIndex]!=getCharAtPos(st, curNode, curNodeIndex)){
                    break; //for loop that compares characters
                }
            }

            //at end of edge and need to branch (not to traverse a suffix link bec of mismatch)
            //first: see if at a pattern occurrence to announce, by examining the char before this branch leaves the root
            if(curNodeIndex == curNodeLength && curNodeLength>0 && text[textIndex+curNodeIndex-1]==getCharAtPos(st, curNode, curNodeIndex-1))
            {
                continue; //don't try to find suffix link to follow since want to branch and continue comparing text to pattern chars
            }

            if(curNodeLength>0 && getCharAtPos(st, curNode, curNodeIndex) == END_OF_STRING_MARKER) // length from root is length of this pattern
            {

            //when have mismatch, see if at a pattern occ.  if yes, announce it. (END_OF_STRING_MARKER will never match text character)
                bool occ = false;
                int pos;

                pos = st.csa[st.lb(curNode)]-1;

                if (pos<0){
                    occ = true;
                }
                else{
                    unsigned char c = getCharAtPatternPos(st, pos);
                    if(c == END_OF_STRING_MARKER)
                    occ = true;
                }
                if (occ){
                    numOccs++;
                    announcePatOcc(st, curNode, textIndex);
                }
            }

            //at mismatch: use suffix link OR move on in text
            if (st.depth(curNode)!=0 || st.depth(lastNode)!=0)  //not at root (but could have tried to branch and failed)
            {
                //if couldn't find where to branch to, see if can follow suffix link of lastNode
                if ((st.id(st.sl(curNode))==st.id(st.root())) &&  (st.id(st.sl(lastNode))!=st.id(st.root())))
                {
                    curNode = lastNode;
                    curNodeLength = getNodeLabelLength(st, curNode);
                    curNodeIndex = curNodeLength;  //mismatched when trying to branch
                    textIndex -= curNodeLength;
                }

                textPos = curNodeIndex+textIndex; //position of character in text that is being compared now, shouldn't change when traverse suffix link
                //exception: can move on by 1 char when have nothing left on edge to cut off (mismatched at curNodeIndex = 1 and at root)

                curNodeLength = getNodeLabelLength(st,curNode);
                //following suffix link from child of root so losing 1 char at beginning
                if (st.id(st.parent(curNode))==st.id(st.root()) && curNodeIndex == 1){
                    //when traverse suffix link: will be at branch of mismatch.  skip 1 pattern char and continue looking for text at root
                    textIndex++;
                    curNodeIndex = 0;
                    curNode = st.parent(curNode); //not suffix link but root
                    curNodeLength = getNodeLabelLength(st, curNode);
                    continue;
                }

                node_type LMA = findLMA(st, curNode, bv_M, bv_D);

                if(st.id(LMA)!= st.id(st.root())){
                    int LMAdist = st.depth(st.parent(curNode))-st.depth(st.parent(LMA));
                    announcePatOcc(st, LMA, textIndex-(LMAdist));
                    numOccs++;
                }

                skipcount = curNodeLength - curNodeIndex;
                usedSkipCount=true;
                curNode = st.sl(curNode);
                curNodeLength=getNodeLabelLength(st, curNode);

                do{
                    if(skipcount >= curNodeLength){
                        if (curNodeLength == 0 ) //at root
                        {
                            usedSkipCount=false;  //not really using skip count and want to branch at beginning of next iteration of outer loop
                            //move on in text and look for next char at root
                            curNodeIndex=0;
                            skipcount = 0;
                        }
                        else{
                            if(skipcount==curNodeLength){
                                curNodeIndex--;
                                usedSkipCount=false;  //want to branch immediately
                            }
                            skipcount -= curNodeLength;
                            curNode = st.parent(curNode);
                            curNodeLength = getNodeLabelLength(st, curNode);
                        }
                    }
                    else{
                        curNodeIndex = curNodeLength - skipcount;
                        skipcount=0;
                    }
                }while(skipcount>0);

                textIndex = textPos - curNodeIndex;
            }
            else{
                //was mismatch and sl of curNode and of lastNode is root, node has length 1 and mismatch
                textIndex++;  //begin looking for next character at root
            }

        }while(textIndex+curNodeIndex<textLen);

    return numOccs;
}


string loadTextFromFile(string file_name)
{
    string text;
    ifstream textFile;
    textFile.open(file_name.data());
      getline(textFile, text, '\0'); //text files should not have \0
    textFile.close();
    text+=END_OF_STRING_MARKER2; //concatenate # to end of text so that don't miss pattern occurrence at end of text
    return text;
}

//announce pattern occurrence at position found
void announcePatOcc(const tCST &st, const node_type &curNode, const int textIndex)
{
    //occPos is using 1-based indexing in text so if begins at first char: will say "pattern occurrence! at 1"
    int occPos;
    int distanceFromRoot = 0;
    if (st.id(st.parent(curNode))!=st.id(st.root()))
        distanceFromRoot = st.depth(st.parent(curNode));

    occPos = textIndex-distanceFromRoot+1;
    //can annnounce which pattern was found - based on position in concatenated pattern string
    //cout<<"pattern occurrence at "<<occPos<<endl; //<<" tIdx="<<textIndex<<" distanceFromRoot = "<<distanceFromRoot<<endl<<endl<<endl;
    //removed so that should have less output
}

