#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // To use uint8_t
#include <math.h>
#include <time.h>
/* */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>

#include<sys/poll.h>

#define DEVICE "eth0"
#define BUF_SIZE 64
#define MAX_LEN 68
#define MAX_MSG_LEN 500 // Wrong, check it later.
#define STD_TIMEOUT 3000

#define MASK_TAM 252
#define MASK_SEQ 1008
#define MASK_SEQ_MSB 3
#define MASK_SEQ_LSB 240
#define MASK_TYPE 15

#define TYPE_NACK 0
#define TYPE_ACK 1
#define TYPE_CD 3
#define TYPE_LS 4
#define TYPE_PUT 5
#define TYPE_GET 6
#define TYPE_OK 8
#define TYPE_FILESIZE 9
#define TYPE_SHOWSCREEN 10
#define TYPE_ERROR 14
#define TYPE_END 15

typedef struct Attr {
	unsigned short len : 6,
     	  		   seq : 6,
	     		   type : 4;
} Attr;

typedef struct Message {
    unsigned char init : 8;
   	Attr attr;
    unsigned char *data;
    unsigned char par : 8;
} Message;
