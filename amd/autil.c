/*
 * Copyright (c) 1997-2003 Erez Zadok
 * Copyright (c) 1990 Jan-Simon Pendry
 * Copyright (c) 1990 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * $Id: autil.c,v 1.26 2002/12/27 22:43:48 ezk Exp $
 *
 */

/*
 * utilities specified to amd, taken out of the older amd/util.c.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

int NumChild = 0;		/* number of children of primary amd */
static char invalid_keys[] = "\"'!;@ \t\n";

#ifdef HAVE_TRANSPORT_TYPE_TLI
# define PARENT_USLEEP_TIME	100000 /* 0.1 seconds */
#endif /* HAVE_TRANSPORT_TYPE_TLI */


char *
strealloc(char *p, char *s)
{
  int len = strlen(s) + 1;

  p = (char *) xrealloc((voidp) p, len);

  strcpy(p, s);
#ifdef DEBUG_MEM
  malloc_verify();
#endif /* DEBUG_MEM */
  return p;
}


char **
strsplit(char *s, int ch, int qc)
{
  char **ivec;
  int ic = 0;
  int done = 0;

  ivec = (char **) xmalloc((ic + 1) * sizeof(char *));

  while (!done) {
    char *v;

    /*
     * skip to split char
     */
    while (*s && (ch == ' ' ? (isascii(*s) && isspace((int)*s)) : *s == ch))
      *s++ = '\0';

    /*
     * End of string?
     */
    if (!*s)
      break;

    /*
     * remember start of string
     */
    v = s;

    /*
     * skip to split char
     */
    while (*s && !(ch == ' ' ? (isascii(*s) && isspace((int)*s)) : *s == ch)) {
      if (*s++ == qc) {
	/*
	 * Skip past string.
	 */
	s++;
	while (*s && *s != qc)
	  s++;
	if (*s == qc)
	  s++;
      }
    }

    if (!*s)
      done = 1;
    *s++ = '\0';

    /*
     * save string in new ivec slot
     */
    ivec[ic++] = v;
    ivec = (char **) xrealloc((voidp) ivec, (ic + 1) * sizeof(char *));
    if (amuDebug(D_STR))
      plog(XLOG_DEBUG, "strsplit saved \"%s\"", v);
  }

  if (amuDebug(D_STR))
    plog(XLOG_DEBUG, "strsplit saved a total of %d strings", ic);

  ivec[ic] = 0;

  return ivec;
}


/*
 * Strip off the trailing part of a domain
 * to produce a short-form domain relative
 * to the local host domain.
 * Note that this has no effect if the domain
 * names do not have the same number of
 * components.  If that restriction proves
 * to be a problem then the loop needs recoding
 * to skip from right to left and do partial
 * matches along the way -- ie more expensive.
 */
static void
domain_strip(char *otherdom, char *localdom)
{
  char *p1, *p2;

  if ((p1 = strchr(otherdom, '.')) &&
      (p2 = strchr(localdom, '.')) &&
      STREQ(p1 + 1, p2 + 1))
    *p1 = '\0';
}


/*
 * Normalize a host name
 */
void
host_normalize(char **chp)
{
  /*
   * Normalize hosts is used to resolve host name aliases
   * and replace them with the standard-form name.
   * Invoked with "-n" command line option.
   */
  if (gopt.flags & CFM_NORMALIZE_HOSTNAMES) {
    struct hostent *hp;
    clock_valid = 0;
    hp = gethostbyname(*chp);
    if (hp && hp->h_addrtype == AF_INET) {
      dlog("Hostname %s normalized to %s", *chp, hp->h_name);
      *chp = strealloc(*chp, (char *) hp->h_name);
    }
  }
  domain_strip(*chp, hostd);
}



/*
 * Keys are not allowed to contain " ' ! or ; to avoid
 * problems with macro expansions.
 */
int
valid_key(char *key)
{
  while (*key)
    if (strchr(invalid_keys, *key++))
      return FALSE;
  return TRUE;
}


