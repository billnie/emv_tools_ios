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

#include "openemv/tlv.h"
#include "openemv/emv_tags.h"

#include <stdlib.h>

enum emv_tag_t {
	EMV_TAG_GENERIC,
	EMV_TAG_BITMASK,
	EMV_TAG_DOL,
	EMV_TAG_CVM_LIST,
	EMV_TAG_STRING,
	EMV_TAG_NUMERIC,
	EMV_TAG_YYMMDD,
};

struct emv_tag {
	tlv_tag_t tag;
	char *name;
	enum emv_tag_t type;
	const void *data;
};

struct emv_tag_bit {
	unsigned bit;
	const char *name;
};

#define EMV_BIT(byte, bit) ((byte - 1) * 8 + (8 - bit))
#define EMV_BIT_FINISH { (~0), NULL }

static const struct emv_tag_bit EMV_AIP[] = {
	{ EMV_BIT(1, 7), "SDA supported" },
	{ EMV_BIT(1, 6), "DDA supported" },
	{ EMV_BIT(1, 5), "Cardholder verification is supported" },
	{ EMV_BIT(1, 4), "Terminal risk management is to be performed" },
	{ EMV_BIT(1, 3), "Issuer authentication is supported" },
	{ EMV_BIT(1, 2), "Reserved for use by the EMV Contactless Specifications" },
	{ EMV_BIT(1, 1), "CDA supported" },
	{ EMV_BIT(2, 8), "Reserved for use by the EMV Contactless Specifications" },
	{ EMV_BIT(2, 7), "Reserved for use by the EMV Contactless Specifications" },
	{ EMV_BIT(2, 6), "Reserved for use by the EMV Contactless Specifications" },
	{ EMV_BIT(2, 1), "Reserved for use by the EMV Contactless Specifications" },
	EMV_BIT_FINISH,
};

static const struct emv_tag_bit EMV_AUC[] = {
	{ EMV_BIT(1, 8), "Valid for domestic cash transactions" },
	{ EMV_BIT(1, 7), "Valid for international cash transactions" },
	{ EMV_BIT(1, 6), "Valid for domestic goods" },
	{ EMV_BIT(1, 5), "Valid for international goods" },
	{ EMV_BIT(1, 4), "Valid for domestic services" },
	{ EMV_BIT(1, 3), "Valid for international services" },
	{ EMV_BIT(1, 2), "Valid for ATMs" },
	{ EMV_BIT(1, 1), "Valid at terminals other than ATMs" },
	{ EMV_BIT(2, 8), "Domestic cashback allowed" },
	{ EMV_BIT(2, 7), "International cashback allowed" },
	EMV_BIT_FINISH,
};

static const struct emv_tag_bit EMV_TVR[] = {
	{ EMV_BIT(1, 8), "Offline data authentication was not performed" },
	{ EMV_BIT(1, 7), "SDA failed" },
	{ EMV_BIT(1, 6), "ICC data missing" },
	{ EMV_BIT(1, 5), "Card appears on terminal exception file" },
	{ EMV_BIT(1, 4), "DDA failed" },
	{ EMV_BIT(1, 3), "CDA failed" },
	{ EMV_BIT(1, 2), "SDA selected" },
	{ EMV_BIT(2, 8), "ICC and terminal have different application versions" },
	{ EMV_BIT(2, 7), "Expired application" },
	{ EMV_BIT(2, 6), "Application not yet effective" },
	{ EMV_BIT(2, 5), "Requested service not allowed for card product" },
	{ EMV_BIT(2, 4), "New card" },
	{ EMV_BIT(3, 8), "Cardholder verification was not successful" },
	{ EMV_BIT(3, 7), "Unrecognised CVM" },
	{ EMV_BIT(3, 6), "PIN Try Limit exceeded" },
	{ EMV_BIT(3, 5), "PIN entry required and PIN pad not present or not working" },
	{ EMV_BIT(3, 4), "PIN entry required, PIN pad present, but PIN was not entered" },
	{ EMV_BIT(3, 3), "Online PIN entered" },
	{ EMV_BIT(4, 8), "Transaction exceeds floor limit" },
	{ EMV_BIT(4, 7), "Lower consecutive offline limit exceeded" },
	{ EMV_BIT(4, 6), "Upper consecutive offline limit exceeded" },
	{ EMV_BIT(4, 5), "Transaction selected randomly for online processing" },
	{ EMV_BIT(4, 4), "Merchant forced transaction online" },
	{ EMV_BIT(5, 8), "Default TDOL used" },
	{ EMV_BIT(5, 7), "Issuer authentication failed" },
	{ EMV_BIT(5, 6), "Script processing failed before final GENERATE AC" },
	{ EMV_BIT(5, 5), "Script processing failed after final GENERATE AC" },
	{ EMV_BIT(5, 4), "Reserved for use by the EMV Contactless Specifications" },
	{ EMV_BIT(5, 3), "Reserved for use by the EMV Contactless Specifications" },
	{ EMV_BIT(5, 2), "Reserved for use by the EMV Contactless Specifications" },
	{ EMV_BIT(5, 1), "Reserved for use by the EMV Contactless Specifications" },
	EMV_BIT_FINISH,
};

