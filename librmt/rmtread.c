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
#ident	"$Header: /proj/irix6.5m-melb/isms/eoe/lib/librmt/src/RCS/rmtread.c,v 1.3 1995/03/15 17:56:11 tap Exp $"

#include <errno.h>

#include "rmtlib.h"

static int _rmt_read(int, char *, unsigned int);

/*
 *	Read from stream.  Looks just like read(2) to caller.
 */
  
int rmtread (fildes, buf, nbyte)
int fildes;
char *buf;
unsigned int nbyte;
{
	if (isrmt (fildes))
	{
		return (_rmt_read (fildes - REM_BIAS, buf, nbyte));
	}
	else
	{
		return (read (fildes, buf, nbyte));
	}
}


/*
 *	_rmt_read --- read a buffer from a remote tape
 */

static int _rmt_read(int fildes, char *buf, unsigned int nbyte)
{
	int rc, i;
	char buffer[BUFMAGIC];

	sprintf(buffer, "R%d\n", nbyte);
	if (_rmt_command(fildes, buffer) == -1 || (rc = _rmt_status(fildes)) == -1)
		return(-1);

	for (i = 0; i < rc; i += nbyte, buf += nbyte)
	{
		nbyte = read(READ(fildes), buf, rc);
		if (nbyte <= 0)
		{
			_rmt_abort(fildes);
			setoserror(EIO); 
	 		return(-1);
		}
	}

	return(rc);
}
