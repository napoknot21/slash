#ifndef SLASH_SLASHERR_H
#define SLASH_SLASHERR_H

#define EREDIRECT 1
#define EFAIL 1
#define ECDNOTDIR 63
#define ENOCMD 127
#define EACCES 126
#define EKILL 128
#define ESIG 255
#define EOBOUND 255

extern short slasherrno;
char *get_error();

#endif
