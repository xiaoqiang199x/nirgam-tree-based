
/*
 * NoC.cpp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Lavina Jain
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file NoC.cpp
/// \brief Creates network topology
///sc_in<creditLine>	credit_in[num_nb][NUM_VCS];	///< input ports for credit line (buffer status)
//sc_out<creditLine>	credit_out[num_nb][NUM_VCS];	///< output ports for credit line (buffer status)

/// This file creates a 2-dimensional topology of network tiles and interconnects them.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "NoC.h"
#include <string>
#include <fstream>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor to generate topology.
/// It creates 2-d toroidal or non- toroidal mesh of m x n network tiles depending on input from user.
/// Parameters:
/// - number of rows in topology
/// - number of columns in topology
/// .
/// The constructor implements the following:
/// - interconnect network tiles
/// - assign tile IDs
/// - connect clock signal to clock input port of tiles.
////////////////////////////////////////////////////////////////////////////////////////////////////
NoC::NoC(sc_module_name NoC, int num_rows, int num_cols, vector<vector<UI> > tree_id): sc_module(NoC)
{
    rows = num_rows;
    cols = num_cols;

//////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN//////////////////////////
    int flag=0,flagE=0,ctr=0,source=0,sourceE=0;
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN//////////////////////

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            char tileName[19];
            sprintf(tileName, "nwtile[%d][%d]",i,j);
            UI id = j + i * cols;

            cout<<"Creating tile "<<id<<endl;

            switch (TOPO)
            {

            case TORUS:
                nwtile[i][j] = new NWTile<NUM_NB, NUM_IC, NUM_OC>(tileName, id, tree_id);	// create a tile
                (ptr nwtile[i][j])->switch_cntrl(*nw_clock);		// connect clock
                if (RT_ALGO == QRT)
                {
                    (ptr nwtile[i][j])->ctr.rtable->num_tiles = num_tiles;
                    (ptr nwtile[i][j])->ctr.rtable->initialize();
                }
                break;


            case MESH:
                if (corner(id))
                {
                    //	cout << "NUM_IC_C :"<< NUM_IC_C << "ID :"<<id << endl;
                    nwtile[i][j] = new NWTile<NUM_NB_C, NUM_IC_C, NUM_OC_C>(tileName, id, tree_id);
                    (ptr_c nwtile[i][j])->switch_cntrl(*nw_clock);
                    //qrt*************************************************************************************************
                    if (RT_ALGO == QRT)
                    {
                        (ptr_c nwtile[i][j])->ctr.rtable->num_tiles = num_tiles;
                        (ptr_c nwtile[i][j])->ctr.rtable->initialize();
                        //qrt*************************************************************************************************
                    }
                }
                else if (border(id))
                {
                    //	cout << "NUM_IC_B :"<< NUM_IC_B << "ID :"<<id << endl;
                    nwtile[i][j] = new NWTile<NUM_NB_B, NUM_IC_B, NUM_OC_B>(tileName, id, tree_id);
                    (ptr_b nwtile[i][j])->switch_cntrl(*nw_clock);
                    //qrt*************************************************************************************************
                    if (RT_ALGO == QRT)
                    {
                        (ptr_b nwtile[i][j])->ctr.rtable->num_tiles = num_tiles;
                        (ptr_b nwtile[i][j])->ctr.rtable->initialize();
                    }
                    //qrt*************************************************************************************************
                }
                else
                {
                    //	cout << "NUM_IC :"<< NUM_IC << "ID :"<<id << endl;
                    nwtile[i][j] = new NWTile<NUM_NB, NUM_IC, NUM_OC>(tileName, id, tree_id);
                    (ptr nwtile[i][j])->switch_cntrl(*nw_clock);
                    //qrt*************************************************************************************************
                    if (RT_ALGO == QRT)
                    {
                        (ptr nwtile[i][j])->ctr.rtable->num_tiles = num_tiles;
                        (ptr nwtile[i][j])->ctr.rtable->initialize();
                    }
                    //qrt*************************************************************************************************
                }
                break;
            }
        }
    }

    //cout<<"\n FAILNO: "<<failno;

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////
    for (int m=0; m<failno; m++)
    {
        //cout<<"\n HERE "<<m;
        temp1[m]=new temp;
        //cout<<"\nNOC.CPP"<<m;
    }
    for (int m=failno; m<40; m++)
        temp1[m]=NULL;
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////


    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {

            UI id = j + i * cols; //tile id of tile(i,j)
            UI id_S = j + ((i+1) % rows) * cols; //south neighbor of tile(i,j)
            UI id_E = (j+1) % cols + i * cols; //east neighbor of tile(i,j)
            //qrt*************************************************************************************************
            //Q-Router
            UI num_tiles = rows*cols;
            //qrt*************************************************************************************************
            switch (TOPO)
            {

            case TORUS:
                //Q-Router


                //connect data line to South neihbor
                (ptr nwtile[i][j])->op_port[1](sigs[i][j].sig_toS);
                (ptr nwtile[(i+1)%rows][j])->ip_port[0](sigs[i][j].sig_toS);

                //qrt*************************************************************************************************
                ///////////EST_LINE////////////
                (ptr nwtile[i][j])->estOut_port[1](sigs[i][j].est_sig_toS);
                (ptr nwtile[(i+1)%rows][j])->estIn_port[0](sigs[i][j].est_sig_toS);
                //qrt*************************************************************************************************

                //connect credit line to South neihbor
                for (int k = 0; k < NUM_VCS; k++)
                {
                    (ptr nwtile[(i+1)%rows][j])->credit_in[0][k](sigs[i][j].cr_sig_toS[k]);
                    (ptr nwtile[i][j])->credit_out[1][k](sigs[i][j].cr_sig_toS[k]);
                }
                /*added*/
                (ptr nwtile[i][j])->p_credit_out[1](sigs[i][j].power_sig_toS);
                (ptr nwtile[(i+1)%rows][j])->p_credit_in[0](sigs[i][j].power_sig_toS);
                /*end*/

                // connect data line from South neighbor
                (ptr nwtile[i][j])->ip_port[1](sigs[i][j].sig_fromS);
                (ptr nwtile[(i+1)%rows][j])->op_port[0](sigs[i][j].sig_fromS);

                //qrt*************************************************************************************************
                ////////////EST_LINE////////////
                (ptr nwtile[i][j])->estOut_port[1](sigs[i][j].est_sig_fromS);
                (ptr nwtile[(i+1)%rows][j])->estIn_port[0](sigs[i][j].est_sig_fromS);
                //qrt*************************************************************************************************

                // connect credit line from South neighbor
                for (int k = 0; k < NUM_VCS; k++)
                {
                    (ptr nwtile[(i+1)%rows][j])->credit_out[0][k](sigs[i][j].cr_sig_fromS[k]);
                    (ptr nwtile[i][j])->credit_in[1][k](sigs[i][j].cr_sig_fromS[k]);
                }
                /*added*/
                (ptr nwtile[i][j])->p_credit_in[1](sigs[i][j].power_sig_fromS);
                (ptr nwtile[(i+1)%rows][j])->p_credit_out[0](sigs[i][j].power_sig_fromS);
                /*end*/

                // connect data line from East neighbor
                (ptr nwtile[i][j])->ip_port[2](sigs[i][j].sig_fromE);
                (ptr nwtile[i][(j+1)%cols])->op_port[3](sigs[i][j].sig_fromE);

                //qrt*************************************************************************************************
                ///////////////EST_LINE///////////////
                (ptr nwtile[i][j])->estOut_port[2](sigs[i][j].est_sig_fromE);
                (ptr nwtile[i][(j+1)%cols])->estIn_port[3](sigs[i][j].est_sig_fromE);
                //qrt*************************************************************************************************

                // connect credit line from East neighbor
                for (int k = 0; k < NUM_VCS; k++)
                {
                    (ptr nwtile[i][(j+1)%cols])->credit_out[3][k](sigs[i][j].cr_sig_fromE[k]);
                    (ptr nwtile[i][j])->credit_in[2][k](sigs[i][j].cr_sig_fromE[k]);
                }

                /*added*/
                (ptr nwtile[i][j])->p_credit_in[2](sigs[i][j].power_sig_fromE);
                (ptr nwtile[i][(j+1)%cols])->p_credit_out[3](sigs[i][j].power_sig_fromE);
                /*end*/
                //connect data line to East neighbor
                (ptr nwtile[i][j])->op_port[2](sigs[i][j].sig_toE);
                (ptr nwtile[i][(j+1)%cols])->ip_port[3](sigs[i][j].sig_toE);

                //qrt*************************************************************************************************
                ///////////////EST_LINE///////////////
                (ptr nwtile[i][j])->estOut_port[2](sigs[i][j].est_sig_toE);
                (ptr nwtile[i][(j+1)%cols])->estIn_port[3](sigs[i][j].est_sig_toE);
                //qrt*************************************************************************************************

                //connect credit line to East neighbor
                for (int k = 0; k < NUM_VCS; k++)
                {
                    (ptr nwtile[i][(j+1)%cols])->credit_in[3][k](sigs[i][j].cr_sig_toE[k]);
                    (ptr nwtile[i][j])->credit_out[2][k](sigs[i][j].cr_sig_toE[k]);
                }
                /*added*/
                (ptr nwtile[i][j])->p_credit_out[2](sigs[i][j].power_sig_toE);
                (ptr nwtile[i][(j+1)%cols])->p_credit_in[3](sigs[i][j].power_sig_toE);
                /*end*/
                break;


            case MESH:
                if (!borderS(nwtile[i][j]->tileID))
                {

                    // connect data and credit line to South neihbor
                    if (corner(id))  	// corner tile
                    {
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////
                        for (int m=0; m<failno; m++)
                        {
                            if (id==fail[m][0])
                            {
                                if (id_S==fail[m][1])
                                {
                                    flag=1;
                                    source=m;
                                    break;
                                }
                            }
                        }
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////

                        (ptr_c nwtile[i][j])->op_port[nwtile[i][j]->portS](sigs[i][j].sig_toS);
                        (ptr_c nwtile[i][j])->ip_port[nwtile[i][j]->portS](sigs[i][j].sig_fromS);

                        //qrt*************************************************************************************************
                        ////////////EST_LINE//////////
                        (ptr_c nwtile[i][j])->estOut_port[nwtile[i][j]->portS](sigs[i][j].est_sig_toS);
                        (ptr_c nwtile[i][j])->estIn_port[nwtile[i][j]->portS](sigs[i][j].est_sig_fromS);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_c nwtile[i][j])->credit_out[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_toS[k]);
                            (ptr_c nwtile[i][j])->credit_in[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_fromS[k]);
                        }
                        /*added*/
                        (ptr_c nwtile[i][j])->p_credit_out[nwtile[i][j]->portS](sigs[i][j].power_sig_toS);
                        (ptr_c nwtile[i][j])->p_credit_in[nwtile[i][j]->portS](sigs[i][j].power_sig_fromS);
                        /*end*/
                    }
                    else if (border(id))  	// border tile
                    {

/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////
                        for (int m=0; m<failno; m++)
                        {
                            if (id==fail[m][0])
                            {
                                if (id_S==fail[m][1])
                                {
                                    flag=1;
                                    source=m;
                                    break;
                                }
                            }
                        }
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////

                        (ptr_b nwtile[i][j])->op_port[nwtile[i][j]->portS](sigs[i][j].sig_toS);
                        (ptr_b nwtile[i][j])->ip_port[nwtile[i][j]->portS](sigs[i][j].sig_fromS);

                        //qrt*************************************************************************************************
                        //////////////EST_LINE///////////
                        (ptr_b nwtile[i][j])->estOut_port[nwtile[i][j]->portS](sigs[i][j].est_sig_toS);
                        (ptr_b nwtile[i][j])->estIn_port[nwtile[i][j]->portS](sigs[i][j].est_sig_fromS);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_b nwtile[i][j])->credit_out[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_toS[k]);
                            (ptr_b nwtile[i][j])->credit_in[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_fromS[k]);
                        }
                        /*added*/
                        (ptr_b nwtile[i][j])->p_credit_out[nwtile[i][j]->portS](sigs[i][j].power_sig_toS);
                        (ptr_b nwtile[i][j])->p_credit_in[nwtile[i][j]->portS](sigs[i][j].power_sig_fromS);
                        /*end*/
                    }
                    else  		// generic tile
                    {

/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////
                        for (int m=0; m<failno; m++)
                        {
                            if (id==fail[m][0])
                            {
                                if (id_S==fail[m][1])
                                {
                                    flag=1;
                                    source=m;
                                    break;
                                }
                            }
                        }
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////


                        (ptr nwtile[i][j])->op_port[nwtile[i][j]->portS](sigs[i][j].sig_toS);
                        (ptr nwtile[i][j])->ip_port[nwtile[i][j]->portS](sigs[i][j].sig_fromS);

                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr nwtile[i][j])->estOut_port[nwtile[i][j]->portS](sigs[i][j].est_sig_toS);
                        (ptr nwtile[i][j])->estIn_port[nwtile[i][j]->portS](sigs[i][j].est_sig_fromS);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr nwtile[i][j])->credit_out[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_toS[k]);
                            (ptr nwtile[i][j])->credit_in[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_fromS[k]);
                        }
                        /*added*/
                        (ptr nwtile[i][j])->p_credit_out[nwtile[i][j]->portS](sigs[i][j].power_sig_toS);
                        (ptr nwtile[i][j])->p_credit_in[nwtile[i][j]->portS](sigs[i][j].power_sig_fromS);
                        /*end*/
                    }

                    // connect data and credit line from South neighbor
                    if (corner(id_S))  	// South neighbor is corner tile
                    {

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN///////////////////////
                        if (id_S==fail[source][1] && flag==1)
                        {
                            flag=0;
                            (temp1[ctr])->ip_port[0](sigs[i][j].sig_toS);
                            (temp1[ctr])->op_port[0](sigs[i][j].sig_fromS);

                            (ptr_c nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](temp_sigs[ctr].tempsig_toS);
                            (ptr_c nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](temp_sigs[ctr].tempsig_fromS);


                            (temp1[ctr])->op_port[1](temp_sigs[ctr].tempsig_toS);
                            (temp1[ctr])->ip_port[1](temp_sigs[ctr].tempsig_fromS);

                            ctr++;

                        }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////
                        else
                        {

                            (ptr_c nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_toS);
                            (ptr_c nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_fromS);

                        }
                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr_c nwtile[(i+1)%rows][j])->estIn_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].est_sig_toS);
                        (ptr_c nwtile[(i+1)%rows][j])->estOut_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].est_sig_fromS);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_c nwtile[(i+1)%rows][j])->credit_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_toS[k]);
                            (ptr_c nwtile[(i+1)%rows][j])->credit_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_fromS[k]);
                        }
                        /*added*/
                        (ptr_c nwtile[(i+1)%rows][j])->p_credit_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].power_sig_toS);
                        (ptr_c nwtile[(i+1)%rows][j])->p_credit_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].power_sig_fromS);
                        /*end*/
                    }
                    else if (border(id_S))  	// South neigbor is border tile
                    {

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN///////////////////////
                        if (id_S==fail[source][1] && flag==1)
                        {
                            flag=0;
                            (temp1[ctr])->ip_port[0](sigs[i][j].sig_toS);
                            (temp1[ctr])->op_port[0](sigs[i][j].sig_fromS);

                            (ptr_b nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](temp_sigs[ctr].tempsig_toS);
                            (ptr_b nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](temp_sigs[ctr].tempsig_fromS);


                            (temp1[ctr])->op_port[1](temp_sigs[ctr].tempsig_toS);
                            (temp1[ctr])->ip_port[1](temp_sigs[ctr].tempsig_fromS);

                            ctr++;

                        }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////
                        else
                        {

                            (ptr_b nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_toS);
                            (ptr_b nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_fromS);

                        }
                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr_b nwtile[(i+1)%rows][j])->estIn_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].est_sig_toS);
                        (ptr_b nwtile[(i+1)%rows][j])->estOut_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].est_sig_fromS);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_b nwtile[(i+1)%rows][j])->credit_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_toS[k]);
                            (ptr_b nwtile[(i+1)%rows][j])->credit_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_fromS[k]);
                        }
                        /*added*/
                        (ptr_b nwtile[(i+1)%rows][j])->p_credit_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].power_sig_toS);
                        (ptr_b nwtile[(i+1)%rows][j])->p_credit_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].power_sig_fromS);
                        /*end*/
                    }
                    else  	// South neigbor is generic tile
                    {

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN///////////////////////
                        if (id_S==fail[source][1] && flag==1)
                        {
                            flag=0;
                            (temp1[ctr])->ip_port[0](sigs[i][j].sig_toS);
                            (temp1[ctr])->op_port[0](sigs[i][j].sig_fromS);

                            (ptr nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](temp_sigs[ctr].tempsig_toS);
                            (ptr nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](temp_sigs[ctr].tempsig_fromS);


                            (temp1[ctr])->op_port[1](temp_sigs[ctr].tempsig_toS);
                            (temp1[ctr])->ip_port[1](temp_sigs[ctr].tempsig_fromS);

                            ctr++;

                        }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////
                        else
                        {
                            (ptr nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_toS);
                            (ptr nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_fromS);

                        }

                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr nwtile[(i+1)%rows][j])->estIn_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].est_sig_toS);
                        (ptr nwtile[(i+1)%rows][j])->estOut_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].est_sig_fromS);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr nwtile[(i+1)%rows][j])->credit_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_toS[k]);
                            (ptr nwtile[(i+1)%rows][j])->credit_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_fromS[k]);
                        }
                        /*added*/
                        (ptr nwtile[(i+1)%rows][j])->p_credit_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].power_sig_toS);
                        (ptr nwtile[(i+1)%rows][j])->p_credit_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].power_sig_fromS);
                        /*end*/
                    }
                }

                if (!borderE(nwtile[i][j]->tileID))  	// connect to East neighbor only if current tile is not on East border
                {

                    // connect data and credit line from East neighbor
                    if (corner(id))  	// corner tile
                    {

/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////
                        for (int m=0; m<failno; m++)
                        {
                            if (id==fail[m][0])
                            {
                                if (id_E==fail[m][1])
                                {
                                    flagE=1;
                                    sourceE=m;
                                    //cout<<"\n ASSIGNED 3 ";
                                    break;
                                }

                            }
                        }
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////


                        (ptr_c nwtile[i][j])->ip_port[nwtile[i][j]->portE](sigs[i][j].sig_fromE);
                        (ptr_c nwtile[i][j])->op_port[nwtile[i][j]->portE](sigs[i][j].sig_toE);

                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr_c nwtile[i][j])->estIn_port[nwtile[i][j]->portE](sigs[i][j].est_sig_fromE);
                        (ptr_c nwtile[i][j])->estOut_port[nwtile[i][j]->portE](sigs[i][j].est_sig_toE);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_c nwtile[i][j])->credit_in[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_fromE[k]);
                            (ptr_c nwtile[i][j])->credit_out[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_toE[k]);
                        }
                        (ptr_c nwtile[i][j])->p_credit_in[nwtile[i][j]->portE](sigs[i][j].power_sig_fromE);
                        (ptr_c nwtile[i][j])->p_credit_out[nwtile[i][j]->portE](sigs[i][j].power_sig_toE);
                    }
                    else if (border(id))  	// border tile
                    {

/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////
                        for (int m=0; m<failno; m++)
                        {
                            if (id==fail[m][0])
                            {
                                if (id_E==fail[m][1])
                                {
                                    flagE=1;
                                    sourceE=m;
                                    //cout<<"\n ASSIGNED 2 ";
                                    break;
                                }
                            }
                        }
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////


                        (ptr_b nwtile[i][j])->ip_port[nwtile[i][j]->portE](sigs[i][j].sig_fromE);
                        (ptr_b nwtile[i][j])->op_port[nwtile[i][j]->portE](sigs[i][j].sig_toE);

                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr_b nwtile[i][j])->estIn_port[nwtile[i][j]->portE](sigs[i][j].est_sig_fromE);
                        (ptr_b nwtile[i][j])->estOut_port[nwtile[i][j]->portE](sigs[i][j].est_sig_toE);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_b nwtile[i][j])->credit_in[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_fromE[k]);
                            (ptr_b nwtile[i][j])->credit_out[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_toE[k]);
                        }
                        /*added*/
                        (ptr_b nwtile[i][j])->p_credit_in[nwtile[i][j]->portE](sigs[i][j].power_sig_fromE);
                        (ptr_b nwtile[i][j])->p_credit_out[nwtile[i][j]->portE](sigs[i][j].power_sig_toE);
                        /*end*/
                    }
                    else  	// generic tile
                    {

/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////
                        for (int m=0; m<failno; m++)
                        {
                            if (id==fail[m][0])
                            {
                                if (id_E==fail[m][1])
                                {
                                    flagE=1;
                                    sourceE=m;
                                    //cout<<"\n ASSIGNED 1 ";
                                    break;
                                }
                            }
                        }
/////NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN////////////////////////////////////////////////


                        (ptr nwtile[i][j])->ip_port[nwtile[i][j]->portE](sigs[i][j].sig_fromE);
                        (ptr nwtile[i][j])->op_port[nwtile[i][j]->portE](sigs[i][j].sig_toE);

                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr nwtile[i][j])->estIn_port[nwtile[i][j]->portE](sigs[i][j].est_sig_fromE);
                        (ptr nwtile[i][j])->estOut_port[nwtile[i][j]->portE](sigs[i][j].est_sig_toE);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr nwtile[i][j])->credit_in[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_fromE[k]);
                            (ptr nwtile[i][j])->credit_out[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_toE[k]);
                        }
                        /*added*/
                        (ptr nwtile[i][j])->p_credit_in[nwtile[i][j]->portE](sigs[i][j].power_sig_fromE);
                        (ptr nwtile[i][j])->p_credit_out[nwtile[i][j]->portE](sigs[i][j].power_sig_toE);
                    }

                    // connect data and credit line to East neighbor
                    if (corner(id_E))  	// East neighbor is corner tile
                    {

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN///////////////////////
                        if (id_E==fail[sourceE][1] && flagE==1)
                        {
                            flagE=0;
                            (temp1[ctr])->ip_port[0](sigs[i][j].sig_toE);
                            (temp1[ctr])->op_port[0](sigs[i][j].sig_fromE);

                            (ptr_c nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](temp_sigs[ctr].tempsig_toE);
                            (ptr_c nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](temp_sigs[ctr].tempsig_fromE);


                            (temp1[ctr])->ip_port[1](temp_sigs[ctr].tempsig_toE);
                            (temp1[ctr])->op_port[1](temp_sigs[ctr].tempsig_fromE);

                            ctr++;

                        }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////

                        else
                        {

                            (ptr_c nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_fromE);
                            (ptr_c nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_toE);

                        }
                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr_c nwtile[i][(j+1)%cols])->estOut_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].est_sig_fromE);
                        (ptr_c nwtile[i][(j+1)%cols])->estIn_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].est_sig_toE);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_c nwtile[i][(j+1)%cols])->credit_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_fromE[k]);
                            (ptr_c nwtile[i][(j+1)%cols])->credit_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_toE[k]);
                        }
                        /*added*/
                        (ptr_c nwtile[i][(j+1)%cols])->p_credit_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].power_sig_fromE);
                        (ptr_c nwtile[i][(j+1)%cols])->p_credit_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].power_sig_toE);
                        /*end*/
                    }
                    else if (border(id_E))  	// East neighbor is border tile
                    {


//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN///////////////////////
                        if (id_E==fail[sourceE][1] && flagE==1)
                        {

                            flagE=0;
                            (temp1[ctr])->ip_port[0](sigs[i][j].sig_toE);
                            (temp1[ctr])->op_port[0](sigs[i][j].sig_fromE);

                            (ptr_b nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](temp_sigs[ctr].tempsig_toE);
                            (ptr_b nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](temp_sigs[ctr].tempsig_fromE);


                            (temp1[ctr])->ip_port[1](temp_sigs[ctr].tempsig_toE);
                            (temp1[ctr])->op_port[1](temp_sigs[ctr].tempsig_fromE);

                            ctr++;

                        }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////
                        else
                        {

                            (ptr_b nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_fromE);
                            (ptr_b nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_toE);

                        }
                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr_b nwtile[i][(j+1)%cols])->estOut_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].est_sig_fromE);
                        (ptr_b nwtile[i][(j+1)%cols])->estIn_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].est_sig_toE);
                        //qrt*************************************************************************************************

                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr_b nwtile[i][(j+1)%cols])->credit_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_fromE[k]);
                            (ptr_b nwtile[i][(j+1)%cols])->credit_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_toE[k]);
                        }
                        /*added*/
                        (ptr_b nwtile[i][(j+1)%cols])->p_credit_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].power_sig_fromE);
                        (ptr_b nwtile[i][(j+1)%cols])->p_credit_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].power_sig_toE);
                        /*end*/
                    }
                    else  	// East neighbor is generic tile
                    {

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN///////////////////////
                        if (id_E==fail[sourceE][1] && flagE==1)
                        {
                            flagE=0;
                            (temp1[ctr])->ip_port[0](sigs[i][j].sig_toE);
                            (temp1[ctr])->op_port[0](sigs[i][j].sig_fromE);

                            (ptr nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](temp_sigs[ctr].tempsig_toE);
                            (ptr nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](temp_sigs[ctr].tempsig_fromE);


                            (temp1[ctr])->ip_port[1](temp_sigs[ctr].tempsig_toE);
                            (temp1[ctr])->op_port[1](temp_sigs[ctr].tempsig_fromE);

                            ctr++;

                        }
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN/////////////////////////

                        else
                        {
                            (ptr nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_fromE);
                            (ptr nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_toE);
                        }

                        //qrt*************************************************************************************************
                        /////////////EST_LINE////////////////
                        (ptr nwtile[i][(j+1)%cols])->estOut_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].est_sig_fromE);
                        (ptr nwtile[i][(j+1)%cols])->estIn_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].est_sig_toE);
                        //qrt*************************************************************************************************
                        for (int k = 0; k < NUM_VCS; k++)
                        {
                            (ptr nwtile[i][(j+1)%cols])->credit_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_fromE[k]);
                            (ptr nwtile[i][(j+1)%cols])->credit_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_toE[k]);
                        }
                        /*added*/
                        (ptr nwtile[i][(j+1)%cols])->p_credit_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].power_sig_fromE);
                        (ptr nwtile[i][(j+1)%cols])->p_credit_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].power_sig_toE);
                        /*end*/
                    }
                }
                break;
            }
        }
    }

    SC_THREAD(entry);	// Thread entry() sensitive to clock
    sensitive_pos << switch_cntrl;
}

