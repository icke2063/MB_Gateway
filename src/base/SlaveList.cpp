/**
 * @file   SlaveList.cpp
 * @Author icke2063
 * @date   02.06.2013
 * @brief  Brief description of file.
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

#include "SlaveList.h"
namespace icke2063 {
namespace MB_Gateway {

#ifndef ICKE2063_CRUMBY_NO_CPP11
	using namespace std;
#else
	using namespace boost;
#endif
  

#include "crumby_logging_macros.h"

SlaveList::SlaveList(){
  /* initialisation of lock object */
  m_slavelist_lock = shared_ptr<mutex>(new mutex());
}
SlaveList::~SlaveList(){}  
  
bool SlaveList::addSlave(shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> newSlave) {
    shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> curSlave = dynamic_pointer_cast<icke2063::MB_Framework::MBVirtualRTUSlave>(newSlave);
    uint8_t index;

    if(curSlave.get() == NULL)
    {
      return false;
    }
    index = curSlave->getSlaveAddr();
    lock_guard<mutex> lock(*m_slavelist_lock.get()); //lock slavelist

    std::map<uint8_t, shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> >::iterator it = m_slavelist.find(index); //slave already added?
    if (it == m_slavelist.end())
    {
	    crumby_INFO_WRITE("add new Slave[%u]\n", index);
	    m_slavelist[index] = newSlave;
	    return true;
    }
    else
    {
	    crumby_WARN_WRITE("Cannot add slave[%u]: already listeted\n", index);
    }

    return false;
}

shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> SlaveList::removeSlave(uint8_t index) {
	shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> result;
	lock_guard<mutex> lock(*m_slavelist_lock.get()); //lock slavelist

	std::map<uint8_t, shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> >::iterator it = m_slavelist.find(index); //slave in list?
	if (it != m_slavelist.end()) {
		result = it->second; //get reference
		m_slavelist.erase(it); //remove from list
	}
	return result;
}

shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> SlaveList::getSlave(uint8_t index) {
    shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> result;
	std::map<uint8_t, shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> >::iterator it = m_slavelist.find(index); //slave in list?
	if (it != m_slavelist.end()) {
		return it->second; //get pointer
	}
	return result;
}

//shared_ptr<Mutex> SlaveList::getLock(){return m_slavelist_lock;}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
