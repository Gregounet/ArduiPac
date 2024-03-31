#include <stdint.h>
#include "arduipac_8048.h"
#include "mnemonics.h"


struct lookup_tag lookup[] = {
		/* 00 */ {"NOP",1,0},
		/* 01 */ {"ILL",1,0},
		/* 02 */ {"OUTL BUS, A",1,0},
		/* 03 */ {"ADD A,",2,1},
		/* 04 */ {"JMP",2,2},
		/* 05 */ {"EN I",1,0},
		/* 06 */ {"ILL",1,0},
		/* 07 */ {"DEC A",1,0},
		/* 08 */ {"INS A, BUS",1,0},
		/* 09 */ {"IN A, P1",1,0},
		/* 0A */ {"IN A, P2",1,0},
		/* 0B */ {"ILL",1,0},
		/* 0C */ {"MOVD A, P4",1,0},
		/* 0D */ {"MOVD A, P5",1,0},
		/* 0E */ {"MOVD A, P6",1,0},
		/* 0F */ {"MOVD A, P7",1,0},
		/* 10 */ {"INC @R0",1,0},
		/* 11 */ {"INC @R1",1,0},
		/* 12 */ {"JB0",2,3},
		/* 13 */ {"ADDC A,",2,1},
		/* 14 */ {"CALL",2,2},
		/* 15 */ {"DIS I",1,0},
		/* 16 */ {"JTF",2,3},
		/* 17 */ {"INC A",1,0},
		/* 18 */ {"INC R0",1,0},
		/* 19 */ {"INC R1",1,0},
		/* 1A */ {"INC R2",1,0},
		/* 1B */ {"INC R3",1,0},
		/* 1C */ {"INC R4",1,0},
		/* 1D */ {"INC R5",1,0},
		/* 1E */ {"INC R6",1,0},
		/* 1F */ {"INC R7",1,0},
		/* 20 */ {"XCH A, @R0",1,0},
		/* 21 */ {"XCH A, @R1",1,0},
		/* 22 */ {"ILL",1,0},
		/* 23 */ {"MOV A,",2,1},
		/* 24 */ {"JMP",2,2},
		/* 25 */ {"EN TCNTI",1,0},
		/* 26 */ {"JNT0",2,3},
		/* 27 */ {"CLR A",1,0},
		/* 28 */ {"XCH A, R0",1,0},
		/* 29 */ {"XCH A, R1",1,0},
		/* 2A */ {"XCH A, R2",1,0},
		/* 2B */ {"XCH A, R3",1,0},
		/* 2C */ {"XCH A, R4",1,0},
		/* 2D */ {"XCH A, R5",1,0},
		/* 2E */ {"XCH A, R6",1,0},
		/* 2F */ {"XCH A, R7",1,0},
		/* 30 */ {"XCHD A, @R0",1,0},
		/* 31 */ {"XCHD A, @R1",1,0},
		/* 32 */ {"JB1",2,3},
		/* 33 */ {"ILL",1,0},
		/* 34 */ {"CALL",2,2},
		/* 35 */ {"DIS TCNTI",1,0},
		/* 36 */ {"JT0",2,3},
		/* 37 */ {"CPL A",1,0},
		/* 38 */ {"ILL",1,0},
		/* 39 */ {"OUTL P1, A",1,0},
		/* 3A */ {"OUTL P2, A",1,0},
		/* 3B */ {"ILL",1,0},
		/* 3C */ {"MOVD P4, A",1,0},
		/* 3D */ {"MOVD P5, A",1,0},
		/* 3E */ {"MOVD P6, A",1,0},
		/* 3F */ {"MOVD P7, A",1,0},
		/* 40 */ {"ORL A, @R0",1,0},
		/* 41 */ {"ORL A, @R1",1,0},
		/* 42 */ {"MOV A, T",1,0},
		/* 43 */ {"ORL A,",2,1},
		/* 44 */ {"JMP",2,2},
		/* 45 */ {"STRT CNT",1,0},
		/* 46 */ {"JNT1",2,3},
		/* 47 */ {"SWAP",1,0},
		/* 48 */ {"ORL A, R0",1,0},
		/* 49 */ {"ORL A, R1",1,0},
		/* 4A */ {"ORL A, R2",1,0},
		/* 4B */ {"ORL A, R3",1,0},
		/* 4C */ {"ORL A, R4",1,0},
		/* 4D */ {"ORL A, R5",1,0},
		/* 4E */ {"ORL A, R6",1,0},
		/* 4F */ {"ORL A, R7",1,0},
		/* 50 */ {"ANL A, @R0",1,0},
		/* 51 */ {"ANL A, @R1",1,0},
		/* 52 */ {"JB2",2,3},
		/* 53 */ {"ANL A,",2,1},
		/* 54 */ {"CALL",2,2},
		/* 55 */ {"STRT T",1,0},
		/* 56 */ {"JT1",2,3},
		/* 57 */ {"DA A",1,0},
		/* 58 */ {"ANL A, R0",1,0},
		/* 59 */ {"ANL A, R1",1,0},
		/* 5A */ {"ANL A, R2",1,0},
		/* 5B */ {"ANL A, R3",1,0},
		/* 5C */ {"ANL A, R4",1,0},
		/* 5D */ {"ANL A, R5",1,0},
		/* 5E */ {"ANL A, R6",1,0},
		/* 5F */ {"ANL A, R7",1,0},
		/* 60 */ {"ADD A, @R0",1,0},
		/* 61 */ {"ADD A, @R1",1,0},
		/* 62 */ {"MOV T, A",1,0},
		/* 63 */ {"ILL",1,0},
		/* 64 */ {"JMP",2,2},
		/* 65 */ {"STOP TCNT",1,0},
		/* 66 */ {"ILL",1,0},
		/* 67 */ {"RRC A",1,0},
		/* 68 */ {"ADD A, R0",1,0},
		/* 69 */ {"ADD A, R1",1,0},
		/* 6A */ {"ADD A, R2",1,0},
		/* 6B */ {"ADD A, R3",1,0},
		/* 6C */ {"ADD A, R4",1,0},
		/* 6D */ {"ADD A, R5",1,0},
		/* 6E */ {"ADD A, R6",1,0},
		/* 6F */ {"ADD A, R7",1,0},
		/* 70 */ {"ADDC A, @R0",1,0},
		/* 71 */ {"ADDC A, @R1",1,0},
		/* 72 */ {"JB3",2,3},
		/* 73 */ {"ILL",1,0},
		/* 74 */ {"CALL",2,2},
		/* 75 */ {"ENT0 CLK",1,0},
		/* 76 */ {"JF1",2,3},
		/* 77 */ {"RR A",1,0},
		/* 78 */ {"ADDC A, R0",1,0},
		/* 79 */ {"ADDC A, R1",1,0},
		/* 7A */ {"ADDC A, R2",1,0},
		/* 7B */ {"ADDC A, R3",1,0},
		/* 7C */ {"ADDC A, R4",1,0},
		/* 7D */ {"ADDC A, R5",1,0},
		/* 7E */ {"ADDC A, R6",1,0},
		/* 7F */ {"ADDC A, R7",1,0},
		/* 80 */ {"MOVX A, @R0",1,0},
		/* 81 */ {"MOVX A, @R1",1,0},
		/* 82 */ {"ILL",1,0},
		/* 83 */ {"RET",1,0},
		/* 84 */ {"JMP",2,2},
		/* 85 */ {"CLR F0",1,0},
		/* 86 */ {"JNI",2,3},
		/* 87 */ {"ILL",1,0},
		/* 88 */ {"ORL BUS,",2,1},
		/* 89 */ {"ORL P1,",2,1},
		/* 8A */ {"ORL P2,",2,1},
		/* 8B */ {"ILL",1,0},
		/* 8C */ {"ORLD P4, A",1,0},
		/* 8D */ {"ORLD P5, A",1,0},
		/* 8E */ {"ORLD P6, A",1,0},
		/* 8F */ {"ORLD P7, A",1,0},
		/* 90 */ {"MOVX @R0, A",1,0},
		/* 91 */ {"MOVX @R1, A",1,0},
		/* 92 */ {"JB4",2,3},
		/* 93 */ {"RETR",1,0},
		/* 94 */ {"CALL",2,2},
		/* 95 */ {"CPL F0",1,0},
		/* 96 */ {"JNZ",2,3},
		/* 97 */ {"CLR C",1,0},
		/* 98 */ {"ANL BUS,",2,1},
		/* 99 */ {"ANL P1,",2,1},
		/* 9A */ {"ANL P2,",2,1},
		/* 9B */ {"ILL",1,0},
		/* 9C */ {"ANLD P4, A",1,0},
		/* 9D */ {"ANLD P5, A",1,0},
		/* 9E */ {"ANLD P6, A",1,0},
		/* 9F */ {"ANLD P7, A",1,0},
		/* A0 */ {"MOV @R0, A",1,0},
		/* A1 */ {"MOV @R1, A",1,0},
		/* A2 */ {"ILL",1,0},
		/* A3 */ {"MOVP A, @A",1,0},
		/* A4 */ {"JMP",2,2},
		/* A5 */ {"CLR F1",1,0},
		/* A6 */ {"ILL",1,0},
		/* A7 */ {"CPL C",1,0},
		/* A8 */ {"MOV R0, A",1,0},
		/* A9 */ {"MOV R1, A",1,0},
		/* AA */ {"MOV R2, A",1,0},
		/* AB */ {"MOV R3, A",1,0},
		/* AC */ {"MOV R4, A",1,0},
		/* AD */ {"MOV R5, A",1,0},
		/* AE */ {"MOV R6, A",1,0},
		/* AF */ {"MOV R7, A",1,0},
		/* B0 */ {"MOV @R0,",2,1},
		/* B1 */ {"MOV @R1,",2,1},
		/* B2 */ {"JB5",2,3},
		/* B3 */ {"JMPP @A",1,0},
		/* B4 */ {"CALL",2,2},
		/* B5 */ {"CPL F1",1,0},
		/* B6 */ {"JF0",2,3},
		/* B7 */ {"ILL",1,0},
		/* B8 */ {"MOV R0,",2,1},
		/* B9 */ {"MOV R1,",2,1},
		/* BA */ {"MOV R2,",2,1},
		/* BB */ {"MOV R3,",2,1},
		/* BC */ {"MOV R4,",2,1},
		/* BD */ {"MOV R5,",2,1},
		/* BE */ {"MOV R6,",2,1},
		/* BF */ {"MOV R7,",2,1},
		/* C0 */ {"ILL",1,0},
		/* C1 */ {"ILL",1,0},
		/* C2 */ {"ILL",1,0},
		/* C3 */ {"ILL",1,0},
		/* C4 */ {"JMP",2,2},
		/* C5 */ {"SEL RB0",1,0},
		/* C6 */ {"JZ",2,3},
		/* C7 */ {"MOV A, PSW",1,0},
		/* C8 */ {"DEC R0",1,0},
		/* C9 */ {"DEC R1",1,0},
		/* CA */ {"DEC R2",1,0},
		/* CB */ {"DEC R3",1,0},
		/* CC */ {"DEC R4",1,0},
		/* CD */ {"DEC R5",1,0},
		/* CE */ {"DEC R6",1,0},
		/* CF */ {"DEC R7",1,0},
		/* D0 */ {"XRL A, @R0",1,0},
		/* D1 */ {"XRL A, @R1",1,0},
		/* D2 */ {"JB6",2,3},
		/* D3 */ {"XRL A,",2,1},
		/* D4 */ {"CALL",2,2},
		/* D5 */ {"SEL RB1",1,0},
		/* D6 */ {"ILL",1,0},
		/* D7 */ {"MOV PSW,A",1,0},
		/* D8 */ {"XRL A, R0",1,0},
		/* D9 */ {"XRL A, R1",1,0},
		/* DA */ {"XRL A, R2",1,0},
		/* DB */ {"XRL A, R3",1,0},
		/* DC */ {"XRL A, R4",1,0},
		/* DD */ {"XRL A, R5",1,0},
		/* DE */ {"XRL A, R6",1,0},
		/* DF */ {"XRL A, R7",1,0},
		/* E0 */ {"ILL",1,0},
		/* E1 */ {"ILL",1,0},
		/* E2 */ {"ILL",1,0},
		/* E3 */ {"MOVP3 A, @A",1,0},
		/* E4 */ {"JMP",2,2},
		/* E5 */ {"SEL MB0",2,2},
		/* E6 */ {"JNC",2,3},
		/* E7 */ {"RL A",1,0},
		/* E8 */ {"DJNZ R0,",2,3},
		/* E9 */ {"DJNZ R1,",2,3},
		/* EA */ {"DJNZ R2,",2,3},
		/* EB */ {"DJNZ R3,",2,3},
		/* EC */ {"DJNZ R4,",2,3},
		/* ED */ {"DJNZ R5,",2,3},
		/* EE */ {"DJNZ R6,",2,3},
		/* EF */ {"DJNZ R7,",2,3},
		/* F0 */ {"MOV A, @R0",1,0},
		/* F1 */ {"MOV A, @R1",1,0},
		/* F2 */ {"JB7",2,3},
		/* F3 */ {"ILL",1,0},
		/* F4 */ {"CALL",2,2},
		/* F5 */ {"SEL MB1",1,0},
		/* F6 */ {"JC",2,3},
		/* F7 */ {"RLC A",1,0},
		/* F8 */ {"MOV A, R0",1,0},
		/* F9 */ {"MOV A, R1",1,0},
		/* FA */ {"MOV A, R2",1,0},
		/* FB */ {"MOV A, R3",1,0},
		/* FC */ {"MOV A, R4",1,0},
		/* FD */ {"MOV A, R5",1,0},
		/* FE */ {"MOV A, R6",1,0},
		/* FF */ {"MOV A, R7",1,0}
};