static const struct emv_tag emv_tags[] = {
	{ 0x00  , "Unknown ???" },
	{ 0x4f  , "Application Dedicated File (ADF) Name" },
	{ 0x50  , "Application Label", EMV_TAG_STRING },
	{ 0x56  , "Track 1 Data" },
	{ 0x57  , "Track 2 Equivalent Data" },
	{ 0x5a  , "Application Primary Account Number (PAN)" },
	{ 0x5f20, "Cardholder Name", EMV_TAG_STRING },
	{ 0x5f24, "Application Expiration Date", EMV_TAG_YYMMDD },
	{ 0x5f25, "Application Effective Date", EMV_TAG_YYMMDD },
	{ 0x5f28, "Issuer Country Code", EMV_TAG_NUMERIC },
	{ 0x5f2a, "Transaction Currency Code", EMV_TAG_NUMERIC },
	{ 0x5f2d, "Language Preference", EMV_TAG_STRING },
	{ 0x5f30, "Service Code", EMV_TAG_NUMERIC },
	{ 0x5f34, "Application Primary Account Number (PAN) Sequence Number", EMV_TAG_NUMERIC },
	{ 0x61  , "Application Template" },
	{ 0x6f  , "File Control Information (FCI) Template" },
	{ 0x70  , "READ RECORD Response Message Template" },
	{ 0x77  , "Response Message Template Format 2" },
	{ 0x80  , "Response Message Template Format 1" },
	{ 0x82  , "Application Interchange Profile", EMV_TAG_BITMASK, &EMV_AIP },
	{ 0x83  , "Command Template" },
	{ 0x84  , "Dedicated File (DF) Name" },
	{ 0x87  , "Application Priority Indicator" },
	{ 0x88  , "Short File Identifier (SFI)" },
	{ 0x8a  , "Authorisation Response Code" },
	{ 0x8c  , "Card Risk Management Data Object List 1 (CDOL1)", EMV_TAG_DOL },
	{ 0x8d  , "Card Risk Management Data Object List 2 (CDOL2)", EMV_TAG_DOL },
	{ 0x8e  , "Cardholder Verification Method (CVM) List", EMV_TAG_CVM_LIST },
	{ 0x8f  , "Certification Authority Public Key Index" },
	{ 0x90  , "Issuer Public Key Certificate" },
	{ 0x91  , "Issuer Authentication Data" },
	{ 0x92  , "Issuer Public Key Remainder" },
	{ 0x93  , "Signed Static Application Data" },
	{ 0x94  , "Application File Locator (AFL)" },
	{ 0x95  , "Terminal Verification Results" },
	{ 0x9a  , "Transaction Date", EMV_TAG_YYMMDD },
	{ 0x9c  , "Transaction Type" },
	{ 0x9f02, "Amount, Authorised (Numeric)", EMV_TAG_NUMERIC },
	{ 0x9f03, "Amount, Other (Numeric)", EMV_TAG_NUMERIC, },
	{ 0x9f07, "Application Usage Control", EMV_TAG_BITMASK, &EMV_AUC },
	{ 0x9f08, "Application Version Number" },
	{ 0x9f0d, "Issuer Action Code - Default", EMV_TAG_BITMASK, &EMV_TVR },
	{ 0x9f0e, "Issuer Action Code - Denial", EMV_TAG_BITMASK, &EMV_TVR },
	{ 0x9f0f, "Issuer Action Code - Online", EMV_TAG_BITMASK, &EMV_TVR },
	{ 0x9f10, "Issuer Application Data" },
	{ 0x9f11, "Issuer Code Table Index", EMV_TAG_NUMERIC },
	{ 0x9f12, "Application Preferred Name", EMV_TAG_STRING },
	{ 0x9f13, "Last Online Application Transaction Counter (ATC) Register" },
	{ 0x9f17, "Personal Identification Number (PIN) Try Counter" },
	{ 0x9f1a, "Terminal Country Code" },
	{ 0x9f1f, "Track 1 Discretionary Data", EMV_TAG_STRING },
	{ 0x9f21, "Transaction Time" },
	{ 0x9f26, "Application Cryptogram" },
	{ 0x9f27, "Cryptogram Information Data" },
	{ 0x9f2d, "ICC PIN Encipherment Public Key Certificate" },
	{ 0x9f2e, "ICC PIN Encipherment Public Key Exponent" },
	{ 0x9f2f, "ICC PIN Encipherment Public Key Remainder" },
	{ 0x9f32, "Issuer Public Key Exponent" },
	{ 0x9f34, "Cardholder Verification Method (CVM) Results" },
	{ 0x9f35, "Terminal Type" },
	{ 0x9f36, "Application Transaction Counter (ATC)" },
	{ 0x9f37, "Unpredictable Number" },
	{ 0x9f38, "Processing Options Data Object List (PDOL)", EMV_TAG_DOL },
	{ 0x9f42, "Application Currency Code", EMV_TAG_NUMERIC },
	{ 0x9f44, "Application Currency Exponent", EMV_TAG_NUMERIC },
	{ 0x9f45, "Data Authentication Code" },
	{ 0x9f46, "ICC Public Key Certificate" },
	{ 0x9f47, "ICC Public Key Exponent" },
	{ 0x9f48, "ICC Public Key Remainder" },
	{ 0x9f49, "Dynamic Data Authentication Data Object List (DDOL)", EMV_TAG_DOL },
	{ 0x9f4a, "Static Data Authentication Tag List" },
	{ 0x9f4b, "Signed Dynamic Application Data" },
	{ 0x9f4c, "ICC Dynamic Number" },
	{ 0x9f4d, "Log Entry" },
	{ 0x9f4f, "Log Format", EMV_TAG_DOL },
	{ 0x9f62, "PCVC3(Track1)" },
	{ 0x9f63, "PUNATC(Track1)" },
	{ 0x9f64, "NATC(Track1)" },
	{ 0x9f65, "PCVC3(Track2)" },
	{ 0x9f66, "PUNATC(Track2)" },
	{ 0x9f67, "NATC(Track2)" },
	{ 0x9f6b, "Track 2 Data" },
	{ 0xa5  , "File Control Information (FCI) Proprietary Template" },
	{ 0xbf0c, "File Control Information (FCI) Issuer Discretionary Data" },
};

