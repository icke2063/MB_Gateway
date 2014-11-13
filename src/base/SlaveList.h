/**
 * @file   SlaveList.h
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

#ifndef SLAVELIST_H_
#define SLAVELIST_H_

#include <build_options.h>

#ifndef ICKE2063_CRUMBY_NO_CPP11
	#include <memory>

	#define SLAVELIST_H_NS std
#else
	#include <boost/shared_ptr.hpp>

	#define SLAVELIST_H_NS boost
#endif

#include <MBSlaveList.h>
#include <MBVirtualRTUSlave.h>

#include <ThreadPool.h>

namespace icke2063 {
namespace MB_Gateway {

class SlaveList: public MB_Framework::MBSlaveList {
public:
	SlaveList();
	virtual ~SlaveList();

	virtual bool addSlave(SLAVELIST_H_NS::shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> newSlave);
	virtual SLAVELIST_H_NS::shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> removeSlave(uint8_t index);
	virtual SLAVELIST_H_NS::shared_ptr<icke2063::MB_Framework::MBVirtualRTUSlave> getSlave(uint8_t index);

	//shared_ptr<Mutex> getLock();
};

} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* SLAVELIST_H_ */
