/**	@file
 *	@author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2004-2005  Bram de Greve
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.sourceforge.net
 */

/** @class liar::kernel::TimePeriod
 *  @brief a period in time
 *  @author Bram de Greve [BdG]
 */

#pragma once
#ifndef LIAR_GUARDIAN_OF_INCLUSION_TIME_PERIOD_H
#define LIAR_GUARDIAN_OF_INCLUSION_TIME_PERIOD_H

#include "kernel_common.h"

namespace liar
{
namespace kernel
{

class LIAR_KERNEL_DLL TimePeriod
{
public:

	TimePeriod(TTime iBegin, TTime iEnd): begin_(iBegin), end_(iEnd) {}

	const TTime begin() const { return begin_; }
	const TTime end() const { return end_; }
	const TTime duration() const { return end_ - begin_; }

	const TTime interpolate(TScalar iTau) const { return begin_ + iTau * duration(); }

	TimePeriod& operator+=(TTime iOffset) 
	{ 
		begin_ += iOffset; 
		end_ += iOffset; 
		return *this; 
	}

private:

	TTime begin_;
	TTime end_;
};

inline TimePeriod operator+(const TimePeriod& iPeriod, TTime iOffset)
{
	TimePeriod result(iPeriod);
	result += iOffset;
	return result;
}

inline TimePeriod operator+(TTime iOffset, const TimePeriod& iPeriod)
{
	TimePeriod result(iPeriod);
	result += iOffset;
	return result;
}

}

}

#endif

// EOF
