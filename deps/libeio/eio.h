/*
 * libeio API header
 *
 * Copyright (c) 2007,2008 Marc Alexander Lehmann <libeio@schmorp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 * 
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 * 
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License ("GPL") version 2 or any later version,
 * in which case the provisions of the GPL are applicable instead of
 * the above. If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the BSD license, indicate your decision
 * by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the BSD or the GPL.
 */

#ifndef EIO_H_
#define EIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>

typedef struct eio_req eio_req;

typedef int (*eio_cb)(eio_req *req);

#ifndef EIO_REQ_MEMBERS
# define EIO_REQ_MEMBERS
#endif

#ifndef EIO_STRUCT_STAT
# define EIO_STRUCT_STAT struct stat
#endif

enum {
  EIO_CUSTOM,
  EIO_OPEN, EIO_CLOSE, EIO_DUP2,
  EIO_READ, EIO_WRITE,
  EIO_READAHEAD, EIO_SENDFILE,
  EIO_STAT, EIO_LSTAT, EIO_FSTAT,
  EIO_TRUNCATE, EIO_FTRUNCATE,
  EIO_UTIME, EIO_FUTIME,
  EIO_CHMOD, EIO_FCHMOD,
  EIO_CHOWN, EIO_FCHOWN,
  EIO_SYNC, EIO_FSYNC, EIO_FDATASYNC,
  EIO_MSYNC, EIO_MTOUCH, EIO_SYNC_FILE_RANGE,
  EIO_UNLINK, EIO_RMDIR, EIO_MKDIR, EIO_RENAME,
  EIO_MKNOD, EIO_READDIR,
  EIO_LINK, EIO_SYMLINK, EIO_READLINK,
  EIO_GROUP, EIO_NOP,
  EIO_BUSY
};

/* eio_sync_file_range flags */

enum {
  EIO_SYNC_FILE_RANGE_WAIT_BEFORE = 1,
  EIO_SYNC_FILE_RANGE_WRITE       = 2,
  EIO_SYNC_FILE_RANGE_WAIT_AFTER  = 4
};

typedef double eio_tstamp; /* feel free to use double in your code directly */

/* eio request structure */
/* this structure is mostly read-only */
struct eio_req
{
  eio_req volatile *next; /* private ETP */

  ssize_t result;  /* result of syscall, e.g. result = read (... */
  off_t offs;      /* read, write, truncate, readahead, sync_file_range: file offset */
  size_t size;     /* read, write, readahead, sendfile, msync, sync_file_range: length */
  void *ptr1;      /* all applicable requests: pathname, old name */
  void *ptr2;      /* all applicable requests: new name or memory buffer */
  eio_tstamp nv1;  /* utime, futime: atime; busy: sleep time */
  eio_tstamp nv2;  /* utime, futime: mtime */

  int type;        /* EIO_xxx constant ETP */
  int int1;        /* all applicable requests: file descriptor; sendfile: output fd; open, msync: flags */
  long int2;       /* chown, fchown: uid; sendfile: input fd; open, chmod, mkdir, mknod: file mode, sync_file_range: flags */
  long int3;       /* chown, fchown: gid; mknod: dev_t */
  int errorno;     /* errno value on syscall return */

  unsigned char flags; /* private */
  signed char pri;     /* the priority */

  void *data;
  eio_cb finish;
  void (*destroy)(eio_req *req); /* called when requets no longer needed */
  void (*feed)(eio_req *req);    /* only used for group requests */

  EIO_REQ_MEMBERS

  eio_req *grp, *grp_prev, *grp_next, *grp_first; /* private */
};

/* _private_ flags */
enum {
  EIO_FLAG_CANCELLED = 0x01, /* request was cancelled */
  EIO_FLAG_PTR1_FREE = 0x02, /* need to free(ptr1) */
  EIO_FLAG_PTR2_FREE = 0x04, /* need to free(ptr2) */
  EIO_FLAG_GROUPADD  = 0x08  /* some request was added to the group */
};

enum {
  EIO_PRI_MIN     = -4,
  EIO_PRI_MAX     =  4,
  EIO_PRI_DEFAULT =  0,
};

/* returns < 0 on error, errno set
 * need_poll, if non-zero, will be called when results are available
 * and eio_poll_cb needs to be invoked (it MUST NOT call eio_poll_cb itself).
 * done_poll is called when the need to poll is gone.
 */
int eio_init (void (*want_poll)(void), void (*done_poll)(void));

/* must be called regularly to handle pending requests */
/* returns 0 if all requests were handled, -1 if not, or the value of EIO_FINISH if != 0 */
int eio_poll (void);

/* stop polling if poll took longer than duration seconds */
void eio_set_max_poll_time (eio_tstamp nseconds);
/* do not handle more then count requests in one call to eio_poll_cb */
void eio_set_max_poll_reqs (unsigned int nreqs);

/* set minimum required number
 * maximum wanted number
 * or maximum idle number of threads */
void eio_set_min_parallel (unsigned int nthreads);
void eio_set_max_parallel (unsigned int nthreads);
void eio_set_max_idle     (unsigned int nthreads);

unsigned int eio_nreqs    (void); /* number of requests in-flight */
unsigned int eio_nready   (void); /* number of not-yet handled requests */
unsigned int eio_npending (void); /* numbe rof finished but unhandled requests */
unsigned int eio_nthreads (void); /* number of worker threads in use currently */

/*****************************************************************************/
/* convinience wrappers */

