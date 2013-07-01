/**
 * @file   Mutex.h
 * @Author icke2063
 * @date   01.06.2013
 * @brief  Mutex implementation uses boost
 *
 * Copyright © 2013 icke2063 <icke2063@gmail.com>
 *
 * This framework is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <MBMutex.h>				/*interface*/
#include <boost/thread/mutex.hpp>	/*implementation*/

namespace MB_Gateway {

class Mutex: public MB_Framework::MBMutex {
public:
	Mutex(){};
	virtual ~Mutex(){};

	virtual boost::mutex *getMutex(void){return &m_mutex;}

private:
	boost::mutex m_mutex;
};

} /* namespace MB_Framework */
#endif /* MUTEX_H_ */
