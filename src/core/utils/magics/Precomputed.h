#ifndef PRECOMPUTED_H
#define PRECOMPUTED_H

#include <stdint.h>

namespace MagicNumbers {
    static constexpr uint64_t rookMasks[64] = {
        0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL,
    };

    static constexpr uint64_t bishopMasks[64] = {
        0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL,
    };

    int32_t rookShifts[64] = {
        52,53,53,53,53,53,53,52,53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,52,53,53,53,53,53,53,52,
    };

    static constexpr int32_t bishopShifts[64] = {

    };

    uint64_t rookMagics[64] = {
        0x1c80001080204000ULL,0x140200240001000ULL,0x4100081100200040ULL,0x4a001074400a0020ULL,0x8200020120249008ULL,0x220008100c8a0021ULL,0x100630004840200ULL,0x1100004200e48100ULL,0x800040002290ULL,0x48080802000c000ULL,0x90806000900080ULL,0x21001002282100ULL,0x1082800400080280ULL,0x1000204010028ULL,0x2000811440200ULL,0x22800080014100ULL,0x22c00280008a6940ULL,0x2020a20040820100ULL,0x4b00820042201200ULL,0x8808010002800ULL,0x800400c004201ULL,0x8048808002004400ULL,0x7028040010011802ULL,0x220220011004084ULL,0x2160400680009020ULL,0x80400880200880ULL,0xa880200980100182ULL,0x1000230100100008ULL,0x452002200342810ULL,0x420040080800200ULL,0x5040010080aULL,0x30000412000140a1ULL,0x8400060800082ULL,0x4100200040c01000ULL,0x1010002800200400ULL,0x82a0210029005000ULL,0x81800800801400ULL,0x2000400808002ULL,0x4120824001005ULL,0x43008042001401ULL,0x6002040008a8002ULL,0x8002600150004000ULL,0x10080400202000ULL,0x8080120120c20008ULL,0x4462009009220004ULL,0x4021000400070008ULL,0x20801040010ULL,0x4025310082420004ULL,0x8389401021800080ULL,0x8210004001a000c0ULL,0x420021000208080ULL,0x51280280100080ULL,0xc00050010080100ULL,0xc41000400083700ULL,0x4509011026080400ULL,0x12800041000280ULL,0xaa2008020164106ULL,0x641044000802011ULL,0x8000401048200101ULL,0x808100100200dULL,0x80090004021800b1ULL,0x5201008802040001ULL,0x860080504823004ULL,0x1000a8402310046ULL,
    };

    static constexpr uint64_t bishopMagics[64] = {

    };
}

#endif