#ifndef EIO_NO_WRAPPERS
eio_req *eio_nop       (int pri, eio_cb cb, void *data); /* does nothing except go through the whole process */
eio_req *eio_busy      (eio_tstamp delay, int pri, eio_cb cb, void *data); /* ties a thread for this long, simulating busyness */
eio_req *eio_sync      (int pri, eio_cb cb, void *data);
eio_req *eio_fsync     (int fd, int pri, eio_cb cb, void *data);
eio_req *eio_fdatasync (int fd, int pri, eio_cb cb, void *data);
eio_req *eio_msync     (void *addr, size_t length, int flags, int pri, eio_cb cb, void *data);
eio_req *eio_mtouch    (void *addr, size_t length, int flags, int pri, eio_cb cb, void *data);
eio_req *eio_sync_file_range (int fd, off_t offset, size_t nbytes, unsigned int flags, int pri, eio_cb cb, void *data);
eio_req *eio_close     (int fd, int pri, eio_cb cb, void *data);
eio_req *eio_readahead (int fd, off_t offset, size_t length, int pri, eio_cb cb, void *data);
eio_req *eio_read      (int fd, void *buf, size_t length, off_t offset, int pri, eio_cb cb, void *data);
eio_req *eio_write     (int fd, void *buf, size_t length, off_t offset, int pri, eio_cb cb, void *data);
eio_req *eio_fstat     (int fd, int pri, eio_cb cb, void *data); /* stat buffer=ptr2 allocated dynamically */
eio_req *eio_futime    (int fd, eio_tstamp atime, eio_tstamp mtime, int pri, eio_cb cb, void *data);
eio_req *eio_ftruncate (int fd, off_t offset, int pri, eio_cb cb, void *data);
eio_req *eio_fchmod    (int fd, mode_t mode, int pri, eio_cb cb, void *data);
eio_req *eio_fchown    (int fd, uid_t uid, gid_t gid, int pri, eio_cb cb, void *data);
eio_req *eio_dup2      (int fd, int fd2, int pri, eio_cb cb, void *data);
eio_req *eio_sendfile  (int out_fd, int in_fd, off_t in_offset, size_t length, int pri, eio_cb cb, void *data);
eio_req *eio_open      (const char *path, int flags, mode_t mode, int pri, eio_cb cb, void *data);
eio_req *eio_utime     (const char *path, eio_tstamp atime, eio_tstamp mtime, int pri, eio_cb cb, void *data);
eio_req *eio_truncate  (const char *path, off_t offset, int pri, eio_cb cb, void *data);
eio_req *eio_chown     (const char *path, uid_t uid, gid_t gid, int pri, eio_cb cb, void *data);
eio_req *eio_chmod     (const char *path, mode_t mode, int pri, eio_cb cb, void *data);
eio_req *eio_mkdir     (const char *path, mode_t mode, int pri, eio_cb cb, void *data);
eio_req *eio_readdir   (const char *path, int pri, eio_cb cb, void *data); /* result=ptr2 allocated dynamically */
eio_req *eio_rmdir     (const char *path, int pri, eio_cb cb, void *data);
eio_req *eio_unlink    (const char *path, int pri, eio_cb cb, void *data);
eio_req *eio_readlink  (const char *path, int pri, eio_cb cb, void *data); /* result=ptr2 allocated dynamically */
eio_req *eio_stat      (const char *path, int pri, eio_cb cb, void *data); /* stat buffer=ptr2 allocated dynamically */
eio_req *eio_lstat     (const char *path, int pri, eio_cb cb, void *data); /* stat buffer=ptr2 allocated dynamically */
eio_req *eio_mknod     (const char *path, mode_t mode, dev_t dev, int pri, eio_cb cb, void *data);
eio_req *eio_link      (const char *path, const char *new_path, int pri, eio_cb cb, void *data);
eio_req *eio_symlink   (const char *path, const char *new_path, int pri, eio_cb cb, void *data);
eio_req *eio_rename    (const char *path, const char *new_path, int pri, eio_cb cb, void *data);
eio_req *eio_custom    (eio_cb execute, int pri, eio_cb cb, void *data);
#endif

/*****************************************************************************/
/* groups */

eio_req *eio_grp       (eio_cb cb, void *data);
void eio_grp_feed      (eio_req *grp, void (*feed)(eio_req *req), int limit);
void eio_grp_limit     (eio_req *grp, int limit);
void eio_grp_add       (eio_req *grp, eio_req *req);
void eio_grp_cancel    (eio_req *grp); /* cancels all sub requests but not the group */

/*****************************************************************************/
/* request api */

/* true if the request was cancelled, useful in the invoke callback */
#define EIO_CANCELLED(req) ((req)->flags & EIO_FLAG_CANCELLED)

#define EIO_RESULT(req)    ((req)->result)
/* returns a pointer to the result buffer allocated by eio */
#define EIO_BUF(req)       ((req)->ptr2)
#define EIO_STAT_BUF(req)  ((EIO_STRUCT_STAT *)EIO_BUF(req))
#define EIO_PATH(req)      ((char *)(req)->ptr1)

/* submit a request for execution */
void eio_submit (eio_req *req);
/* cancel a request as soon fast as possible, if possible */
void eio_cancel (eio_req *req);
/* destroy a request that has never been submitted */
void eio_destroy (eio_req *req);

/*****************************************************************************/
/* convinience functions */

ssize_t eio_sendfile_sync (int ofd, int ifd, off_t offset, size_t count);

#ifdef __cplusplus
}
#endif

#endif