///////////////////////////////////////////////////////////
/// This thread keeps track of global simulation count.
/// It also closes logfiles upon completion of simulation.
////////////////////////////////////////////////////////////
void NoC::entry()
{
	bool flag=0;

    /*added*/
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            (nwtile[i][j])->Pavg=0;
            (nwtile[i][j])->Plink=0;
        }
    }

    /*end*/
    while (true)
    {

        int count=0;
        /*added*/
        ///initializing
        /*added*/
        /*for(int i = 0; i < num_rows; i++) {
        	for(int j = 0; j < num_cols; j++) {
        			(nwtile[i][j])->prev_buf_read=0;
        			(nwtile[i][j])->prev_buf_write=0;
        			(nwtile[i][j])->prev_xbar_trav=0;
        			(nwtile[i][j])->prev_link_trav=0;
        			(nwtile[i][j])->prev_vc_reads=0;
        			(nwtile[i][j])->prev_vc_writes=0;
        			(nwtile[i][j])->prev_n_arb=0;
        		}
        	}*/
        /*end*/
        ULL sim_count = 0;
        ULL prev_qlat = 0;
        ULL prev_packets = 0;

        ULL prev_qlatGT = 0;
        ULL prev_packetsGT = 0;

        ULL prev_qlatBE = 0;
        ULL prev_packetsBE = 0;


        ULL prev_forwarded = 0;
        ULL prev_ic_pktin = 0;
        double clock_latency = 0;
        ULL temp_latency = 0;
        ULL temp_packets = 0;

        double clock_time_insim;
        ULL temp_time_insim = 0;
        ULL temp_oc_pktout_core;

        int isCout = 0;
        /*	cout<<"Do you want Cout?"<<endl;
        	cin>>isCout;*/

        ULL prev_latency=0;
        ULL prev_no_of_packets=0;
        ULL prev_latencyGT=0;
        ULL prev_no_of_packetsGT=0;
        ULL prev_latencyBE=0;
        ULL prev_no_of_packetsBE=0;

        while (sim_count < SIM_NUM)
        {
            wait();
            sim_count++;
            /*added*/
            if (sim_count%INTERVAL==0)
            {
                count++;
                for (int i = 0; i < num_rows; i++)
                {
                    for (int j = 0; j < num_cols; j++)
                    {
                        (nwtile[i][j])->buf_read=(nwtile[i][j])->return_buf_read()-(nwtile[i][j])->prev_buf_read;
                        (nwtile[i][j])->prev_buf_read =(nwtile[i][j])->return_buf_read();
                        (nwtile[i][j])->buf_write=(nwtile[i][j])->return_buf_write()-(nwtile[i][j])->prev_buf_write;
                        (nwtile[i][j])->prev_buf_write =(nwtile[i][j])->return_buf_write();
                        (nwtile[i][j])->xbar_trav=(nwtile[i][j])->return_xbar_trav()-(nwtile[i][j])->prev_xbar_trav;
                        (nwtile[i][j])->prev_xbar_trav =(nwtile[i][j])->return_xbar_trav();
                        (nwtile[i][j])->link_trav=(nwtile[i][j])->return_link_trav()-(nwtile[i][j])->prev_link_trav;
                        (nwtile[i][j])->prev_link_trav =(nwtile[i][j])->return_link_trav();
                        (nwtile[i][j])->vc_reads=(nwtile[i][j])->return_vc_reads()-(nwtile[i][j])->prev_vc_reads;
                        (nwtile[i][j])->prev_vc_reads =(nwtile[i][j])->return_vc_reads();
                        (nwtile[i][j])->vc_writes=(nwtile[i][j])->return_vc_writes()-(nwtile[i][j])->prev_vc_writes;
                        (nwtile[i][j])->prev_vc_writes =(nwtile[i][j])->return_vc_writes();
                        (nwtile[i][j])->n_arb=(nwtile[i][j])->return_n_arb()-(nwtile[i][j])->prev_n_arb;
                        (nwtile[i][j])->prev_n_arb =(nwtile[i][j])->return_n_arb();
                    }
                }
                double buf_rl,buf_wl,c_tl,vc_reads,vc_writes,n_arbs,l_tl, temp1,temp2,temp;
                for (int i = 0; i < num_rows; i++)
                {
                    for (int j = 0; j < num_cols; j++)
                    {
                        //parameters:buf_rl: avg no of buffer reads in one clk cycle
                        //buf_wl: avg no. of buffer writes in one clk cycle
                        //c_tl: avg no of xbar traversal per clk cycle
                        //vc_al: avg no. of vc arbitrations per clk cycle

                        buf_rl=((double)(nwtile[i][j])->buf_read)/INTERVAL;
                        buf_wl=((double)(nwtile[i][j])->buf_write)/INTERVAL;
                        c_tl=((double)(nwtile[i][j])->xbar_trav)/INTERVAL;
                        vc_reads=((double)(nwtile[i][j])->vc_reads)/INTERVAL;
                        vc_writes=((double)(nwtile[i][j])->vc_writes)/INTERVAL;
                        n_arbs=((double)(nwtile[i][j])->n_arb)/INTERVAL;
                        l_tl=((double)(nwtile[i][j])->link_trav)/INTERVAL;
                        temp1=power_main(buf_rl,buf_wl,c_tl,n_arbs,vc_reads,vc_writes);
                        temp2=link_power(l_tl);
                        ((nwtile[i][j])->Plink)=((nwtile[i][j])->Plink)+temp2;
                        temp=temp1+temp2;
                        ((nwtile[i][j])->Pavg) = ((nwtile[i][j])->Pavg)+temp;

                    }
                }
                for (int i=0; i<num_rows; i++)
                {
                    for (int j=0; j<num_cols; j++)
                    {
                        nwtile[i][j]->update_power_info();
                    }
                }

            }/*end*/
            ///DYAD////////////////////
            if(congestion==1 && flag==0)
            {
            	congestion_begin_stamp=sim_count;
            	flag=1;
            }
        
            if ((congestion_begin_stamp+settle_cycle) == sim_count)
            {
                congestion=0;
                flag=0;
            }
            ////////////////////////

            ///ADDED FOR Q ROUTER
            ULL total_ic_pktin = 0;
            ULL total_ic_pktout = 0;
            ULL total_ic_estin = 0;
            ULL total_ic_buffersize = 0;

            ULL total_ic_pktin_core = 0;
            ULL total_oc_pktout_core = 0;
            ULL total_pkt_dropped = 0;
            //Controller
            ULL total_ctr_estout = 0;
            UI total_ctr_estbuffer = 0;
            double total_ctr_estavg = 0.0;
            ULL total_time_insim = 0;

            //total sum of latencies and total number of packets
            ULL total_qlat=0;
            ULL total_packets=0;
            ULL total_qlatGT=0;
            ULL total_packetsGT=0;
            ULL total_qlatBE=0;
            ULL total_packetsBE=0;
            /**********************************************************************************************/
            ULL no_of_pkts_generated=0;
            ULL latency_calculated=0;
            ULL temp1=0,temp2=0;
            ULL no_of_pktsGT_generated=0;
            ULL latencyGT_calculated=0;
            ULL temp1GT=0,temp2GT=0;

            ULL no_of_pktsBE_generated=0;
            ULL latencyBE_calculated=0;
            ULL temp1BE=0,temp2BE=0;

            for (int i=0; i<rows; i++)
            {
                for (int j=0; j<cols; j++)
                {


                    if (corner(j + i*cols))
                    {
                        temp1=(ptr_c nwtile[i][j])->cal_no_of_pkts_per_tile();
                        temp2=(ptr_c nwtile[i][j])->cal_latency_per_tile();

                        temp1GT=(ptr_c nwtile[i][j])->cal_no_of_pktsGT_per_tile();
                        temp2GT=(ptr_c nwtile[i][j])->cal_latencyGT_per_tile();

                        temp1BE=(ptr_c nwtile[i][j])->cal_no_of_pktsBE_per_tile();
                        temp2BE=(ptr_c nwtile[i][j])->cal_latencyBE_per_tile();

                        //cout << "ERROR 1\n";
                    }
                    else if (border(j + i*cols))
                    {
                        temp1=(ptr_b nwtile[i][j])->cal_no_of_pkts_per_tile();
                        temp2=(ptr_b nwtile[i][j])->cal_latency_per_tile();

                        temp1GT=(ptr_b nwtile[i][j])->cal_no_of_pktsGT_per_tile();
                        temp2GT=(ptr_b nwtile[i][j])->cal_latencyGT_per_tile();

                        temp1BE=(ptr_b nwtile[i][j])->cal_no_of_pktsBE_per_tile();
                        temp2BE=(ptr_b nwtile[i][j])->cal_latencyBE_per_tile();

                        //cout << "ERROR 2\n";
                    }
                    else
                    {
                        temp1=(ptr nwtile[i][j])->cal_no_of_pkts_per_tile();
                        temp2=(ptr nwtile[i][j])->cal_latency_per_tile();

                        temp1GT=(ptr nwtile[i][j])->cal_no_of_pktsGT_per_tile();
                        temp2GT=(ptr nwtile[i][j])->cal_latencyGT_per_tile();

                        temp1BE=(ptr nwtile[i][j])->cal_no_of_pktsBE_per_tile();
                        temp2BE=(ptr nwtile[i][j])->cal_latencyBE_per_tile();

                        //cout << "ERROR 3\n";
                    }


//ULL temp=(nwtile[k][l])->cal_no_of_pkts_per_tile();
                    no_of_pkts_generated+=temp1;
                    latency_calculated+=temp2;

                    no_of_pktsGT_generated+=temp1GT;
                    latencyGT_calculated+=temp2GT;

                    no_of_pktsBE_generated+=temp1BE;
                    latencyBE_calculated+=temp2BE;

                    //	cout << sim_count << "\t" << no_of_pkts_per_clk << endl;
                }
            }

            //  cout<<"\n"<<sim_count<<"\t"<<(no_of_pkts_generated - prev_no_of_packets)<<"\t"<<(latency_calculated - prev_latency);

//latency_throughput_result1<<sim_count<<"\t\t"<<(latency_calculated - prev_latency)<<endl;
            if (sim_count==SIM_NUM)
//latency_throughput_result1<<"\nTotal no of packets generated : "<<no_of_pkts_generated;

                prev_latency = latency_calculated;
            prev_no_of_packets = no_of_pkts_generated;

            prev_latencyGT = latencyGT_calculated;
            prev_no_of_packetsGT = no_of_pktsGT_generated;

            prev_latencyBE = latencyBE_calculated;
            prev_no_of_packetsBE = no_of_pktsBE_generated;

            /**********************************************************************************************/





            for (int i=0; i<rows; i++)
            {
                for (int j=0; j<cols; j++)
                {

                    switch (TOPO)
                    {

                    case MESH:
                        if (corner(j + i*cols))
                        {
                            (ptr_c nwtile[i][j])->calc_results();

                            total_time_insim += (ptr_c nwtile[i][j])->rs.total_time_insim;
                            total_qlat+=(ptr_c nwtile[i][j])->return_total_latency();
                            total_packets+=(ptr_c nwtile[i][j])->return_total_packets();
                            total_ic_pktin += (ptr_c nwtile[i][j])->rs.total_ic_pktin;
                            total_ic_pktout += (ptr_c nwtile[i][j])->rs.total_ic_pktout;
                            total_ic_estin += (ptr_c nwtile[i][j])->rs.total_ic_estin;
                            total_ic_buffersize += (ptr_c nwtile[i][j])->rs.total_ic_buffersize;
                            total_ic_pktin_core += (ptr_c nwtile[i][j])->rs.total_ic_pktin_core;
                            total_oc_pktout_core +=  (ptr_c nwtile[i][j])->rs.total_oc_pktout_core;
                            total_pkt_dropped +=  (ptr_c nwtile[i][j])->rs.total_pkt_dropped;
                            total_ctr_estout += (ptr_c nwtile[i][j])->rs.total_ctr_estout;
                            total_ctr_estbuffer += (ptr_c nwtile[i][j])->rs.total_ctr_estbuffer;
                            total_ctr_estavg += (ptr_c nwtile[i][j])->rs.avg_ctr_estvalue;

                            total_qlatGT+=(ptr_c nwtile[i][j])->return_total_latency_GT();
                            total_packetsGT+=(ptr_c nwtile[i][j])->cal_no_of_pktsGT_per_tile();

                            total_qlatBE+=(ptr_c nwtile[i][j])->return_total_latency_BE();
                            total_packetsBE+=(ptr_c nwtile[i][j])->cal_no_of_pktsBE_per_tile();

                        }
                        else if (border(j+i*cols))
                        {
                            (ptr_b nwtile[i][j])->calc_results();

                            total_time_insim += (ptr_b nwtile[i][j])->rs.total_time_insim;
                            total_qlat+=(ptr_b nwtile[i][j])->return_total_latency();
                            total_packets+=(ptr_b nwtile[i][j])->return_total_packets();
                            total_ic_pktin += (ptr_b nwtile[i][j])->rs.total_ic_pktin;
                            total_ic_pktout += (ptr_b nwtile[i][j])->rs.total_ic_pktout;
                            total_ic_estin += (ptr_b nwtile[i][j])->rs.total_ic_estin;
                            total_ic_buffersize += (ptr_b nwtile[i][j])->rs.total_ic_buffersize;
                            total_ic_pktin_core += (ptr_b nwtile[i][j])->rs.total_ic_pktin_core;
                            total_oc_pktout_core +=  (ptr_b nwtile[i][j])->rs.total_oc_pktout_core;
                            total_pkt_dropped +=  (ptr_b nwtile[i][j])->rs.total_pkt_dropped;
                            total_ctr_estout += (ptr_b nwtile[i][j])->rs.total_ctr_estout;
                            total_ctr_estbuffer += (ptr_b nwtile[i][j])->rs.total_ctr_estbuffer;
                            total_ctr_estavg += (ptr_b nwtile[i][j])->rs.avg_ctr_estvalue;

                            total_qlatGT+=(ptr_b nwtile[i][j])->return_total_latency_GT();
                            total_packetsGT+=(ptr_b nwtile[i][j])->cal_no_of_pktsGT_per_tile();

                            total_qlatBE+=(ptr_b nwtile[i][j])->return_total_latency_BE();
                            total_packetsBE+=(ptr_b nwtile[i][j])->cal_no_of_pktsBE_per_tile();


                        }
                        else
                        {
                            (ptr nwtile[i][j])->calc_results();

                            total_time_insim += (ptr nwtile[i][j])->rs.total_time_insim;
                            total_qlat+=(ptr nwtile[i][j])->return_total_latency();
                            total_packets+=(ptr nwtile[i][j])->return_total_packets();
                            total_ic_pktin += (ptr nwtile[i][j])->rs.total_ic_pktin;
                            total_ic_pktout += (ptr nwtile[i][j])->rs.total_ic_pktout;
                            total_ic_estin += (ptr nwtile[i][j])->rs.total_ic_estin;
                            total_ic_buffersize += (ptr nwtile[i][j])->rs.total_ic_buffersize;
                            total_ic_pktin_core += (ptr nwtile[i][j])->rs.total_ic_pktin_core;
                            total_oc_pktout_core +=  (ptr nwtile[i][j])->rs.total_oc_pktout_core;
                            total_pkt_dropped +=  (ptr nwtile[i][j])->rs.total_pkt_dropped;
                            total_ctr_estout += (ptr nwtile[i][j])->rs.total_ctr_estout;
                            total_ctr_estbuffer += (ptr nwtile[i][j])->rs.total_ctr_estbuffer;
                            total_ctr_estavg += (ptr nwtile[i][j])->rs.avg_ctr_estvalue;

                            total_qlatGT+=(ptr nwtile[i][j])->return_total_latency_GT();
                            total_packetsGT+=(ptr nwtile[i][j])->cal_no_of_pktsGT_per_tile();

                            total_qlatBE+=(ptr nwtile[i][j])->return_total_latency_BE();
                            total_packetsBE+=(ptr nwtile[i][j])->cal_no_of_pktsBE_per_tile();


                        }
                        break;

                    case TORUS:
                        (ptr nwtile[i][j])->calc_results();

                        total_time_insim += (ptr nwtile[i][j])->rs.total_time_insim;
                        total_qlat+=(ptr nwtile[i][j])->return_total_latency();
                        total_packets+=(ptr nwtile[i][j])->return_total_packets();
                        total_ic_pktin += (ptr nwtile[i][j])->rs.total_ic_pktin;
                        total_ic_pktout += (ptr nwtile[i][j])->rs.total_ic_pktout;
                        total_ic_estin += (ptr nwtile[i][j])->rs.total_ic_estin;
                        total_ic_buffersize += (ptr nwtile[i][j])->rs.total_ic_buffersize;
                        total_ic_pktin_core += (ptr nwtile[i][j])->rs.total_ic_pktin_core;
                        total_oc_pktout_core +=  (ptr nwtile[i][j])->rs.total_oc_pktout_core;
                        total_pkt_dropped +=  (ptr nwtile[i][j])->rs.total_pkt_dropped;
                        total_ctr_estout += (ptr nwtile[i][j])->rs.total_ctr_estout;
                        total_ctr_estbuffer += (ptr nwtile[i][j])->rs.total_ctr_estbuffer;
                        total_ctr_estavg += (ptr nwtile[i][j])->rs.avg_ctr_estvalue;

                        total_qlatGT+=(ptr nwtile[i][j])->return_total_latency_GT();
                        total_packetsGT+=(ptr nwtile[i][j])->cal_no_of_pktsGT_per_tile();

                        total_qlatBE+=(ptr nwtile[i][j])->return_total_latency_BE();
                        total_packetsBE+=(ptr nwtile[i][j])->cal_no_of_pktsBE_per_tile();


                        break;
                    }
                }
            }

            //Average time inside the simulator (genarated time - arrival time at OC core)
            double avg_time_insim;
            if (total_oc_pktout_core != 0)
                avg_time_insim = (double)total_time_insim/total_oc_pktout_core;
            else
                avg_time_insim = 0;

            //current latency
            ULL total_latency_now = total_qlat - prev_qlat;
            ULL total_packets_now = total_packets - prev_packets;

            ULL total_latencyGT_now = total_qlatGT - prev_qlatGT;
            ULL total_packetsGT_now = total_packetsGT - prev_packetsGT;

            ULL total_latencyBE_now = total_qlatBE - prev_qlatBE;
            ULL total_packetsBE_now = total_packetsBE - prev_packetsBE;


            double latency_now;
            if (total_packets_now > 0)
                latency_now =  (double)total_latency_now/total_packets_now;
            else
                latency_now =  0;


            prev_qlat = total_qlat;
            prev_packets = total_packets;




            double latencyGT_now;
            if (total_packetsGT_now > 0)
                latencyGT_now =  (double)total_latencyGT_now/total_packetsGT_now;
            else
                latencyGT_now =  0;



            prev_qlatGT = total_qlatGT;
            prev_packetsGT = total_packetsGT;


            double latencyBE_now;
            if (total_packetsBE_now > 0)
                latencyBE_now =  (double)total_latencyBE_now/total_packetsBE_now;
            else
                latencyBE_now =  0;



            prev_qlatBE = total_qlatBE;
            prev_packetsBE = total_packetsBE;


            //5 cycle avg time spent
            if (sim_count % 10 == 0)
            {

                if ((total_oc_pktout_core - temp_oc_pktout_core) != 0)
                    clock_time_insim = (double)(total_time_insim - temp_time_insim)/(total_oc_pktout_core - temp_oc_pktout_core);
                else
                    clock_time_insim = 0;

                temp_time_insim = total_time_insim;
                temp_oc_pktout_core = total_oc_pktout_core;
            }


            //5 cycle avg latency
            if (sim_count % 10 == 0)
            {
                if (total_packets - temp_packets )
                    clock_latency = (double)(total_qlat - temp_latency)/(total_packets - temp_packets);
                else
                    clock_latency = 0;
                temp_latency = total_qlat;
                temp_latency = total_packets;
            }

            //Number of packets not received
            ULL packet_diff;
            if (total_ic_pktin_core > 0)
                packet_diff = total_ic_pktin_core - total_oc_pktout_core;
            else
                packet_diff = 0;

            //Ratio of estimate in and etimate out
            double estimate_ratio;
            if (total_ctr_estout > 0)
                estimate_ratio = (double)total_ic_estin/total_ctr_estout;
            else
                estimate_ratio = 0;

            //average estimate
            double avg_estimate = (total_ctr_estavg)/(rows*cols);


            //Number of packets forwarded
            long long total_forwarded =  total_ic_pktout - total_oc_pktout_core;
            long long forwarded_now = total_forwarded - prev_forwarded;

            //Packets in circulation
            ULL pkt_in_circulation = total_ic_pktin - prev_ic_pktin;

            prev_forwarded = total_forwarded;
            prev_ic_pktin = total_ic_pktin;

            //Average Latency
            double avg_latency;
            if (total_packets==0)
                avg_latency = 0;
            else
                avg_latency = (double)total_qlat/total_packets;


            //Average Latency GT
            double avg_latencyGT;
            if (total_packetsGT==0)
                avg_latencyGT = 0;
            else
                avg_latencyGT = (double)total_qlatGT/total_packetsGT;

            //Average Latency BE
            double avg_latencyBE;
            if (total_packetsBE==0)
                avg_latencyBE = 0;
            else
                avg_latencyBE = (double)total_qlatBE/total_packetsBE;


            //cout<<total_ic_pktin<<"\t\t"<<total_ic_pktin_core<<endl;*/

            //////time/////////////pkts_generated/////////////reaching destination//////not reached dest///
            qresults_log<<sim_count<<"\t\t"<<latency_now<<endl; //"\t\t"<<total_ic_pktin_core<<"\t\t"<<total_oc_pktout_core<<"\t\t"<<packet_diff<<"\t\t"<<forwarded_now<<"\t\t"<<pkt_in_circulation<<"\t\t"<<total_ic_pktin<<"\t\t"<<total_ctr_estout<<"\t\t"<<total_ic_estin<<"\t\t"<<estimate_ratio<<"\t\t"<<avg_estimate<<endl;

            //////time/////////////pkts_generated/////////////reaching destination//////not reached dest///
            //	qresults_log<<sim_count<<"\t\t"<<latency_now//<<"\t\t"<<avg_latency<<"\t\t"<<total_ic_pktin_core<<"\t\t"<<total_oc_pktout_core<<"\t\t"<<packet_diff<<"\t\t"<<total_ic_pktin<<"\t\t"<<total_ctr_estout<<"\t\t"<<total_ctr_estbuffer<<"\t\t"<<avg_estimate<<"\t\t"<<forwarded_now<<"\t\t"<<pkt_in_circulation<<"\t\t"<<estimate_ratio<<"\t\t"<<clock_latency<<"\t\t"<<avg_time_insim<<"\t\t"<<clock_time_insim<<endl;

            if (qos==1)
            {
                GTresults_log<<sim_count<<"\t\t"<<avg_latencyGT<<"\n";

                BEresults_log<<sim_count<<"\t\t"<<avg_latencyBE<<"\n";
            }

//if(isCout)
//cout<<sim_count<<"\t\t"<<latency_now<<"\t\t"<<avg_latency<<"\t\t"<<total_ic_pktin_core<<"\t\t"<<total_oc_pktout_core<<"\t\t"<<total_ic_pktin<<"\t\t"<<total_ctr_estout<<"\t\t"<<total_ic_estin<<"\t\t"<<avg_estimate<<endl;
//"\t\t"<<total_ic_buffersize<<"\t"<<total_ctr_estbuffer<<endl;

            //////
            /////////////////////
        }
        /*added*/
        for (int i = 0; i < num_rows; i++)
        {
            for (int j = 0; j < num_cols; j++)
            {
                ((nwtile[i][j])->Pavg) = ((nwtile[i][j])->Pavg)/count;
                ((nwtile[i][j])->Plink)=((nwtile[i][j])->Plink)/count;
                //((nwtile[i][j])->Pavg) = ((nwtile[i][j])->Pavg)/(CLK_PERIOD);

            }
        }

        /*end*/
        ULL total_ic_pktin = 0;
        ULL total_ic_pktout = 0;
        ULL total_ic_estin = 0;
        ULL total_ic_buffersize = 0;
        ULL total_ic_pktin_core = 0;
        ULL total_oc_pktout_core = 0;
        ULL total_pkt_dropped = 0;
        //Controller
        ULL total_ctr_estout = 0;
        UI total_ctr_estbuffer = 0;
        double total_ctr_estavg = 0.0;

        //total sum of latencies and total number of packets
        ULL total_qlat=0;
        ULL total_packets=0;

        /*		cout<<"\t"<<"ic_pktin"<<"\t"<<"ic_pktout"<<"\t"<<"ic_estin"<<"\t"<<"ic_pktin_core"<<"\t"<<"oc_pktout_core"<<"\t"<<"pkt_dropped"<<"\t"<<"ctr_estout"<<"\t"<<"ctr_estvalue";

        		for(int i=0;i<rows;i++){
        			for(int j=0;j<cols;j++){

        				cout<< (i*rows) + j;
        				switch(TOPO) {
        					case MESH:
        					if(corner(j + i*cols)){
        					//	cout<<(ptr_c nwtile[i][j])->name();
        						double latency = (double)(ptr_c nwtile[i][j])->return_total_latency()/(ptr_c nwtile[i][j])->return_total_packets();
        						cout<<": "<<latency;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.total_ic_pktin;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.total_ic_pktout;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.total_ic_estin;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.total_ic_pktin_core;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.total_oc_pktout_core;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.total_pkt_dropped;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.total_ctr_estout;
        						cout<<"\t\t"<<(ptr_c nwtile[i][j])->rs.avg_ctr_estvalue;
        						cout<<endl;
        					}
        					else if(border(j+i*cols)){
        					//	cout<<(ptr_c nwtile[i][j])->name();
        						double latency = (double)(ptr_b nwtile[i][j])->return_total_latency()/(ptr_b nwtile[i][j])->return_total_packets();
        						cout<<": "<<latency;
        						cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.total_ic_pktin;

        cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.total_ic_pktout;
        						cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.total_ic_estin;
        						cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.total_ic_pktin_core;
        						cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.total_oc_pktout_core;
        						cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.total_pkt_dropped;
        						cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.total_ctr_estout;
        						cout<<"\t\t"<<(ptr_b nwtile[i][j])->rs.avg_ctr_estvalue;
        						cout<<endl;
        					}
        					else {
        					//	cout<<(ptr nwtile[i][j])->name();
        						double latency = (double)(ptr nwtile[i][j])->return_total_latency()/(ptr nwtile[i][j])->return_total_packets();
        						cout<<": "<<latency;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_pktin;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_pktout;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_estin;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_pktin_core;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_oc_pktout_core;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_pkt_dropped;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ctr_estout;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.avg_ctr_estvalue;
        						cout<<endl;
        					}
        						break;
        					case TORUS:
        					//	cout<<(ptr nwtile[i][j])->name();
        						double latency = (double)(ptr nwtile[i][j])->return_total_latency()/(ptr nwtile[i][j])->return_total_packets();
        						cout<<": "<<latency;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_pktin;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_pktout;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_estin;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ic_pktin_core;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_oc_pktout_core;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_pkt_dropped;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.total_ctr_estout;
        						cout<<"\t\t"<<(ptr nwtile[i][j])->rs.avg_ctr_estvalue;
        						cout<<endl;
        						break;
        				}
        			}
        		}


        */



        for (int i=0; i < rows; i++)
        {
            for (int j=0; j < cols; j++)
            {
                switch (TOPO)
                {

                case TORUS:
                    (ptr nwtile[i][j])->closeLogs();
                    break;

                case MESH:
                    if (corner(j + i*cols))
                        (ptr_c nwtile[i][j])->closeLogs();
                    else if (border(j + i*cols))
                        (ptr_b nwtile[i][j])->closeLogs();
                    else (ptr nwtile[i][j])->closeLogs();
                    break;
                }
            }
        }
        sc_stop();
    }//end while
}//end entry

