/*
 * Copyright (c) 2000 Silicon Graphics, Inc.; provided copyright in
 * certain portions may be held by third parties as indicated herein.
 * All Rights Reserved.
 *
 * The code in this source file represents an aggregation of work from
 * Georgia Tech, Fred Fish, Jeff Lee, Arnold Robbins and other Silicon
 * Graphics engineers over the period 1985-2000.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 * 
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 * 
 * http://www.sgi.com 
 * 
 * For further information regarding this notice, see: 
 * 
 * http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
 */
#ident	"$Header: /proj/irix6.5m-melb/isms/eoe/lib/librmt/src/RCS/rmtioctl.c,v 1.5 1995/03/15 17:55:47 tap Exp $"

#include <errno.h>

#include "rmtlib.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/param.h>

struct  irix_mtget   {
        short   mt_type;          /* type of magtape device */
        unsigned short  mt_dposn; /* status of tape position */
        unsigned short  mt_dsreg; /* ``drive status'' register */
        short   mt_erreg;         /* ``error'' register */
        short   mt_resid;         /* residual count */
	short   pad;
        int     mt_fileno;        /* file number of current position */
        int     mt_blkno;         /* block number of current position */
};

/* IRIX tape device status values */
#define IRIX_MT_EOT          0x01    /* tape is at end of media */
#define IRIX_MT_BOT          0x02    /* tape is at beginning of media */
#define IRIX_MT_WPROT        0x04    /* tape is write-protected */
#define IRIX_MT_EW           0x08    /* hit early warning marker     */
#define IRIX_MT_ONL          0x40    /* drive is online */
#define IRIX_MT_EOD          0x4000  /* tape is at end of data */
#define IRIX_MT_FMK          0x8000  /* tape is at file mark */

/* IRIX mt operations (mt_op values for MTIOCTOP) */
#define IRIX_MTWEOF  0       /* write an end-of-file record */
#define IRIX_MTFSF   1       /* forward space file */
#define IRIX_MTBSF   2       /* backward space file */
#define IRIX_MTFSR   3       /* forward space record */
#define IRIX_MTBSR   4       /* backward space record */
#define IRIX_MTREW   5       /* rewind */
#define IRIX_MTOFFL  6       /* rewind and put the drive offline */
#define IRIX_MTERASE 12      /* erase tape from current position to EOT */
#define IRIX_MTUNLOAD 13     /* unload tape from drive */

/* std (common) mt op codes */
#define STD_MTWEOF  0       /* write an end-of-file record */
#define STD_MTFSF   1       /* forward space file */
#define STD_MTBSF   2       /* backward space file */
#define STD_MTFSR   3       /* forward space record */
#define STD_MTBSR   4       /* backward space record */
#define STD_MTREW   5       /* rewind */
#define STD_MTOFFL  6       /* rewind and put the drive offline */

#define MT_MAX 40 /* encompass potential range of mt_op values */
static int mtop_irixmap[MT_MAX] = {-1};
static int mtop_stdmap[MT_MAX] = {-1};

static void
init_mtop_map(void)
{
    /* set all other values to sentinel (-1) */

    /* only map the ones which xfsdump/restore are interested in */
    mtop_irixmap[MTWEOF]    = IRIX_MTWEOF;
    mtop_irixmap[MTFSF]     = IRIX_MTFSF;
    mtop_irixmap[MTBSF]     = IRIX_MTBSF;
    mtop_irixmap[MTFSR]     = IRIX_MTFSR;
    mtop_irixmap[MTBSR]     = IRIX_MTBSR;
    mtop_irixmap[MTREW]     = IRIX_MTREW;
    mtop_irixmap[MTOFFL]    = IRIX_MTOFFL;
    mtop_irixmap[MTERASE]   = IRIX_MTERASE;
    mtop_irixmap[MTUNLOAD]  = IRIX_MTUNLOAD;

    mtop_stdmap[MTWEOF]    = STD_MTWEOF;
    mtop_stdmap[MTFSF]     = STD_MTFSF;
    mtop_stdmap[MTBSF]     = STD_MTBSF;
    mtop_stdmap[MTFSR]     = STD_MTFSR;
    mtop_stdmap[MTBSR]     = STD_MTBSR;
    mtop_stdmap[MTREW]     = STD_MTREW;
    mtop_stdmap[MTOFFL]    = STD_MTOFFL;
    mtop_stdmap[MTUNLOAD]  = STD_MTOFFL;
}


static int _rmt_ioctl(int, int, char *);

