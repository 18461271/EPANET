//
// test_project.cpp
//
// Date Created: January 24, 2018
//
// Author: Michael E. Tryby
//         US EPA - ORD/NRMRL
//

//#define BOOST_TEST_DYN_LINK

//#ifdef _WIN32
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#else
#include <stdlib.h>
//#endif


#define BOOST_TEST_MODULE "project"

#include "test_shared.hpp"


using namespace std;
using namespace boost;


BOOST_AUTO_TEST_SUITE (test_proj)

BOOST_AUTO_TEST_CASE (test_proj_create_delete)
{
    int error = 0;
    EN_Project ph = NULL;

    error = EN_createproject(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph != NULL);

    error = EN_deleteproject(&ph);

    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(ph == NULL);
}

BOOST_AUTO_TEST_CASE (test_proj_open_close)
{
	string path_inp(DATA_PATH_INP);
	string path_rpt(DATA_PATH_RPT);
    string path_out(DATA_PATH_OUT);

	EN_Project ph = NULL;
    EN_createproject(&ph);

    int error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_CASE(test_proj_savefile)
{
    int error;

    string path_inp(DATA_PATH_INP);
    string inp_save("test_reopen.inp");
	string path_rpt(DATA_PATH_RPT);
	string path_out(DATA_PATH_OUT);

	EN_Project ph_save;


	EN_createproject(&ph_save);
    error = EN_open(ph_save, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_saveinpfile(ph_save, inp_save.c_str());
    BOOST_REQUIRE(error == 0);

	BOOST_CHECK(filesystem::exists(inp_save) == true);

	error = EN_close(ph_save);
	BOOST_REQUIRE(error == 0);
    EN_deleteproject(&ph_save);
}

BOOST_AUTO_TEST_CASE(test_proj_reopen, * unit_test::depends_on("test_proj/test_proj_savefile"))
{
    int error;

    string inp_save("test_reopen.inp");
    string path_rpt(DATA_PATH_RPT);
	string path_out(DATA_PATH_OUT);

    EN_Project ph_reopen;


    EN_createproject(&ph_reopen);
	error = EN_open(ph_reopen, inp_save.c_str(), path_rpt.c_str(), path_out.c_str());
    BOOST_REQUIRE(error == 0);

    error = EN_close(ph_reopen);
    BOOST_REQUIRE(error == 0);
	EN_deleteproject(&ph_reopen);
}

BOOST_AUTO_TEST_CASE(test_proj_run)
{
    string path_inp(DATA_PATH_INP);
    string path_rpt(DATA_PATH_RPT);
    string path_out(DATA_PATH_OUT);

    EN_Project ph;

    EN_createproject(&ph);

    int error = EN_runproject(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str(), NULL);
    BOOST_REQUIRE(error == 0);

    EN_deleteproject(&ph);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(test_proj_fixture)

BOOST_FIXTURE_TEST_CASE(test_proj_title, FixtureOpenClose)
{
    // How is the API user supposed to know array size?
    char c_test[3][80], c_ref[3][80];

    strncpy(c_ref[0], " EPANET Example Network 1", 26);
    strncpy(c_ref[1], "A simple example of modeling chlorine decay. Both bulk and", 59);
    strncpy(c_ref[2], "wall reactions are included. ", 30);

    error = EN_gettitle(ph, c_test[0], c_test[1], c_test[2]);
    BOOST_REQUIRE(error == 0);

   for (int i = 0; i < 3; i++) {
       string test (c_test[i]);
       string ref (c_ref[i]);
       BOOST_CHECK(check_string(test, ref));
   }

   // Need a test for EN_settitle
}

BOOST_FIXTURE_TEST_CASE(test_proj_getcount, FixtureOpenClose)
{
    int i, array[7];

	std::vector<int> test;
	vector<int> ref = { 11, 2, 13, 1, 1, 2, 0 };

    for (i=EN_NODECOUNT; i<=EN_RULECOUNT; i++) {
        error = EN_getcount(ph, i, &array[i]);
        BOOST_REQUIRE(error == 0);
    }

    test.assign(array, array + 7);
    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

	error = EN_getcount(ph, 7, &i);
	BOOST_CHECK(error == 251);
}

BOOST_FIXTURE_TEST_CASE(test_setdemandpattern, FixtureOpenClose)
{
    int i, j, pat_index, pat_index_2, numDemands, nnodes;
	char newpat[] = "new_pattern";

	// get the number of nodes
    error = EN_getcount(ph, EN_NODECOUNT, &nnodes);
    BOOST_REQUIRE(error == 0);

	// add a new pattern
    error = EN_addpattern(ph, newpat);
	BOOST_REQUIRE(error == 0);

	// get the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
	BOOST_REQUIRE(error == 0);

	for (i = 1; i <= nnodes; i++) {
		// get the number of demand categories
		error = EN_getnumdemands(ph, i, &numDemands);
		BOOST_REQUIRE(error == 0);

		for (j = 1; j <= numDemands; j++) {
			// set demand patterns
			error = EN_setdemandpattern(ph, i, j, pat_index);
			BOOST_REQUIRE(error == 0);
			// get demand patterns should be the same with set
			error = EN_getdemandpattern(ph, i, j, &pat_index_2);
			BOOST_REQUIRE(error == 0);
			BOOST_CHECK(pat_index == pat_index_2);
		}
	}
}

BOOST_FIXTURE_TEST_CASE(test_addpattern, FixtureOpenClose)
{
    int pat_index, n_patterns_1, n_patterns_2;
    char newpat[] = "new_pattern";

    // get the number of current patterns
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_1);
    BOOST_REQUIRE(error == 0);

    // add a new pattern
    error = EN_addpattern(ph, newpat);
    BOOST_REQUIRE(error == 0);

    // get the new patterns count, shoul dbe the old one + 1
    error = EN_getcount(ph, EN_PATCOUNT, &n_patterns_2);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(n_patterns_1 + 1 == n_patterns_2);

    // gwt the new patterns index, should be as the number of patterns
    error = EN_getpatternindex(ph, newpat, &pat_index);
    BOOST_CHECK(pat_index == n_patterns_2);
}

BOOST_FIXTURE_TEST_CASE(test_add_control, FixtureOpenClose)
{
    int flag = 00;
    long t, tstep;
    double h1, h2;
    int Cindex;

    // run with original controls
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);
    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        error = EN_getnodevalue(ph, 11, EN_HEAD, &h1); // get the tank head
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    // disable current controls
    error = EN_setcontrol(ph, 1, 0, 0, 0, 0, 0);
    BOOST_REQUIRE(error == 0);
    error = EN_setcontrol(ph, 2, 1, 0, 0, 0, 0);
    BOOST_REQUIRE(error == 0);

    // add new controls
    error = EN_addcontrol(ph, 0, 13, 1, 11, 110, &Cindex);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(Cindex == 3);
    error = EN_addcontrol(ph, 1, 13, 0, 11, 140, &Cindex);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(Cindex == 4);

    // run with new controls
    error = EN_openH(ph);
    BOOST_REQUIRE(error == 0);
    error = EN_initH(ph, flag);
    BOOST_REQUIRE(error == 0);
    do {
        error = EN_runH(ph, &t);
        BOOST_REQUIRE(error == 0);
        error = EN_getnodevalue(ph, 11, EN_HEAD, &h2); // get the tank head
        BOOST_REQUIRE(error == 0);
        error = EN_nextH(ph, &tstep);
        BOOST_REQUIRE(error == 0);
    } while (tstep > 0);

    error = EN_closeH(ph);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK(abs(h1 - h2) < 1.e-5); // end head should be the same with new controls
}

BOOST_AUTO_TEST_SUITE_END()
