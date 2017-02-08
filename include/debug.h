/*
 * debug.h
 *
 *  Created on: 06.01.2017
 *      Author: sefo
 */

#ifndef INCLUDE_DEBUG_H_
#define INCLUDE_DEBUG_H_


#define DEBUG

#ifdef DEBUG
#define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...) NULL
#endif


#endif /* INCLUDE_DEBUG_H_ */
