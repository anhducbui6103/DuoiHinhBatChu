#ifndef PROTOCOL_H
#define PROTOCOL_H

#define LOGIN 0x01
#define LOGIN_SUCCESS 0x02
#define LOGIN_FAILURE 0x03
#define SIGNUP 0x04
#define SIGNUP_SUCCESS 0x05
#define SIGNUP_FAILURE 0x06
#define AUTHENTICATE 0x07

#define JOIN_ROOM 0x21
#define JOIN_ROOM_SUCCESS 0x22
#define JOIN_ROOM_FAILURE 0x23
#define OUT_ROOM 0x24

#define QUESTION 0x31
#define BELL_RING 0x32
#define ANSWER 0x33
#define TRUE 0x34
#define FALSE 0x35
#define WIN 0x36
#define LOSE 0x37
#define GIVE_UP 0x38

#endif