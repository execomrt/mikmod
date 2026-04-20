/*	MikMod sound library
	(c) 1998, 1999 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  Dynamic memory routines

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_POSIX_MEMALIGN
#define _XOPEN_SOURCE 600 /* for posix_memalign */
#endif

#include "string.h"
#include "mikmod_internals.h"

#if defined(MIKMOD_SIMD)

#undef WIN32_ALIGNED_MALLOC

#if defined(_WIN32) && !defined(_WIN32_WCE)
# if defined(_WIN64)
#  define WIN32_ALIGNED_MALLOC 1
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define WIN32_ALIGNED_MALLOC 1
# endif
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MIKMOD_SIMD_ALIGNMENT 16

void* MikMod_amalloc(size_t size)
{
	void* p = NULL;

	if (size == 0)
		size = 1;

#if defined(HAVE_POSIX_MEMALIGN)

	if (posix_memalign(&p, MIKMOD_SIMD_ALIGNMENT, size) == 0) {
		memset(p, 0, size);
		return p;
	}

#elif defined(WIN32_ALIGNED_MALLOC)

	p = _aligned_malloc(size, MIKMOD_SIMD_ALIGNMENT);
	if (p) {
		memset(p, 0, size);
		return p;
	}

#else
	{
		const size_t header = sizeof(uintptr_t);
		const size_t extra = MIKMOD_SIMD_ALIGNMENT - 1 + header;
		unsigned char* base = (unsigned char*)calloc(1, size + extra);

		if (base) {
			uintptr_t raw = (uintptr_t)(base + header);
			uintptr_t aligned = (raw + (MIKMOD_SIMD_ALIGNMENT - 1)) &
				~((uintptr_t)(MIKMOD_SIMD_ALIGNMENT - 1));
			unsigned char* p2 = (unsigned char*)aligned;

			((uintptr_t*)p2)[-1] = (uintptr_t)base;
			return p2;
		}
	}
#endif

	_mm_errno = MMERR_OUT_OF_MEMORY;
	if (_mm_errorhandler) _mm_errorhandler();
	return NULL;
}

void MikMod_afree(void* data)
{
	if (!data)
		return;

#if defined(HAVE_POSIX_MEMALIGN)

	free(data);

#elif defined(WIN32_ALIGNED_MALLOC)

	_aligned_free(data);

#else

	free((void*)((uintptr_t*)data)[-1]);

#endif
}

#endif /* MIKMOD_SIMD */

void* MikMod_realloc(void *data, size_t size)
{
	if (data) return realloc(data, size);
	return calloc(1, size);
}

/* Same as malloc, but sets error variable _mm_error when fails */
void* MikMod_malloc(size_t size)
{
	void *d = malloc(size);
	if (d) return d;

	_mm_errno = MMERR_OUT_OF_MEMORY;
	if(_mm_errorhandler) _mm_errorhandler();
	return NULL;
}

/* Same as calloc, but sets error variable _mm_error when fails */
void* MikMod_calloc(size_t nitems, size_t size)
{
	void *d = calloc(nitems, size);
	if (d) return d;

	_mm_errno = MMERR_OUT_OF_MEMORY;
	if(_mm_errorhandler) _mm_errorhandler();
	return NULL;
}

void MikMod_free(void *data)
{
	if (data) free(data);
}

/* like strdup(), but the result must be freed using MikMod_free() */
CHAR *MikMod_strdup(const CHAR *s)
{
	size_t l;
	CHAR *d;

	if (!s) return NULL;

	l = strlen(s) + 1;
	d = (CHAR *) MikMod_malloc(l);
	if (d) memcpy(d, s, l);
	return d;
}

/* ex:set ts=4: */
