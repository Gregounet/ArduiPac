#ifndef ARDUIPAC_KEYS
#define ARDUIPAC_KEYS

extern uint8_t key_map[6][8] ;

extern struct keyb {
	uint8_t keybcode;
	char    *keybname;
} keytab[];

#endif
