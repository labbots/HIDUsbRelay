#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hiddata.h"

#define CMD_ON      0xff
#define CMD_ALL_ON  0xfe
#define CMD_OFF     0xfd
#define CMD_ALL_OFF 0xfc

#define READ_OFFSET    7
#define READ_MASK_1 0x01
#define READ_MASK_2 0x02

enum OpID {
    ID_ON = 0,
    ID_OFF,
    ID_GET
};

/* ------------------------------------------------------------------------- */

static char *usbErrorMessage(int errCode)
{
static char buffer[80];

    switch(errCode){
        case USBOPEN_ERR_ACCESS:      return "Access to device denied";
        case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
        case USBOPEN_ERR_IO:          return "Communication error with device";
        default:
            sprintf(buffer, "Unknown USB error %d", errCode);
            return buffer;
    }
    return NULL;    /* not reached */
}

static usbDevice_t  *openDevice(int number, const char* name)
{
usbDevice_t     *dev = NULL;
unsigned char   rawVid[2] = { 0xc0, 0x16 };  /* = 0x16c0 = 5824 = voti.nl */
unsigned char   rawPid[2] = { 0xdf, 0x05 };  /* obdev's shared PID for HIDs */
char            vendorName[] = "www.dcttech.com";
char            productName[] = "USBRelay1";
int             vid = rawVid[0] + 256 * rawVid[1];
int             pid = rawPid[0] + 256 * rawPid[1];
int             err;


    const char* pName = name ? name : productName;
    
    if((err = usbhidOpenDevice(&dev, vid, vendorName, pid, pName, number, 0)) != 0){
        fprintf(stderr, "error finding %s: %s\n", pName, usbErrorMessage(err));
        return NULL;
    }
    return dev;
}

/* ------------------------------------------------------------------------- */



static void decode(char *buffer, int len)
{
  fprintf(stdout, " 1: %s\n", ((buffer[READ_OFFSET] & READ_MASK_1) > 0) ? "ON" : "OFF");
}

/* ------------------------------------------------------------------------- */

static void usage(char *myName)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s on [1|2|3|4] [-n <number>] [-name <name>]\n", myName);
    fprintf(stderr, "  %s off [1|2|3|4]\n", myName);
    fprintf(stderr, "  %s get\n", myName);
    fprintf(stderr, "  %s -h\n", myName);
}

static void help(char *myName)
{
    fprintf(stdout, "help: %s on|off [1|2|3|4] [-n <number>|all] [-name <name>] [-h]\n", myName);
    fprintf(stdout, "  -h|--help  print this help\n");
    fprintf(stdout, "  on|off [1|2|3|4]  switch specified relay output on/off\n");
    fprintf(stdout, "  get  get relay status\n");
    fprintf(stdout, "  -n <number>|all  use specified relay (if there are more than one relay)\n");
    fprintf(stdout, "  -name <name>  use relay with name, default is \"USBRelay2\"\n");
}

int main(int argc, char **argv)
{
usbDevice_t *dev;
char        buffer[129]; 
int         err;
char        cmd = 0x00;

int         operation = ID_GET;
char*       name      = NULL;
int         number    = 0;
char        channel   = 0;

    memset(buffer, 0, sizeof(buffer));

    if (argc < 2) {
        fprintf(stderr, "too few arguments!\n");
        usage(argv[0]);
        exit(1);
    }
    
    if (strcasecmp(argv[1], "-h") == 0 || strcasecmp(argv[1], "--help") == 0) {
        help(argv[0]);
        exit(1);
    }
    
    int i;
    for(i=1; i<argc; ) {
    
        if (strcasecmp(argv[i], "on") == 0) {
            operation = ID_ON;
            if (argc > i+1 && *(argv[i+1]) != '-') { /* not an option like -n|-h|-name */
                cmd = CMD_ON;
                i++;
                channel = (char)atol(argv[i]);
            } else {
                cmd = CMD_ALL_ON;
            }
        } else if (strcasecmp(argv[i], "off") == 0) {
            operation = ID_OFF;
            if (argc > i+1 && *(argv[i+1]) != '-') {
                cmd = CMD_OFF;
                i++;
                channel = (char)atol(argv[i]);
            } else {
                cmd = CMD_ALL_OFF;
            }
        } else if (strcasecmp(argv[i], "get") == 0) {
            operation = ID_GET;
        } else if (strcasecmp(argv[i], "-n") == 0) {
            if (argc > i+1) {
                i++;
                number = (int)atol(argv[i]);
            }
        } else if (strcasecmp(argv[i], "-name") == 0) {
            if (argc > i+1) {
                i++;
                name = argv[i];
            }
        }
    
        i++;
    }
    

    if ((dev = openDevice(number,name)) == NULL) {
        exit(1);
    }

    if (operation == ID_GET){
        int len = sizeof(buffer);
        if ((err = usbhidGetReport(dev, 0, buffer, &len)) != 0){
            fprintf(stderr, "error reading data: %s\n", usbErrorMessage(err));
        } else {
            decode(buffer + 1, sizeof(buffer) - 1);
        }
    } else {
        buffer[0] = 0x00;
        buffer[1] = cmd;
        
        if (channel > 0) {
            buffer[2] = channel;
        }

        if ((err = usbhidSetReport(dev, buffer, sizeof(buffer))) != 0) {  
            fprintf(stderr, "error writing data: %s\n", usbErrorMessage(err));
        }
    } 

    usbhidCloseDevice(dev);
    return 0;
}

/* ------------------------------------------------------------------------- */
