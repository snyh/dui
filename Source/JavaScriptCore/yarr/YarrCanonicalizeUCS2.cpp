/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

// DO NOT EDIT! - this file autogenerated by YarrCanonicalizeUCS2.js

#include "config.h"
#include "yarr/YarrCanonicalizeUCS2.h"

namespace JSC { namespace Yarr {

#include <stdint.h>

uint16_t ucs2CharacterSet0[] = { 0x01c4u, 0x01c5u, 0x01c6u, 0 };
uint16_t ucs2CharacterSet1[] = { 0x01c7u, 0x01c8u, 0x01c9u, 0 };
uint16_t ucs2CharacterSet2[] = { 0x01cau, 0x01cbu, 0x01ccu, 0 };
uint16_t ucs2CharacterSet3[] = { 0x01f1u, 0x01f2u, 0x01f3u, 0 };
uint16_t ucs2CharacterSet4[] = { 0x0392u, 0x03b2u, 0x03d0u, 0 };
uint16_t ucs2CharacterSet5[] = { 0x0395u, 0x03b5u, 0x03f5u, 0 };
uint16_t ucs2CharacterSet6[] = { 0x0398u, 0x03b8u, 0x03d1u, 0 };
uint16_t ucs2CharacterSet7[] = { 0x0345u, 0x0399u, 0x03b9u, 0x1fbeu, 0 };
uint16_t ucs2CharacterSet8[] = { 0x039au, 0x03bau, 0x03f0u, 0 };
uint16_t ucs2CharacterSet9[] = { 0x00b5u, 0x039cu, 0x03bcu, 0 };
uint16_t ucs2CharacterSet10[] = { 0x03a0u, 0x03c0u, 0x03d6u, 0 };
uint16_t ucs2CharacterSet11[] = { 0x03a1u, 0x03c1u, 0x03f1u, 0 };
uint16_t ucs2CharacterSet12[] = { 0x03a3u, 0x03c2u, 0x03c3u, 0 };
uint16_t ucs2CharacterSet13[] = { 0x03a6u, 0x03c6u, 0x03d5u, 0 };
uint16_t ucs2CharacterSet14[] = { 0x1e60u, 0x1e61u, 0x1e9bu, 0 };

static const size_t UCS2_CANONICALIZATION_SETS = 15;
uint16_t* characterSetInfo[UCS2_CANONICALIZATION_SETS] = {
    ucs2CharacterSet0,
    ucs2CharacterSet1,
    ucs2CharacterSet2,
    ucs2CharacterSet3,
    ucs2CharacterSet4,
    ucs2CharacterSet5,
    ucs2CharacterSet6,
    ucs2CharacterSet7,
    ucs2CharacterSet8,
    ucs2CharacterSet9,
    ucs2CharacterSet10,
    ucs2CharacterSet11,
    ucs2CharacterSet12,
    ucs2CharacterSet13,
    ucs2CharacterSet14,
};

const size_t UCS2_CANONICALIZATION_RANGES = 364;
UCS2CanonicalizationRange rangeInfo[UCS2_CANONICALIZATION_RANGES] = {
    { 0x0000u, 0x0040u, 0x0000u, CanonicalizeUnique },
    { 0x0041u, 0x005au, 0x0020u, CanonicalizeRangeLo },
    { 0x005bu, 0x0060u, 0x0000u, CanonicalizeUnique },
    { 0x0061u, 0x007au, 0x0020u, CanonicalizeRangeHi },
    { 0x007bu, 0x00b4u, 0x0000u, CanonicalizeUnique },
    { 0x00b5u, 0x00b5u, 0x0009u, CanonicalizeSet },
    { 0x00b6u, 0x00bfu, 0x0000u, CanonicalizeUnique },
    { 0x00c0u, 0x00d6u, 0x0020u, CanonicalizeRangeLo },
    { 0x00d7u, 0x00d7u, 0x0000u, CanonicalizeUnique },
    { 0x00d8u, 0x00deu, 0x0020u, CanonicalizeRangeLo },
    { 0x00dfu, 0x00dfu, 0x0000u, CanonicalizeUnique },
    { 0x00e0u, 0x00f6u, 0x0020u, CanonicalizeRangeHi },
    { 0x00f7u, 0x00f7u, 0x0000u, CanonicalizeUnique },
    { 0x00f8u, 0x00feu, 0x0020u, CanonicalizeRangeHi },
    { 0x00ffu, 0x00ffu, 0x0079u, CanonicalizeRangeLo },
    { 0x0100u, 0x012fu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0130u, 0x0131u, 0x0000u, CanonicalizeUnique },
    { 0x0132u, 0x0137u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0138u, 0x0138u, 0x0000u, CanonicalizeUnique },
    { 0x0139u, 0x0148u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x0149u, 0x0149u, 0x0000u, CanonicalizeUnique },
    { 0x014au, 0x0177u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0178u, 0x0178u, 0x0079u, CanonicalizeRangeHi },
    { 0x0179u, 0x017eu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x017fu, 0x017fu, 0x0000u, CanonicalizeUnique },
    { 0x0180u, 0x0180u, 0x00c3u, CanonicalizeRangeLo },
    { 0x0181u, 0x0181u, 0x00d2u, CanonicalizeRangeLo },
    { 0x0182u, 0x0185u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0186u, 0x0186u, 0x00ceu, CanonicalizeRangeLo },
    { 0x0187u, 0x0188u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x0189u, 0x018au, 0x00cdu, CanonicalizeRangeLo },
    { 0x018bu, 0x018cu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x018du, 0x018du, 0x0000u, CanonicalizeUnique },
    { 0x018eu, 0x018eu, 0x004fu, CanonicalizeRangeLo },
    { 0x018fu, 0x018fu, 0x00cau, CanonicalizeRangeLo },
    { 0x0190u, 0x0190u, 0x00cbu, CanonicalizeRangeLo },
    { 0x0191u, 0x0192u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x0193u, 0x0193u, 0x00cdu, CanonicalizeRangeLo },
    { 0x0194u, 0x0194u, 0x00cfu, CanonicalizeRangeLo },
    { 0x0195u, 0x0195u, 0x0061u, CanonicalizeRangeLo },
    { 0x0196u, 0x0196u, 0x00d3u, CanonicalizeRangeLo },
    { 0x0197u, 0x0197u, 0x00d1u, CanonicalizeRangeLo },
    { 0x0198u, 0x0199u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x019au, 0x019au, 0x00a3u, CanonicalizeRangeLo },
    { 0x019bu, 0x019bu, 0x0000u, CanonicalizeUnique },
    { 0x019cu, 0x019cu, 0x00d3u, CanonicalizeRangeLo },
    { 0x019du, 0x019du, 0x00d5u, CanonicalizeRangeLo },
    { 0x019eu, 0x019eu, 0x0082u, CanonicalizeRangeLo },
    { 0x019fu, 0x019fu, 0x00d6u, CanonicalizeRangeLo },
    { 0x01a0u, 0x01a5u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x01a6u, 0x01a6u, 0x00dau, CanonicalizeRangeLo },
    { 0x01a7u, 0x01a8u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x01a9u, 0x01a9u, 0x00dau, CanonicalizeRangeLo },
    { 0x01aau, 0x01abu, 0x0000u, CanonicalizeUnique },
    { 0x01acu, 0x01adu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x01aeu, 0x01aeu, 0x00dau, CanonicalizeRangeLo },
    { 0x01afu, 0x01b0u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x01b1u, 0x01b2u, 0x00d9u, CanonicalizeRangeLo },
    { 0x01b3u, 0x01b6u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x01b7u, 0x01b7u, 0x00dbu, CanonicalizeRangeLo },
    { 0x01b8u, 0x01b9u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x01bau, 0x01bbu, 0x0000u, CanonicalizeUnique },
    { 0x01bcu, 0x01bdu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x01beu, 0x01beu, 0x0000u, CanonicalizeUnique },
    { 0x01bfu, 0x01bfu, 0x0038u, CanonicalizeRangeLo },
    { 0x01c0u, 0x01c3u, 0x0000u, CanonicalizeUnique },
    { 0x01c4u, 0x01c6u, 0x0000u, CanonicalizeSet },
    { 0x01c7u, 0x01c9u, 0x0001u, CanonicalizeSet },
    { 0x01cau, 0x01ccu, 0x0002u, CanonicalizeSet },
    { 0x01cdu, 0x01dcu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x01ddu, 0x01ddu, 0x004fu, CanonicalizeRangeHi },
    { 0x01deu, 0x01efu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x01f0u, 0x01f0u, 0x0000u, CanonicalizeUnique },
    { 0x01f1u, 0x01f3u, 0x0003u, CanonicalizeSet },
    { 0x01f4u, 0x01f5u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x01f6u, 0x01f6u, 0x0061u, CanonicalizeRangeHi },
    { 0x01f7u, 0x01f7u, 0x0038u, CanonicalizeRangeHi },
    { 0x01f8u, 0x021fu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0220u, 0x0220u, 0x0082u, CanonicalizeRangeHi },
    { 0x0221u, 0x0221u, 0x0000u, CanonicalizeUnique },
    { 0x0222u, 0x0233u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0234u, 0x0239u, 0x0000u, CanonicalizeUnique },
    { 0x023au, 0x023au, 0x2a2bu, CanonicalizeRangeLo },
    { 0x023bu, 0x023cu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x023du, 0x023du, 0x00a3u, CanonicalizeRangeHi },
    { 0x023eu, 0x023eu, 0x2a28u, CanonicalizeRangeLo },
    { 0x023fu, 0x0240u, 0x2a3fu, CanonicalizeRangeLo },
    { 0x0241u, 0x0242u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x0243u, 0x0243u, 0x00c3u, CanonicalizeRangeHi },
    { 0x0244u, 0x0244u, 0x0045u, CanonicalizeRangeLo },
    { 0x0245u, 0x0245u, 0x0047u, CanonicalizeRangeLo },
    { 0x0246u, 0x024fu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0250u, 0x0250u, 0x2a1fu, CanonicalizeRangeLo },
    { 0x0251u, 0x0251u, 0x2a1cu, CanonicalizeRangeLo },
    { 0x0252u, 0x0252u, 0x2a1eu, CanonicalizeRangeLo },
    { 0x0253u, 0x0253u, 0x00d2u, CanonicalizeRangeHi },
    { 0x0254u, 0x0254u, 0x00ceu, CanonicalizeRangeHi },
    { 0x0255u, 0x0255u, 0x0000u, CanonicalizeUnique },
    { 0x0256u, 0x0257u, 0x00cdu, CanonicalizeRangeHi },
    { 0x0258u, 0x0258u, 0x0000u, CanonicalizeUnique },
    { 0x0259u, 0x0259u, 0x00cau, CanonicalizeRangeHi },
    { 0x025au, 0x025au, 0x0000u, CanonicalizeUnique },
    { 0x025bu, 0x025bu, 0x00cbu, CanonicalizeRangeHi },
    { 0x025cu, 0x025fu, 0x0000u, CanonicalizeUnique },
    { 0x0260u, 0x0260u, 0x00cdu, CanonicalizeRangeHi },
    { 0x0261u, 0x0262u, 0x0000u, CanonicalizeUnique },
    { 0x0263u, 0x0263u, 0x00cfu, CanonicalizeRangeHi },
    { 0x0264u, 0x0264u, 0x0000u, CanonicalizeUnique },
    { 0x0265u, 0x0265u, 0xa528u, CanonicalizeRangeLo },
    { 0x0266u, 0x0267u, 0x0000u, CanonicalizeUnique },
    { 0x0268u, 0x0268u, 0x00d1u, CanonicalizeRangeHi },
    { 0x0269u, 0x0269u, 0x00d3u, CanonicalizeRangeHi },
    { 0x026au, 0x026au, 0x0000u, CanonicalizeUnique },
    { 0x026bu, 0x026bu, 0x29f7u, CanonicalizeRangeLo },
    { 0x026cu, 0x026eu, 0x0000u, CanonicalizeUnique },
    { 0x026fu, 0x026fu, 0x00d3u, CanonicalizeRangeHi },
    { 0x0270u, 0x0270u, 0x0000u, CanonicalizeUnique },
    { 0x0271u, 0x0271u, 0x29fdu, CanonicalizeRangeLo },
    { 0x0272u, 0x0272u, 0x00d5u, CanonicalizeRangeHi },
    { 0x0273u, 0x0274u, 0x0000u, CanonicalizeUnique },
    { 0x0275u, 0x0275u, 0x00d6u, CanonicalizeRangeHi },
    { 0x0276u, 0x027cu, 0x0000u, CanonicalizeUnique },
    { 0x027du, 0x027du, 0x29e7u, CanonicalizeRangeLo },
    { 0x027eu, 0x027fu, 0x0000u, CanonicalizeUnique },
    { 0x0280u, 0x0280u, 0x00dau, CanonicalizeRangeHi },
    { 0x0281u, 0x0282u, 0x0000u, CanonicalizeUnique },
    { 0x0283u, 0x0283u, 0x00dau, CanonicalizeRangeHi },
    { 0x0284u, 0x0287u, 0x0000u, CanonicalizeUnique },
    { 0x0288u, 0x0288u, 0x00dau, CanonicalizeRangeHi },
    { 0x0289u, 0x0289u, 0x0045u, CanonicalizeRangeHi },
    { 0x028au, 0x028bu, 0x00d9u, CanonicalizeRangeHi },
    { 0x028cu, 0x028cu, 0x0047u, CanonicalizeRangeHi },
    { 0x028du, 0x0291u, 0x0000u, CanonicalizeUnique },
    { 0x0292u, 0x0292u, 0x00dbu, CanonicalizeRangeHi },
    { 0x0293u, 0x0344u, 0x0000u, CanonicalizeUnique },
    { 0x0345u, 0x0345u, 0x0007u, CanonicalizeSet },
    { 0x0346u, 0x036fu, 0x0000u, CanonicalizeUnique },
    { 0x0370u, 0x0373u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0374u, 0x0375u, 0x0000u, CanonicalizeUnique },
    { 0x0376u, 0x0377u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0378u, 0x037au, 0x0000u, CanonicalizeUnique },
    { 0x037bu, 0x037du, 0x0082u, CanonicalizeRangeLo },
    { 0x037eu, 0x0385u, 0x0000u, CanonicalizeUnique },
    { 0x0386u, 0x0386u, 0x0026u, CanonicalizeRangeLo },
    { 0x0387u, 0x0387u, 0x0000u, CanonicalizeUnique },
    { 0x0388u, 0x038au, 0x0025u, CanonicalizeRangeLo },
    { 0x038bu, 0x038bu, 0x0000u, CanonicalizeUnique },
    { 0x038cu, 0x038cu, 0x0040u, CanonicalizeRangeLo },
    { 0x038du, 0x038du, 0x0000u, CanonicalizeUnique },
    { 0x038eu, 0x038fu, 0x003fu, CanonicalizeRangeLo },
    { 0x0390u, 0x0390u, 0x0000u, CanonicalizeUnique },
    { 0x0391u, 0x0391u, 0x0020u, CanonicalizeRangeLo },
    { 0x0392u, 0x0392u, 0x0004u, CanonicalizeSet },
    { 0x0393u, 0x0394u, 0x0020u, CanonicalizeRangeLo },
    { 0x0395u, 0x0395u, 0x0005u, CanonicalizeSet },
    { 0x0396u, 0x0397u, 0x0020u, CanonicalizeRangeLo },
    { 0x0398u, 0x0398u, 0x0006u, CanonicalizeSet },
    { 0x0399u, 0x0399u, 0x0007u, CanonicalizeSet },
    { 0x039au, 0x039au, 0x0008u, CanonicalizeSet },
    { 0x039bu, 0x039bu, 0x0020u, CanonicalizeRangeLo },
    { 0x039cu, 0x039cu, 0x0009u, CanonicalizeSet },
    { 0x039du, 0x039fu, 0x0020u, CanonicalizeRangeLo },
    { 0x03a0u, 0x03a0u, 0x000au, CanonicalizeSet },
    { 0x03a1u, 0x03a1u, 0x000bu, CanonicalizeSet },
    { 0x03a2u, 0x03a2u, 0x0000u, CanonicalizeUnique },
    { 0x03a3u, 0x03a3u, 0x000cu, CanonicalizeSet },
    { 0x03a4u, 0x03a5u, 0x0020u, CanonicalizeRangeLo },
    { 0x03a6u, 0x03a6u, 0x000du, CanonicalizeSet },
    { 0x03a7u, 0x03abu, 0x0020u, CanonicalizeRangeLo },
    { 0x03acu, 0x03acu, 0x0026u, CanonicalizeRangeHi },
    { 0x03adu, 0x03afu, 0x0025u, CanonicalizeRangeHi },
    { 0x03b0u, 0x03b0u, 0x0000u, CanonicalizeUnique },
    { 0x03b1u, 0x03b1u, 0x0020u, CanonicalizeRangeHi },
    { 0x03b2u, 0x03b2u, 0x0004u, CanonicalizeSet },
    { 0x03b3u, 0x03b4u, 0x0020u, CanonicalizeRangeHi },
    { 0x03b5u, 0x03b5u, 0x0005u, CanonicalizeSet },
    { 0x03b6u, 0x03b7u, 0x0020u, CanonicalizeRangeHi },
    { 0x03b8u, 0x03b8u, 0x0006u, CanonicalizeSet },
    { 0x03b9u, 0x03b9u, 0x0007u, CanonicalizeSet },
    { 0x03bau, 0x03bau, 0x0008u, CanonicalizeSet },
    { 0x03bbu, 0x03bbu, 0x0020u, CanonicalizeRangeHi },
    { 0x03bcu, 0x03bcu, 0x0009u, CanonicalizeSet },
    { 0x03bdu, 0x03bfu, 0x0020u, CanonicalizeRangeHi },
    { 0x03c0u, 0x03c0u, 0x000au, CanonicalizeSet },
    { 0x03c1u, 0x03c1u, 0x000bu, CanonicalizeSet },
    { 0x03c2u, 0x03c3u, 0x000cu, CanonicalizeSet },
    { 0x03c4u, 0x03c5u, 0x0020u, CanonicalizeRangeHi },
    { 0x03c6u, 0x03c6u, 0x000du, CanonicalizeSet },
    { 0x03c7u, 0x03cbu, 0x0020u, CanonicalizeRangeHi },
    { 0x03ccu, 0x03ccu, 0x0040u, CanonicalizeRangeHi },
    { 0x03cdu, 0x03ceu, 0x003fu, CanonicalizeRangeHi },
    { 0x03cfu, 0x03cfu, 0x0008u, CanonicalizeRangeLo },
    { 0x03d0u, 0x03d0u, 0x0004u, CanonicalizeSet },
    { 0x03d1u, 0x03d1u, 0x0006u, CanonicalizeSet },
    { 0x03d2u, 0x03d4u, 0x0000u, CanonicalizeUnique },
    { 0x03d5u, 0x03d5u, 0x000du, CanonicalizeSet },
    { 0x03d6u, 0x03d6u, 0x000au, CanonicalizeSet },
    { 0x03d7u, 0x03d7u, 0x0008u, CanonicalizeRangeHi },
    { 0x03d8u, 0x03efu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x03f0u, 0x03f0u, 0x0008u, CanonicalizeSet },
    { 0x03f1u, 0x03f1u, 0x000bu, CanonicalizeSet },
    { 0x03f2u, 0x03f2u, 0x0007u, CanonicalizeRangeLo },
    { 0x03f3u, 0x03f4u, 0x0000u, CanonicalizeUnique },
    { 0x03f5u, 0x03f5u, 0x0005u, CanonicalizeSet },
    { 0x03f6u, 0x03f6u, 0x0000u, CanonicalizeUnique },
    { 0x03f7u, 0x03f8u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x03f9u, 0x03f9u, 0x0007u, CanonicalizeRangeHi },
    { 0x03fau, 0x03fbu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x03fcu, 0x03fcu, 0x0000u, CanonicalizeUnique },
    { 0x03fdu, 0x03ffu, 0x0082u, CanonicalizeRangeHi },
    { 0x0400u, 0x040fu, 0x0050u, CanonicalizeRangeLo },
    { 0x0410u, 0x042fu, 0x0020u, CanonicalizeRangeLo },
    { 0x0430u, 0x044fu, 0x0020u, CanonicalizeRangeHi },
    { 0x0450u, 0x045fu, 0x0050u, CanonicalizeRangeHi },
    { 0x0460u, 0x0481u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0482u, 0x0489u, 0x0000u, CanonicalizeUnique },
    { 0x048au, 0x04bfu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x04c0u, 0x04c0u, 0x000fu, CanonicalizeRangeLo },
    { 0x04c1u, 0x04ceu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x04cfu, 0x04cfu, 0x000fu, CanonicalizeRangeHi },
    { 0x04d0u, 0x0527u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x0528u, 0x0530u, 0x0000u, CanonicalizeUnique },
    { 0x0531u, 0x0556u, 0x0030u, CanonicalizeRangeLo },
    { 0x0557u, 0x0560u, 0x0000u, CanonicalizeUnique },
    { 0x0561u, 0x0586u, 0x0030u, CanonicalizeRangeHi },
    { 0x0587u, 0x109fu, 0x0000u, CanonicalizeUnique },
    { 0x10a0u, 0x10c5u, 0x1c60u, CanonicalizeRangeLo },
    { 0x10c6u, 0x1d78u, 0x0000u, CanonicalizeUnique },
    { 0x1d79u, 0x1d79u, 0x8a04u, CanonicalizeRangeLo },
    { 0x1d7au, 0x1d7cu, 0x0000u, CanonicalizeUnique },
    { 0x1d7du, 0x1d7du, 0x0ee6u, CanonicalizeRangeLo },
    { 0x1d7eu, 0x1dffu, 0x0000u, CanonicalizeUnique },
    { 0x1e00u, 0x1e5fu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x1e60u, 0x1e61u, 0x000eu, CanonicalizeSet },
    { 0x1e62u, 0x1e95u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x1e96u, 0x1e9au, 0x0000u, CanonicalizeUnique },
    { 0x1e9bu, 0x1e9bu, 0x000eu, CanonicalizeSet },
    { 0x1e9cu, 0x1e9fu, 0x0000u, CanonicalizeUnique },
    { 0x1ea0u, 0x1effu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x1f00u, 0x1f07u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f08u, 0x1f0fu, 0x0008u, CanonicalizeRangeHi },
    { 0x1f10u, 0x1f15u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f16u, 0x1f17u, 0x0000u, CanonicalizeUnique },
    { 0x1f18u, 0x1f1du, 0x0008u, CanonicalizeRangeHi },
    { 0x1f1eu, 0x1f1fu, 0x0000u, CanonicalizeUnique },
    { 0x1f20u, 0x1f27u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f28u, 0x1f2fu, 0x0008u, CanonicalizeRangeHi },
    { 0x1f30u, 0x1f37u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f38u, 0x1f3fu, 0x0008u, CanonicalizeRangeHi },
    { 0x1f40u, 0x1f45u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f46u, 0x1f47u, 0x0000u, CanonicalizeUnique },
    { 0x1f48u, 0x1f4du, 0x0008u, CanonicalizeRangeHi },
    { 0x1f4eu, 0x1f50u, 0x0000u, CanonicalizeUnique },
    { 0x1f51u, 0x1f51u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f52u, 0x1f52u, 0x0000u, CanonicalizeUnique },
    { 0x1f53u, 0x1f53u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f54u, 0x1f54u, 0x0000u, CanonicalizeUnique },
    { 0x1f55u, 0x1f55u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f56u, 0x1f56u, 0x0000u, CanonicalizeUnique },
    { 0x1f57u, 0x1f57u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f58u, 0x1f58u, 0x0000u, CanonicalizeUnique },
    { 0x1f59u, 0x1f59u, 0x0008u, CanonicalizeRangeHi },
    { 0x1f5au, 0x1f5au, 0x0000u, CanonicalizeUnique },
    { 0x1f5bu, 0x1f5bu, 0x0008u, CanonicalizeRangeHi },
    { 0x1f5cu, 0x1f5cu, 0x0000u, CanonicalizeUnique },
    { 0x1f5du, 0x1f5du, 0x0008u, CanonicalizeRangeHi },
    { 0x1f5eu, 0x1f5eu, 0x0000u, CanonicalizeUnique },
    { 0x1f5fu, 0x1f5fu, 0x0008u, CanonicalizeRangeHi },
    { 0x1f60u, 0x1f67u, 0x0008u, CanonicalizeRangeLo },
    { 0x1f68u, 0x1f6fu, 0x0008u, CanonicalizeRangeHi },
    { 0x1f70u, 0x1f71u, 0x004au, CanonicalizeRangeLo },
    { 0x1f72u, 0x1f75u, 0x0056u, CanonicalizeRangeLo },
    { 0x1f76u, 0x1f77u, 0x0064u, CanonicalizeRangeLo },
    { 0x1f78u, 0x1f79u, 0x0080u, CanonicalizeRangeLo },
    { 0x1f7au, 0x1f7bu, 0x0070u, CanonicalizeRangeLo },
    { 0x1f7cu, 0x1f7du, 0x007eu, CanonicalizeRangeLo },
    { 0x1f7eu, 0x1fafu, 0x0000u, CanonicalizeUnique },
    { 0x1fb0u, 0x1fb1u, 0x0008u, CanonicalizeRangeLo },
    { 0x1fb2u, 0x1fb7u, 0x0000u, CanonicalizeUnique },
    { 0x1fb8u, 0x1fb9u, 0x0008u, CanonicalizeRangeHi },
    { 0x1fbau, 0x1fbbu, 0x004au, CanonicalizeRangeHi },
    { 0x1fbcu, 0x1fbdu, 0x0000u, CanonicalizeUnique },
    { 0x1fbeu, 0x1fbeu, 0x0007u, CanonicalizeSet },
    { 0x1fbfu, 0x1fc7u, 0x0000u, CanonicalizeUnique },
    { 0x1fc8u, 0x1fcbu, 0x0056u, CanonicalizeRangeHi },
    { 0x1fccu, 0x1fcfu, 0x0000u, CanonicalizeUnique },
    { 0x1fd0u, 0x1fd1u, 0x0008u, CanonicalizeRangeLo },
    { 0x1fd2u, 0x1fd7u, 0x0000u, CanonicalizeUnique },
    { 0x1fd8u, 0x1fd9u, 0x0008u, CanonicalizeRangeHi },
    { 0x1fdau, 0x1fdbu, 0x0064u, CanonicalizeRangeHi },
    { 0x1fdcu, 0x1fdfu, 0x0000u, CanonicalizeUnique },
    { 0x1fe0u, 0x1fe1u, 0x0008u, CanonicalizeRangeLo },
    { 0x1fe2u, 0x1fe4u, 0x0000u, CanonicalizeUnique },
    { 0x1fe5u, 0x1fe5u, 0x0007u, CanonicalizeRangeLo },
    { 0x1fe6u, 0x1fe7u, 0x0000u, CanonicalizeUnique },
    { 0x1fe8u, 0x1fe9u, 0x0008u, CanonicalizeRangeHi },
    { 0x1feau, 0x1febu, 0x0070u, CanonicalizeRangeHi },
    { 0x1fecu, 0x1fecu, 0x0007u, CanonicalizeRangeHi },
    { 0x1fedu, 0x1ff7u, 0x0000u, CanonicalizeUnique },
    { 0x1ff8u, 0x1ff9u, 0x0080u, CanonicalizeRangeHi },
    { 0x1ffau, 0x1ffbu, 0x007eu, CanonicalizeRangeHi },
    { 0x1ffcu, 0x2131u, 0x0000u, CanonicalizeUnique },
    { 0x2132u, 0x2132u, 0x001cu, CanonicalizeRangeLo },
    { 0x2133u, 0x214du, 0x0000u, CanonicalizeUnique },
    { 0x214eu, 0x214eu, 0x001cu, CanonicalizeRangeHi },
    { 0x214fu, 0x215fu, 0x0000u, CanonicalizeUnique },
    { 0x2160u, 0x216fu, 0x0010u, CanonicalizeRangeLo },
    { 0x2170u, 0x217fu, 0x0010u, CanonicalizeRangeHi },
    { 0x2180u, 0x2182u, 0x0000u, CanonicalizeUnique },
    { 0x2183u, 0x2184u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x2185u, 0x24b5u, 0x0000u, CanonicalizeUnique },
    { 0x24b6u, 0x24cfu, 0x001au, CanonicalizeRangeLo },
    { 0x24d0u, 0x24e9u, 0x001au, CanonicalizeRangeHi },
    { 0x24eau, 0x2bffu, 0x0000u, CanonicalizeUnique },
    { 0x2c00u, 0x2c2eu, 0x0030u, CanonicalizeRangeLo },
    { 0x2c2fu, 0x2c2fu, 0x0000u, CanonicalizeUnique },
    { 0x2c30u, 0x2c5eu, 0x0030u, CanonicalizeRangeHi },
    { 0x2c5fu, 0x2c5fu, 0x0000u, CanonicalizeUnique },
    { 0x2c60u, 0x2c61u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x2c62u, 0x2c62u, 0x29f7u, CanonicalizeRangeHi },
    { 0x2c63u, 0x2c63u, 0x0ee6u, CanonicalizeRangeHi },
    { 0x2c64u, 0x2c64u, 0x29e7u, CanonicalizeRangeHi },
    { 0x2c65u, 0x2c65u, 0x2a2bu, CanonicalizeRangeHi },
    { 0x2c66u, 0x2c66u, 0x2a28u, CanonicalizeRangeHi },
    { 0x2c67u, 0x2c6cu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x2c6du, 0x2c6du, 0x2a1cu, CanonicalizeRangeHi },
    { 0x2c6eu, 0x2c6eu, 0x29fdu, CanonicalizeRangeHi },
    { 0x2c6fu, 0x2c6fu, 0x2a1fu, CanonicalizeRangeHi },
    { 0x2c70u, 0x2c70u, 0x2a1eu, CanonicalizeRangeHi },
    { 0x2c71u, 0x2c71u, 0x0000u, CanonicalizeUnique },
    { 0x2c72u, 0x2c73u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x2c74u, 0x2c74u, 0x0000u, CanonicalizeUnique },
    { 0x2c75u, 0x2c76u, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x2c77u, 0x2c7du, 0x0000u, CanonicalizeUnique },
    { 0x2c7eu, 0x2c7fu, 0x2a3fu, CanonicalizeRangeHi },
    { 0x2c80u, 0x2ce3u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0x2ce4u, 0x2ceau, 0x0000u, CanonicalizeUnique },
    { 0x2cebu, 0x2ceeu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0x2cefu, 0x2cffu, 0x0000u, CanonicalizeUnique },
    { 0x2d00u, 0x2d25u, 0x1c60u, CanonicalizeRangeHi },
    { 0x2d26u, 0xa63fu, 0x0000u, CanonicalizeUnique },
    { 0xa640u, 0xa66du, 0x0000u, CanonicalizeAlternatingAligned },
    { 0xa66eu, 0xa67fu, 0x0000u, CanonicalizeUnique },
    { 0xa680u, 0xa697u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0xa698u, 0xa721u, 0x0000u, CanonicalizeUnique },
    { 0xa722u, 0xa72fu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0xa730u, 0xa731u, 0x0000u, CanonicalizeUnique },
    { 0xa732u, 0xa76fu, 0x0000u, CanonicalizeAlternatingAligned },
    { 0xa770u, 0xa778u, 0x0000u, CanonicalizeUnique },
    { 0xa779u, 0xa77cu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0xa77du, 0xa77du, 0x8a04u, CanonicalizeRangeHi },
    { 0xa77eu, 0xa787u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0xa788u, 0xa78au, 0x0000u, CanonicalizeUnique },
    { 0xa78bu, 0xa78cu, 0x0000u, CanonicalizeAlternatingUnaligned },
    { 0xa78du, 0xa78du, 0xa528u, CanonicalizeRangeHi },
    { 0xa78eu, 0xa78fu, 0x0000u, CanonicalizeUnique },
    { 0xa790u, 0xa791u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0xa792u, 0xa79fu, 0x0000u, CanonicalizeUnique },
    { 0xa7a0u, 0xa7a9u, 0x0000u, CanonicalizeAlternatingAligned },
    { 0xa7aau, 0xff20u, 0x0000u, CanonicalizeUnique },
    { 0xff21u, 0xff3au, 0x0020u, CanonicalizeRangeLo },
    { 0xff3bu, 0xff40u, 0x0000u, CanonicalizeUnique },
    { 0xff41u, 0xff5au, 0x0020u, CanonicalizeRangeHi },
    { 0xff5bu, 0xffffu, 0x0000u, CanonicalizeUnique },
};

const size_t LATIN_CANONICALIZATION_RANGES = 20;
LatinCanonicalizationRange latinRangeInfo[LATIN_CANONICALIZATION_RANGES] = {
    { 0x0000u, 0x0040u, 0x0000u, CanonicalizeLatinSelf },
    { 0x0041u, 0x005au, 0x0000u, CanonicalizeLatinMask0x20 },
    { 0x005bu, 0x0060u, 0x0000u, CanonicalizeLatinSelf },
    { 0x0061u, 0x007au, 0x0000u, CanonicalizeLatinMask0x20 },
    { 0x007bu, 0x00bfu, 0x0000u, CanonicalizeLatinSelf },
    { 0x00c0u, 0x00d6u, 0x0000u, CanonicalizeLatinMask0x20 },
    { 0x00d7u, 0x00d7u, 0x0000u, CanonicalizeLatinSelf },
    { 0x00d8u, 0x00deu, 0x0000u, CanonicalizeLatinMask0x20 },
    { 0x00dfu, 0x00dfu, 0x0000u, CanonicalizeLatinSelf },
    { 0x00e0u, 0x00f6u, 0x0000u, CanonicalizeLatinMask0x20 },
    { 0x00f7u, 0x00f7u, 0x0000u, CanonicalizeLatinSelf },
    { 0x00f8u, 0x00feu, 0x0000u, CanonicalizeLatinMask0x20 },
    { 0x00ffu, 0x00ffu, 0x0000u, CanonicalizeLatinSelf },
    { 0x0100u, 0x0177u, 0x0000u, CanonicalizeLatinInvalid },
    { 0x0178u, 0x0178u, 0x00ffu, CanonicalizeLatinOther },
    { 0x0179u, 0x039bu, 0x0000u, CanonicalizeLatinInvalid },
    { 0x039cu, 0x039cu, 0x00b5u, CanonicalizeLatinOther },
    { 0x039du, 0x03bbu, 0x0000u, CanonicalizeLatinInvalid },
    { 0x03bcu, 0x03bcu, 0x00b5u, CanonicalizeLatinOther },
    { 0x03bdu, 0xffffu, 0x0000u, CanonicalizeLatinInvalid },
};

} } // JSC::Yarr

