#ifndef SLASH_SLASHERR_H
#define SLASH_SLASHERR_H

#define S_EREDIRECT 1
#define S_EFAIL 1
#define S_ECDNOTDIR 63
#define S_ENOCMD 127
#define S_EACCES 126
#define S_ESKILL 128
#define S_ESIG 255
#define S_EOBOUND 255
#define S_EUNKNOWN 1
#define S_ELISTENING 256

extern short slasherrno;
extern short is_exit_call;
//char *get_error();
int of_errno();

#endif