static int emv_sort_tag(tlv_tag_t tag)
{
	return (int)(tag >= 0x100 ? tag : tag << 8);
}

static int emv_tlv_compare(const void *a, const void *b)
{
	const struct tlv *tlv = a;
	const struct emv_tag *tag = b;

	return emv_sort_tag(tlv->tag) - (emv_sort_tag(tag->tag));
}

static const struct emv_tag *emv_get_tag(const struct tlv *tlv)
{
	struct emv_tag *tag = bsearch(tlv, emv_tags, sizeof(emv_tags)/sizeof(emv_tags[0]),
			sizeof(emv_tags[0]), emv_tlv_compare);

	return tag ? tag : &emv_tags[0];
}

static const char *bitstrings[] = {
	".......1",
	"......1.",
	".....1..",
	"....1...",
	"...1....",
	"..1.....",
	".1......",
	"1.......",
};

static void emv_tag_dump_bitmask(const struct tlv *tlv, const struct emv_tag *tag, FILE *f)
{
	const struct emv_tag_bit *bits = tag->data;
	unsigned bit, byte;
	unsigned char val;

	for (byte = 1; byte <= tlv->len; byte ++) {
		val = tlv->value[byte - 1];
		fprintf(f, "\tByte %d (%02x)\n", byte, val);
		for (bit = 8; bit > 0; bit--, val <<= 1) {
			if (val & 0x80)
				fprintf(f, "\t\t%s - '%s'\n", bitstrings[bit - 1],
						bits->bit == EMV_BIT(byte, bit) ? bits->name : "Unknown");
			if (bits->bit == EMV_BIT(byte, bit))
				bits ++;
		}
	}
}

static void emv_tag_dump_dol(const struct tlv *tlv, const struct emv_tag *tag, FILE *f)
{
	const unsigned char *buf = tlv->value;
	size_t left = tlv->len;

	while (left) {
		struct tlv doltlv;
		const struct emv_tag *doltag;

		if (!tlv_parse_tl(&buf, &left, &doltlv)) {
			fprintf(f, "Invalid Tag-Len\n");
			continue;
		}

		doltag = emv_get_tag(&doltlv);

		fprintf(f, "\tTag %4hx len %02zx ('%s')\n", doltlv.tag, doltlv.len, doltag->name);
	}
}

static void emv_tag_dump_string(const struct tlv *tlv, const struct emv_tag *tag, FILE *f)
{
	fprintf(f, "\tString value '");
	fwrite(tlv->value, 1, tlv->len, f);
	fprintf(f, "'\n");
}

static unsigned long emv_value_numeric(const struct tlv *tlv, unsigned start, unsigned end)
{
	unsigned long ret = 0;
	int i;

	if (end > tlv->len * 2)
		return ret;
	if (start >= end)
		return ret;

	if (start & 1) {
		ret += tlv->value[start/2] & 0xf;
		i = start + 1;
	} else
		i = start;

	for (; i < end - 1; i += 2) {
		ret *= 10;
		ret += tlv->value[i/2] >> 4;
		ret *= 10;
		ret += tlv->value[i/2] & 0xf;
	}

	if (end & 1) {
		ret *= 10;
		ret += tlv->value[end/2] >> 4;
	}

	return ret;
}

