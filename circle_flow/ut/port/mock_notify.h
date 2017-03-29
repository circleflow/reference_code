/*
 * mock_notify.h
 *
 *  Created on: 2014-9-5
 *      Author: jingpa
 */

#ifndef MOCK_NOTIFY_H_
#define MOCK_NOTIFY_H_

#include "port.h"
#include "type_ext.h"

void link_notify(UINT8 instance, PORT_NAME port, bool link);

void pair_notify(UINT8 instance, PORT_NAME port_name, PORT_ID front, PORT_ID engine);

#endif
