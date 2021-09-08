/*
 * single_tree.cpp
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
/// \file single_tree.cpp
/// \brief Implements routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SINGLETREE_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method to set unique id
////////////////////////////////////////////////
void SINGLETREE_router::setID(UI id_tile, vector<UI> t_id) {
    id = id_tile;
    tree_id = t_id;
    initialize();
}

////////////////////////////////////////////////
/// Method that implements routing
/// inherited from base class router
/// Parameters:
/// - input direction from which flit entered the tile
/// - tileID of source tile
/// - tileID of destination tile
/// returns next hop direction
////////////////////////////////////////////////
UI SINGLETREE_router::calc_next(UI ip_dir, ULL source_id, ULL source_idX, ULL source_idY, ULL dest_id, ULL dst_idX, ULL dst_idY) {
    // insert route logic here
    // return next hop direction(N, S, E, W, C)

    // search the last shared node
    int shared_id = 0;
    UI s_id=source_idX, d_id=dst_idX;
    int s_id_length=0, d_id_length=0;
    UI current_id = tree_id[0];

    int pow_index = 1;
    // Find the shared node
    while((s_id%(UI)pow(10, pow_index) == d_id%(UI)pow(10, pow_index)) && (s_id/(UI)pow(10, pow_index-1) != 0))
    {
        //cout << "\n1: " << s_idX%(UI)pow(10, pow_index);
        //cout << " 2: " << d_idX%(UI)pow(10, pow_index);
        //cout << " index: " << pow_index << endl;
        shared_id = s_id%(UI)pow(10, pow_index);
        pow_index ++;
    }

    // goto next node according to shared node direction
    if(LOG >= 20)
    {
        cout << "src: " << source_idX;
        cout << "  dst: " << dst_idX;
        cout << "  current: " << tree_id[0];
        cout << "  shared node: " << shared_id << endl;
    }

    int direction=5;
    UI  d_id_tmp    = d_id;
    d_id_length = 0;
    // calculate the length of destnation id
    while(d_id_tmp > 0)
    {
        d_id_length ++;
        d_id_tmp /= 10;
    }
    d_id_tmp = d_id;

    if(current_id == d_id)
    {
        if(LOG == 20)
            cout << "Arrive destination" << endl;
        direction = 0; // arrive destination route to local
    }
    else if(current_id == shared_id)
    {
        while(d_id_tmp%(UI)pow(10, d_id_length-1) != current_id)
            d_id_length --;
        int switch_direction = (d_id_tmp%(UI)pow(10, d_id_length)) / (pow(10, d_id_length-1));
        switch(switch_direction)
        {
            case 1:
                direction = 2;
                break;
            case 2:
                direction = 1;
                break;
            case 3:
                direction = 4;
                break;
            case 4:
                direction = 3;
                break;
            default:
                direction = 5;
                break;
        } 
        if(LOG == 20)
            cout << "At shared node " << endl;
    } // current_id==shared_id
    else
    {
        int receive_flag = 0;
        d_id_tmp = d_id;
        
        while(d_id_tmp != shared_id)
        {
            //cout << "d_id: " << d_id_tmp << " d_id_length: " << d_id_length << endl;
            if(d_id_tmp%(UI)pow(10, d_id_length-1) == current_id)
            // d_id_tmp is the next hop of current node
            {
                switch((int)(d_id_tmp/pow(10, d_id_length-1)))
                {
                    case 1:
                        direction = 2;
                        break;
                    case 2:
                        direction = 1;
                        break;
                    case 3:
                        direction = 4;
                        break;
                    case 4:
                        direction = 3;
                        break;
                    default:
                        direction = 5;
                        break;
                }
                if(LOG == 20)
                    cout << "At receive branch" << direction << endl;
                receive_flag = 1;
                break;
            }
            else
            {
                d_id_tmp = d_id_tmp%(UI)pow(10, d_id_length-1);
                d_id_length --;
            }
        } // receive branch
        // send branch
        if(receive_flag != 1)
        {
            int current_id_tmp    = current_id;
            int current_id_length = 0;
            // calculate the length of destnation id
            while(current_id_tmp > 0)
            {
                //cout << "c_id: " << current_id_tmp;
                //cout << " c_id_length: " << current_id_length << endl;
                current_id_length ++;
                current_id_tmp /= 10;
            }

            direction = current_id / pow(10, current_id_length-1);
            //cout << "pow: " << pow(10, current_id_length-1);
            //cout << "  c_id: " << current_id << endl;
            //cout << "At send branch";
            //cout << "  direction = " << direction << endl;
        }
    } // else (send branch or receive branch)
    
    switch(direction)
    {
        case 1:
            if(LOG == 20)
                cout << "goto South\n" << endl;
            return S;
        case 2:
            if(LOG == 20)
                cout << "goto North\n" << endl;
            return N;
        case 3:
            if(LOG == 20)
                cout << "goto East\n" << endl;
            return E;
        case 4:
            if(LOG == 20)
                cout << "goto West\n" << endl;
            return W;
        case 0:
            if(LOG == 20)
                cout << "goto Local\n" << endl;
            return C;
        default:
            return ND;
    }
    return ND;
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void SINGLETREE_router::initialize() {

}

// for dynamic linking
extern "C" {
router *maker() {
    return new SINGLETREE_router;
}
}
