#ifdef HAVE_FS_AUTOFS
typedef struct {
  int fd;
  int kernelfd;
  int ioctlfd;
  unsigned long wait_queue_token;
  int waiting;
} autofs_fh_t;
#endif