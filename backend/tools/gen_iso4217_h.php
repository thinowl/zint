<?php
/* Generate ISO 4217 include "backend/iso4217.h" for "backend/gs1.c" */
/*
    libzint - the open source barcode library
    Copyright (C) 2021-2024 <rstuart114@gmail.com>
*/
/* SPDX-License-Identifier: BSD-3-Clause */

/* To create "backend/iso4217.h" (from project directory):
 *
 *   php backend/tools/gen_iso4217_h.php > backend/iso4217.h
 */

$basename = basename(__FILE__);
$dirname = dirname(__FILE__);
$dirdirname = basename(dirname($dirname)) . '/' . basename($dirname);

$opts = getopt('c:h:t:');

$print_copyright = isset($opts['c']) ? (bool) $opts['c'] : true;
$print_h_guard = isset($opts['h']) ? (bool) $opts['h'] : true;
$tab = isset($opts['t']) ? $opts['t'] : '    ';

$numeric = array(
        /*ALL*/   8, /*DZD*/  12, /*ARS*/  32, /*AUD*/  36, /*BSD*/  44, /*BHD*/  48, /*BDT*/  50, /*AMD*/  51, /*BBD*/  52, /*BMD*/  60,
        /*BTN*/  64, /*BOB*/  68, /*BWP*/  72, /*BZD*/  84, /*SBD*/  90, /*BND*/  96, /*MMK*/ 104, /*BIF*/ 108, /*KHR*/ 116, /*CAD*/ 124,
        /*CVE*/ 132, /*KYD*/ 136, /*LKR*/ 144, /*CLP*/ 152, /*CNY*/ 156, /*COP*/ 170, /*KMF*/ 174, /*CRC*/ 188, /*HRK*/ 191, /*CUP*/ 192,
        /*CZK*/ 203, /*DKK*/ 208, /*DOP*/ 214, /*SVC*/ 222, /*ETB*/ 230, /*ERN*/ 232, /*FKP*/ 238, /*FJD*/ 242, /*DJF*/ 262, /*GMD*/ 270,
        /*GIP*/ 292, /*GTQ*/ 320, /*GNF*/ 324, /*GYD*/ 328, /*HTG*/ 332, /*HNL*/ 340, /*HKD*/ 344, /*HUF*/ 348, /*ISK*/ 352, /*INR*/ 356,
        /*IDR*/ 360, /*IRR*/ 364, /*IQD*/ 368, /*ILS*/ 376, /*JMD*/ 388, /*JPY*/ 392, /*KZT*/ 398, /*JOD*/ 400, /*KES*/ 404, /*KPW*/ 408,
        /*KRW*/ 410, /*KWD*/ 414, /*KGS*/ 417, /*LAK*/ 418, /*LBP*/ 422, /*LSL*/ 426, /*LRD*/ 430, /*LYD*/ 434, /*MOP*/ 446, /*MWK*/ 454,
        /*MYR*/ 458, /*MVR*/ 462, /*MUR*/ 480, /*MXN*/ 484, /*MNT*/ 496, /*MDL*/ 498, /*MAD*/ 504, /*OMR*/ 512, /*NAD*/ 516, /*NPR*/ 524,
        /*ANG*/ 532, /*AWG*/ 533, /*VUV*/ 548, /*NZD*/ 554, /*NIO*/ 558, /*NGN*/ 566, /*NOK*/ 578, /*PKR*/ 586, /*PAB*/ 590, /*PGK*/ 598,
        /*PYG*/ 600, /*PEN*/ 604, /*PHP*/ 608, /*QAR*/ 634, /*RUB*/ 643, /*RWF*/ 646, /*SHP*/ 654, /*SAR*/ 682, /*SCR*/ 690, /*SLL*/ 694,
        /*SGD*/ 702, /*VND*/ 704, /*SOS*/ 706, /*ZAR*/ 710, /*SSP*/ 728, /*SZL*/ 748, /*SEK*/ 752, /*CHF*/ 756, /*SYP*/ 760, /*THB*/ 764,
        /*TOP*/ 776, /*TTD*/ 780, /*AED*/ 784, /*TND*/ 788, /*UGX*/ 800, /*MKD*/ 807, /*EGP*/ 818, /*GBP*/ 826, /*TZS*/ 834, /*USD*/ 840,
        /*UYU*/ 858, /*UZS*/ 860, /*WST*/ 882, /*YER*/ 886, /*TWD*/ 901, /*ZWG*/ 924, /*SLE*/ 925, /*UYW*/ 927, /*VES*/ 928, /*MRU*/ 929,
        /*STN*/ 930, /*CUC*/ 931, /*ZWL*/ 932, /* TODO: remove 1 Sept 2024 */
        /*BYN*/ 933, /*TMT*/ 934, /*GHS*/ 936, /*SDG*/ 938, /*UYI*/ 940, /*RSD*/ 941, /*MZN*/ 943, /*AZN*/ 944,
        /*RON*/ 946, /*CHE*/ 947, /*CHW*/ 948, /*TRY*/ 949, /*XAF*/ 950, /*XCD*/ 951, /*XOF*/ 952, /*XPF*/ 953, /*XBA*/ 955, /*XBB*/ 956,
        /*XBC*/ 957, /*XBD*/ 958, /*XAU*/ 959, /*XDR*/ 960, /*XAG*/ 961, /*XPT*/ 962, /*XTS*/ 963, /*XPD*/ 964, /*XUA*/ 965, /*ZMW*/ 967,
        /*SRD*/ 968, /*MGA*/ 969, /*COU*/ 970, /*AFN*/ 971, /*TJS*/ 972, /*AOA*/ 973, /*BGN*/ 975, /*CDF*/ 976, /*BAM*/ 977, /*EUR*/ 978,
        /*MXV*/ 979, /*UAH*/ 980, /*GEL*/ 981, /*BOV*/ 984, /*PLN*/ 985, /*BRL*/ 986, /*CLF*/ 990, /*XSU*/ 994, /*USN*/ 997, /*XXX*/ 999,
);