static void emv_tag_dump_numeric(const struct tlv *tlv, const struct emv_tag *tag, FILE *f)
{
	fprintf(f, "\tNumeric value %lu\n", emv_value_numeric(tlv, 0, tlv->len * 2));
}

static void emv_tag_dump_yymmdd(const struct tlv *tlv, const struct emv_tag *tag, FILE *f)
{
	fprintf(f, "\tDate: 20%02ld.%ld.%ld\n",
			emv_value_numeric(tlv, 0, 2),
			emv_value_numeric(tlv, 2, 4),
			emv_value_numeric(tlv, 4, 6));
}

static uint32_t emv_get_binary(const unsigned char *S)
{
	return (S[0] << 24) | (S[1] << 16) | (S[2] << 8) | (S[3] << 0);
}

static void emv_tag_dump_cvm_list(const struct tlv *tlv, const struct emv_tag *tag, FILE *f)
{
	uint32_t X, Y;
	int i;

	if (tlv->len < 10 || tlv->len % 2) {
		fprintf(f, "\tINVALID!\n");
		return;
	}

	X = emv_get_binary(tlv->value);
	Y = emv_get_binary(tlv->value + 4);

	fprintf(f, "\tX: %d\n", X);
	fprintf(f, "\tY: %d\n", Y);

	for (i = 8; i < tlv->len; i+= 2) {
		const char *method;
		const char *condition;

		switch (tlv->value[i] & 0x3f) {
		case 0x0:
			method = "Fail CVM processing";
			break;
		case 0x1:
			method = "Plaintext PIN verification performed by ICC";
			break;
		case 0x2:
			method = "Enciphered PIN verified online";
			break;
		case 0x3:
			method = "Plaintext PIN verification performed by ICC and signature (paper)";
			break;
		case 0x4:
			method = "Enciphered PIN verification performed by ICC";
			break;
		case 0x5:
			method = "Enciphered PIN verification performed by ICC and signature (paper)";
			break;
		case 0x1e:
			method = "Signature (paper)";
			break;
		case 0x1f:
			method = "No CVM required";
			break;
		case 0x3f:
			method = "NOT AVAILABLE!";
			break;
		default:
			method = "Unknown";
			break;
		}

		switch (tlv->value[i+1]) {
		case 0x00:
			condition = "Always";
			break;
		case 0x01:
			condition = "If unattended cash";
			break;
		case 0x02:
			condition = "If not unattended cash and not manual cash and not purchase with cashback";
			break;
		case 0x03:
			condition = "If terminal supports the CVM";
			break;
		case 0x04:
			condition = "If manual cash";
			break;
		case 0x05:
			condition = "If purchase with cashback";
			break;
		case 0x06:
			condition = "If transaction is in the application currency and is under X value";
			break;
		case 0x07:
			condition = "If transaction is in the application currency and is over X value";
			break;
		case 0x08:
			condition = "If transaction is in the application currency and is under Y value";
			break;
		case 0x09:
			condition = "If transaction is in the application currency and is over Y value";
			break;
		default:
			condition = "Unknown";
			break;
		}

		fprintf(f, "\t%02x %02x: '%s' '%s' and '%s' if this CVM is unsuccessful\n",
				tlv->value[i], tlv->value[i+1],
				method, condition, tlv->value[i] & 0x40 ? "continue" : "fail");
	}
}

bool emv_tag_dump(const struct tlv *tlv, FILE *f)
{
	if (!tlv) {
		fprintf(f, "NULL\n");
		return false;
	}

	const struct emv_tag *tag = emv_get_tag(tlv);

	fprintf(f, "Got tag %4hx len %02zx '%s':\n", tlv->tag, tlv->len, tag->name);

	switch (tag->type) {
	case EMV_TAG_GENERIC:
		break;
	case EMV_TAG_BITMASK:
		emv_tag_dump_bitmask(tlv, tag, f);
		break;
	case EMV_TAG_DOL:
		emv_tag_dump_dol(tlv, tag, f);
		break;
	case EMV_TAG_CVM_LIST:
		emv_tag_dump_cvm_list(tlv, tag, f);
		break;
	case EMV_TAG_STRING:
		emv_tag_dump_string(tlv, tag, f);
		break;
	case EMV_TAG_NUMERIC:
		emv_tag_dump_numeric(tlv, tag, f);
		break;
	case EMV_TAG_YYMMDD:
		emv_tag_dump_yymmdd(tlv, tag, f);
		break;
	};

	return true;
}
