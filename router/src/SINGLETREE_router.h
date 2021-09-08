
/*
 * XY_router.h
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
/// \file XY_router.h
/// \brief Defines class to implement XY routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __SINGLETREE_router__
#define __SINGLETREE_router__

#include "../../core/router.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
/// brief Class to implement XY routing algorithm
///
/// inherited from from class router
//////////////////////////////////////////////////////////////////////
class SINGLETREE_router : public router {
	public:
		/// Constructor
		SINGLETREE_router() { }
		
		/// returns next hop for a given source and destination. ip_dir is the incoming direction
		UI calc_next(UI ip_dir, ULL source_id, ULL source_idX, ULL source_idY, ULL dest_id, ULL dst_idX, ULL dst_idY);
		
		void initialize();	///< any initializations to be done
		void setID(UI tileid, vector<UI> tree_id);	///< set unique id
		UI get_estimate(UI){}
		void update_estimate(UI,UI,UI,ULL){}
		void update_power(int,double ){};
};

#endif
