/*
 * File:   minisat_ext.h
 * Author: mikolas
 *
 * Created on November 29, 2010, 5:40 PM
 */
#ifndef MINISATEXT_HH
#define	MINISATEXT_HH

#ifdef USE_SMS
#include "SMSWrap.h"
#else
#if USE_IPASIR
#include "IpasirWrap.h"
#else
#include "minisat_ext_minisat.h"
#endif
#endif

#endif	/* MINISATEXT_HH */