void
forcibly_timeout_mp(am_node *mp)
{
  mntfs *mf = mp->am_mnt;
  /*
   * Arrange to timeout this node
   */
  if (mf && ((mp->am_flags & AMF_ROOT) ||
	     (mf->mf_flags & (MFF_MOUNTING | MFF_UNMOUNTING)))) {
    if (mf->mf_flags & MFF_UNMOUNTING)
      plog(XLOG_WARNING, "node %s is currently being unmounted, ignoring timeout request", mp->am_path);
    else
      plog(XLOG_WARNING, "ignoring timeout request for active node %s", mp->am_path);
  } else {
    plog(XLOG_INFO, "\"%s\" forcibly timed out", mp->am_path);
    mp->am_flags &= ~AMF_NOTIMEOUT;
    mp->am_ttl = clocktime();
    reschedule_timeout_mp();
  }
}


void
mf_mounted(mntfs *mf)
{
  int quoted;
  int wasmounted = mf->mf_flags & MFF_MOUNTED;

  if (!wasmounted) {
    /*
     * If this is a freshly mounted
     * filesystem then update the
     * mntfs structure...
     */
    mf->mf_flags |= MFF_MOUNTED;
    mf->mf_error = 0;

    /*
     * Do mounted callback
     */
    if (mf->mf_ops->mounted) {
      (*mf->mf_ops->mounted) (mf);
    }

    mf->mf_fo = 0;
  }

  /*
   * Log message
   */
  quoted = strchr(mf->mf_info, ' ') != 0;
  plog(XLOG_INFO, "%s%s%s %s fstype %s on %s",
       quoted ? "\"" : "",
       mf->mf_info,
       quoted ? "\"" : "",
       wasmounted ? "referenced" : "mounted",
       mf->mf_ops->fs_type, mf->mf_mount);
}


void
am_mounted(am_node *mp)
{
  mntfs *mf = mp->am_mnt;

  mf_mounted(mf);

#ifdef HAVE_FS_AUTOFS
  if (mf->mf_flags & MFF_AUTOFS)
    autofs_mounted(mp);
#endif /* HAVE_FS_AUTOFS */

  /*
   * Patch up path for direct mounts
   */
  if (mp->am_parent && mp->am_parent->am_mnt->mf_fsflags & FS_DIRECT)
    mp->am_path = str3cat(mp->am_path, mp->am_parent->am_path, "/", ".");

  /*
   * Check whether this mount should be cached permanently
   */
  if (mf->mf_fsflags & FS_NOTIMEOUT) {
    mp->am_flags |= AMF_NOTIMEOUT;
  } else if (mf->mf_mount[1] == '\0' && mf->mf_mount[0] == '/') {
    mp->am_flags |= AMF_NOTIMEOUT;
  } else {
    mntent_t mnt;
    if (mf->mf_mopts) {
      mnt.mnt_opts = mf->mf_mopts;
      if (amu_hasmntopt(&mnt, "nounmount"))
	mp->am_flags |= AMF_NOTIMEOUT;
      if ((mp->am_timeo = hasmntval(&mnt, "utimeout")) == 0)
	mp->am_timeo = gopt.am_timeo;
    }
  }

  /*
   * If this node is a symlink then
   * compute the length of the returned string.
   */
  if (mp->am_fattr.na_type == NFLNK)
    mp->am_fattr.na_size = strlen(mp->am_link ? mp->am_link : mp->am_mnt->mf_mount);

  /*
   * Record mount time
   */
  mp->am_fattr.na_mtime.nt_seconds = mp->am_stats.s_mtime = clocktime();
  new_ttl(mp);

  /*
   * Update mtime of parent node
   */
  if (mp->am_parent && mp->am_parent->am_mnt)
    mp->am_parent->am_fattr.na_mtime.nt_seconds = mp->am_stats.s_mtime;

  /*
   * This is ugly, but essentially unavoidable
   * Sublinks must be treated separately as type==link
   * when the base type is different.
   */
  if (mp->am_link && mp->am_mnt->mf_ops != &amfs_link_ops)
    amfs_link_ops.mount_fs(mp, mp->am_mnt);

#ifdef HAVE_FS_AUTOFS
  if (mp->am_flags & AMF_AUTOFS)
    autofs_mount_succeeded(mp);
  else
#endif /* HAVE_FS_AUTOFS */
    /*
     * Now, if we can, do a reply to our NFS client here
     * to speed things up.
     */
    nfs_quick_reply(mp, 0);

  /*
   * Update stats
   */
  amd_stats.d_mok++;
}