/*
 *	Do ioctl on file.  Looks just like ioctl(2) to caller.
 */
 
int rmtioctl (fildes, request, arg)
int fildes, request, arg;
{
	if (isrmt (fildes))
	{
		return (_rmt_ioctl (fildes - REM_BIAS, request, (char *)arg));
	}
	else
	{
		return (ioctl (fildes, request, arg));
	}
}


/*
 *	_rmt_ioctl --- perform raw tape operations remotely
 */

/*
 * WARNING: MTIOCGET code is highly dependent on the format
 *          of mtget on different platforms
 *          We only support Linux or IRIX for this case. 
 *          We use the result of uname(1) (in rmtopen()) if it works or
 *          the size of the mtget structure to determine which host it is.
 */

static int
_rmt_ioctl(int fildes, int op, char *arg)
{
	char buffer[BUFMAGIC];
	int rc, cnt, ssize;
	char *p = NULL, *irixget = NULL;
	struct irix_mtget irix_mtget;
	static int onetrip = 0;

	if (!onetrip) {
		onetrip = 1; 
		init_mtop_map();
	}

/*
 *	MTIOCTOP is the easy one. nothing is transfered in binary
 */

	if (op == MTIOCTOP)
	{
		int mt_op = ((struct mtop *) arg)->mt_op;
		int mt_count = ((struct mtop *) arg)->mt_count;

		/* ensure we know what host we reckon we have */
		/* We must do a MTIOCGET for this */
		if (RMTHOST(fildes) == UNAME_UNDEFINED) {
			struct mtget mtget;
			int sts;
			
			if ((sts = _rmt_ioctl(fildes, MTIOCGET, (char*)&mtget)) < 0) 
				return sts;
		}

		/* map the linux op code to the irix op code */
		if (RMTHOST(fildes) == UNAME_IRIX) {
			mt_op = mtop_irixmap[mt_op];
			if (mt_op == -1) {
			    setoserror( EINVAL );
			    return(-1);
			}
		}
		else if (RMTHOST(fildes) != UNAME_LINUX) {
		/* map the linux op code to the standard/fallback op code */
			mt_op = mtop_stdmap[mt_op];
			if (mt_op == -1) {
			    setoserror( EINVAL );
			    return(-1);
			}
		}

			

		sprintf(buffer, "I%d\n%d\n", mt_op, mt_count);
		if (_rmt_command(fildes, buffer) == -1)
		{
			return(-1);
		}
		return(_rmt_status(fildes));
	}
        else if (op == MTIOCGET) {

		/*
		 *  Grab the status and read it directly into the structure.
		 *  Since the data is binary data, and the other machine might
		 *  be IRIX or Linux of a different byte-order,
		 *  we have to be careful in converting the data. 
		 *
		 *  NOTE: the original /etc/rmt did NOT support a newline after
		 *  the S command, and Sun still does not.  Neither does the
		 *  current bsd source, all the way through the tahoe release.
		 *  So do NOT add the \n to this!  The sgi rmt command will
		 *  work either way.  Olson, 4/91
		 */
		if (_rmt_command(fildes, "S") == -1 ||
		    (rc = _rmt_status(fildes)) == -1)
			return(-1);


		/* If undefined then try and define it by looking
		 * and the size of the get structure.
		 * If we know our rmt host, then verify that the
		 * structure is the correct size for the supported ones
		 */ 
 		switch (RMTHOST(fildes)) {
		    case UNAME_UNDEFINED:
			RMTDEBUG("rmt: determining host from mtget size\n");
			if (sizeof(struct mtget) == rc) {
				RMTHOST(fildes) = UNAME_LINUX;
			}
			else if (sizeof(struct irix_mtget) == rc) {
				RMTHOST(fildes) = UNAME_IRIX;
			}
			else {
			    RMTDEBUG("rmt: no matching mtget structure\n");
			    /* DUNNO */
			    setoserror( EPROTONOSUPPORT );
			    return(-1);
			}
			break;
		    case UNAME_IRIX:
			if (sizeof(struct irix_mtget) != rc) {
			    RMTDEBUG("rmt: IRIX mtget structure of wrong size\n");
			    setoserror( EPROTONOSUPPORT );
			    return(-1);
			}
			break;
		    case UNAME_LINUX:
			if (sizeof(struct mtget) != rc) {
			    RMTDEBUG("rmt: Linux mtget structure of wrong size\n");
			    setoserror( EPROTONOSUPPORT );
			    return(-1);
			}
			break;
		    default:
			setoserror( EPROTONOSUPPORT );
			return(-1);
		}


		assert(RMTHOST(fildes)==UNAME_LINUX || RMTHOST(fildes)==UNAME_IRIX);


		if (RMTHOST(fildes) == UNAME_LINUX) 
		    p = arg;
		else
		    p = irixget = (char *)&irix_mtget;


		/* read in all the data */
		ssize = rc;
		for (; ssize > 0; ssize -= cnt, p += cnt)
		{
			cnt = read(READ(fildes), p, ssize);
			if (cnt <= 0)
			{
				_rmt_abort(fildes);
				setoserror( EIO );
				return(-1);
			}
		}


		/*
		 * May need to byteswap
		 */
		if (RMTHOST(fildes) == UNAME_IRIX) {
			struct irix_mtget *irixp = (struct irix_mtget *)irixget;

			if (irixp->mt_type > 0xff) {
			    /* assume that mt_type should fit in 1 byte */ 

			    irixp->mt_type   = INT_SWAP(irixp->mt_type, irixp->mt_type);	
			    irixp->mt_dsreg  = INT_SWAP(irixp->mt_dsreg, irixp->mt_dsreg);
			    irixp->mt_erreg  = INT_SWAP(irixp->mt_erreg, irixp->mt_erreg);
			    irixp->mt_resid  = INT_SWAP(irixp->mt_resid, irixp->mt_resid);
			    irixp->mt_fileno = INT_SWAP(irixp->mt_fileno, irixp->mt_fileno);
			    irixp->mt_blkno  = INT_SWAP(irixp->mt_blkno, irixp->mt_blkno);
			    irixp->mt_dposn  = INT_SWAP(irixp->mt_dposn, irixp->mt_dposn);
			}
		}
		else { /* LINUX */
			struct mtget *linuxp = (struct mtget *)arg;

			if (linuxp->mt_type > 0xffff) {
			    /* assume that mt_type should fit in 2 bytes */ 

			    linuxp->mt_type   = INT_SWAP(linuxp->mt_type, linuxp->mt_type);	
			    linuxp->mt_dsreg  = INT_SWAP(linuxp->mt_dsreg, linuxp->mt_dsreg);
			    linuxp->mt_erreg  = INT_SWAP(linuxp->mt_erreg, linuxp->mt_erreg);
			    linuxp->mt_resid  = INT_SWAP(linuxp->mt_resid, linuxp->mt_resid);
			    linuxp->mt_fileno = INT_SWAP(linuxp->mt_fileno, linuxp->mt_fileno);
			    linuxp->mt_blkno  = INT_SWAP(linuxp->mt_blkno, linuxp->mt_blkno);
			    linuxp->mt_gstat  = INT_SWAP(linuxp->mt_gstat, linuxp->mt_gstat);

			}
		}

		/* 
		 * now mtget has the correct (byte-swapped if needed)
		 * data, if we are talking to irix then lets convert
		 * the data into something that we can use
		 * else all done
		 */
		if (RMTHOST(fildes) == UNAME_IRIX) {
			struct mtget *dstp = (struct mtget *)arg;
			struct irix_mtget *srcp = (struct irix_mtget *)irixget;
			short status = srcp->mt_dsreg;

			dstp->mt_type = srcp->mt_type;
			dstp->mt_erreg = srcp->mt_erreg;
			dstp->mt_resid = srcp->mt_resid;
			dstp->mt_fileno = srcp->mt_fileno;
			dstp->mt_blkno = srcp->mt_blkno;
			dstp->mt_dsreg = srcp->mt_dsreg; /* different semantics */

			/* need to do tape status conversions */
			dstp->mt_gstat = 0;
			if (status & IRIX_MT_EOT)
			    dstp->mt_gstat |= GMT_EOT(0xffffffff);
			if (status & IRIX_MT_BOT)
			    dstp->mt_gstat |= GMT_BOT(0xffffffff);
			if (status & IRIX_MT_WPROT)
			    dstp->mt_gstat |= GMT_WR_PROT(0xffffffff);
			if (status & IRIX_MT_ONL)
			    dstp->mt_gstat |= GMT_ONLINE(0xffffffff);
			if (status & IRIX_MT_EOD)
			    dstp->mt_gstat |= GMT_EOD(0xffffffff);
			if (status & IRIX_MT_FMK)
			    dstp->mt_gstat |= GMT_EOF(0xffffffff);
			if (status & IRIX_MT_EW)
			    ;/* No GMT_ to map it to */
		}

		return(0);

	}
        else {
	    setoserror( EINVAL );
	    return(-1);
	}
}
