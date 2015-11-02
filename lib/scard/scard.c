/*
 * libopenemv - a library to work with EMV family of smart cards
 * Copyright (C) 2015 Dmitry Eremin-Solenikov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "openemv/config.h"
#include "openemv/scard.h"
#include "scard_backend.h"
#include "openemv/dump.h"

#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C"
{
#endif
 size_t x_apdu(struct sc *sc,
                       const unsigned char *inbuf, size_t inlen,
                       unsigned char *outbuf, size_t outlen)
{
    
    dump_buffer(inbuf, inlen, stdout);
//    unsigned char  pbSend[256] = {cla, ins, p1, p2, dataSize};
//    memcpy(pbSend + 5, data, dataSize);
//    int dwSend = 5 + dataSize;
//    if (cla == 0x80 && ins == 0xa8) {
//        dwSend ++;              //le
//    }
    //    NSString *str;
    //    str = [XGDByteUtils byteArray2HexString:(char*)pbSend nsize:dwSend];
    //    NSLog(str);
    //[libemvInstance reqsendApdu:pbSend len: dwSend];

//    memcpy(outData, pbRecv, dwRecv);
//    *outDataSize = dwRecv;
    return 0;
}

 int x_printf(const char * format, ...){
    printf(format);
    return 1;
}
void x_shutdown(struct sc *sc){
    
}
void x_connect(struct sc *sc, unsigned idx){
    
}
void x_disconnect(struct sc *sc){
    
}
#ifdef __cplusplus
}
#endif
struct sc *scard_init(const char *driver)
{
    struct sc *scs;
    scs = malloc (sizeof(*scs));
    memset(scs, 0, sizeof(*scs));
    scs->transmit = x_apdu;
    scs->connect = x_connect;
    scs->shutdown = x_shutdown;
    scs->disconnect = x_disconnect;
    scs->proto = SCARD_PROTO_T0;
	if (!driver)
    	driver = openemv_config_get("scard.driver");
    return scs;
	if (!driver)
		return NULL;
	else if (!strcmp(driver, "pcsc"))
		return scard_pcsc_init();
	else if (!strcmp(driver, "emu"))
		return scard_emu_init();
	else if (!strcmp(driver, "apduio_t0"))
		return scard_apduio_t0_init();
	else if (!strcmp(driver, "apduio_t1"))
		return scard_apduio_t1_init();
	else
		return NULL;
}

void scard_shutdown(struct sc *sc)
{
	sc->shutdown(sc);
}

void scard_raise_error(struct sc *sc, int type)
{
	sc->error = type;
}

bool scard_is_error(struct sc *sc)
{
	return sc && (sc->error != SCARD_NO_ERROR);
}

const char *scard_error(struct sc *sc)
{
	switch (sc->error) {
	case SCARD_NO_ERROR:
		return "No error";
	case SCARD_CARD:
		return "Card error";
	case SCARD_MEMORY:
		return "Memory error";
	case SCARD_PARAMETER:
		return "Parameter error";
	}

	return "Unknown???";
}

void scard_connect(struct sc *sc, unsigned idx)
{
	return sc->connect(sc, idx);
}

void scard_disconnect(struct sc *sc)
{
	sc->disconnect(sc);
}

int scard_getproto(struct sc *sc)
{
	if (!sc)
		return SCARD_PROTO_INVALID;

	return sc->proto;
}

size_t scard_transmit(struct sc *sc,
		const unsigned char *inbuf, size_t inlen,
		unsigned char *outbuf, size_t outlen)
{
	return sc->transmit(sc, inbuf, inlen, outbuf, outlen);
}