int
mount_node(am_node *mp)
{
  mntfs *mf = mp->am_mnt;
  int error = 0;

#ifdef HAVE_FS_AUTOFS
  if (mf->mf_flags & MFF_AUTOFS)
    mf->mf_autofs_fh = autofs_get_fh(mp);
#endif /* HAVE_FS_AUTOFS */

  mf->mf_flags |= MFF_MOUNTING;
  error = mf->mf_ops->mount_fs(mp, mf);

  /* do this again, it might have changed */
  mf = mp->am_mnt;
  if (error >= 0)
    mf->mf_flags &= ~MFF_MOUNTING;
  if (!error && !(mf->mf_fsflags & FS_MBACKGROUND))
    am_mounted(mp);

  return error;
}


void
am_unmounted(am_node *mp)
{
  mntfs *mf = mp->am_mnt;

  if (!foreground)		/* firewall - should never happen */
    return;

  /*
   * Do unmounted callback
   */
  if (mf->mf_ops->umounted)
    mf->mf_ops->umounted(mf);

  /*
   * This is ugly, but essentially unavoidable.
   * Sublinks must be treated separately as type==link
   * when the base type is different.
   */
  if (mp->am_link && mp->am_mnt->mf_ops != &amfs_link_ops)
    amfs_link_ops.umount_fs(mp, mp->am_mnt);

#ifdef HAVE_FS_AUTOFS
  if (mp->am_flags & AMF_AUTOFS)
    autofs_umount_succeeded(mp);
#endif /* HAVE_FS_AUTOFS */

  /*
   * Clean up any directories that were made
   *
   * If we remove the mount point of a pending mount, any queued access
   * to it will fail. So don't do it in that case.
   * Also don't do it if the refcount is > 1.
   */
  if (mf->mf_flags & MFF_MKMNT &&
      mf->mf_refc == 1 &&
      !(mp->am_flags & AMF_REMOUNT)) {
    plog(XLOG_INFO, "removing mountpoint directory '%s'", mf->mf_real_mount);
    rmdirs(mf->mf_real_mount);
  }

  /*
   * If this is a pseudo-directory then adjust the link count
   * in the parent
   */
  if (mp->am_parent && mp->am_fattr.na_type == NFDIR)
    --mp->am_parent->am_fattr.na_nlink;

  /*
   * Update mtime of parent node
   */
  if (mp->am_parent && mp->am_parent->am_mnt)
    mp->am_parent->am_fattr.na_mtime.nt_seconds = clocktime();

  if (mp->am_flags & AMF_REMOUNT) {
    char *fname = strdup(mp->am_name);
    am_node *mp_parent = mp->am_parent;
    int error = 0;

    free_map(mp);
    plog(XLOG_INFO, "am_unmounted: remounting %s", fname);
    mp = amfs_auto_lookup_child(mp_parent, fname, &error, VLOOK_CREATE);
    if (mp && error < 0)
      mp = amfs_auto_mount_child(mp, &error);
    if (error > 0) {
      errno = error;
      plog(XLOG_ERROR, "am_unmounted: could not remount %s: %m", fname);
    }
    XFREE(fname);
  } else
    free_map(mp);
}


/*
 * Fork the automounter
 *
 * TODO: Need a better strategy for handling errors
 */
static int
dofork(void)
{
  int pid;

top:
  pid = fork();

  if (pid < 0) {		/* fork error, retry in 1 second */
    sleep(1);
    goto top;
  }
  if (pid == 0) {		/* child process (foreground==false) */
    am_set_mypid();
    foreground = 0;
  } else {			/* parent process, has one more child */
    NumChild++;
  }

  return pid;
}


int
background(void)
{
  int pid = dofork();

  if (pid == 0) {
    dlog("backgrounded");
    foreground = 0;
  }
  return pid;
}
