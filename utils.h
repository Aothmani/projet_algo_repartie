#ifndef CHORD_CONSTANTS
#define CHORD_CONSTANTS

enum Tags {
	   TAGINIT, /* Sent by the simulator to define the variables */
	   TAGSEARCH, /* Searching a value in the hash table */
	   TAGFOUND, /* Sending back the result of the search */
	   TAGELECT, /* Choosing a leader for the election */
	   TAGANN, /* Annoucing the leader at the end of the election */
	   TAGTAB, /* Collecting all the node ids */
	   TAGTABANN, /* Broadcasting the completed table with all the node ids */
	   TAGTELLFINGER, /* Searching for a finger */
	   TAGTELLFINGERRESP, /* Response to TAGTELLFINGER from the new finger */
	   TAGUPDATE, /* Tell a node to update its fingers in regards to the newly inserted node */
	   TAGUPDATERESP, /* Response to TAGTELLFINGER, contains the new finger */
	   TAGINSERT, /* Insert a node in the ring */
	   TAGINSERTRESP /* Response of TAGINSERT, indicate that the node has been inserted in the ring */
};


void calcFinger(const int* tab, int n, int k, int* fingers);


#endif // CHORD_CONSTANTS