$numeric_tab = array();
$val = 0;
$byte = 0;
$max = $numeric[count($numeric) - 1];
for ($i = 0; $i <= $max; $i++) {
    if ($i && $i % 8 == 0) {
        $numeric_tab[$byte++] = $val;
        $val = 0;
    }
    if (in_array($i, $numeric)) {
        $val |= 1 << ($i & 0x7);
    }
}
$numeric_tab[$byte++] = $val;
$numeric_cnt = count($numeric_tab);

print <<<EOD
/*
 * ISO 4217 currency codes generated by "$dirdirname/$basename"
 */

EOD;

if ($print_copyright) {
print <<<'EOD'
/*
    libzint - the open source barcode library
    Copyright (C) 2021-2024 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* SPDX-License-Identifier: BSD-3-Clause */


EOD;
}

if ($print_h_guard) {
print <<<'EOD'
#ifndef Z_ISO4217_H
#define Z_ISO4217_H

EOD;
}

print <<<EOD

/* Whether ISO 4217-1 numeric */
static int iso4217_numeric(int cc) {
{$tab}static const unsigned char codes[$numeric_cnt] = {
EOD;

for ($i = 0; $i < $numeric_cnt; $i++) {
    if ($i % 8 == 0) {
        print "\n$tab$tab";
    } else {
        print " ";
    }
    printf("0x%02X,", $numeric_tab[$i]);
}
print <<<EOD

{$tab}};
{$tab}int b = cc >> 3;

{$tab}if (b < 0 || b >= $numeric_cnt) {
{$tab}{$tab}return 0;
{$tab}}
{$tab}return codes[b] & (1 << (cc & 0x7)) ? 1 : 0;
}

EOD;

if ($print_h_guard) {
print <<<'EOD'

#endif /* Z_ISO4217_H */

EOD;
}

/* vim: set ts=4 sw=4 et : */
