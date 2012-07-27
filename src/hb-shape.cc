/*
 * Copyright © 2009  Red Hat, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Red Hat Author(s): Behdad Esfahbod
 */

#include "hb-private.hh"

#include "hb-shaper-private.hh"
#include "hb-buffer-private.hh"



static const char **static_shaper_list;

static
void free_static_shaper_list (void)
{
  free (static_shaper_list);
}

const char **
hb_shape_list_shapers (void)
{
retry:
  const char **shaper_list = (const char **) hb_atomic_ptr_get (&static_shaper_list);

  if (unlikely (!shaper_list))
  {
    /* Not found; allocate one. */
    shaper_list = (const char **) calloc (1 + HB_SHAPERS_COUNT, sizeof (const char *));
    if (unlikely (!shaper_list)) {
      static const char *nil_shaper_list[] = {NULL};
      return nil_shaper_list;
    }

    const hb_shaper_pair_t *shapers = _hb_shapers_get ();
    unsigned int i;
    for (i = 0; i < HB_SHAPERS_COUNT; i++)
      shaper_list[i] = shapers[i].name;
    shaper_list[i] = NULL;

    if (!hb_atomic_ptr_cmpexch (&static_shaper_list, NULL, shaper_list)) {
      free (shaper_list);
      goto retry;
    }

#ifdef HAVE_ATEXIT
    atexit (free_static_shaper_list); /* First person registers atexit() callback. */
#endif
  }

  return shaper_list;
}


hb_bool_t
hb_shape_full (hb_font_t          *font,
	       hb_buffer_t        *buffer,
	       const hb_feature_t *features,
	       unsigned int        num_features,
	       const char * const *shaper_list)
{
  const hb_shaper_pair_t *shapers = _hb_shapers_get ();

  hb_font_make_immutable (font); /* So we can safely cache stuff on it */

  if (likely (!shaper_list)) {
    for (unsigned int i = 0; i < HB_SHAPERS_COUNT; i++)
      if (likely (shapers[i].func (NULL, font, buffer, features, num_features)))
        return true;
  } else {
    while (*shaper_list) {
      for (unsigned int i = 0; i < HB_SHAPERS_COUNT; i++)
	if (0 == strcmp (*shaper_list, shapers[i].name)) {
	  if (likely (shapers[i].func (NULL, font, buffer, features, num_features)))
	    return true;
	  break;
	}
      shaper_list++;
    }
  }
  return false;
}

void
hb_shape (hb_font_t           *font,
	  hb_buffer_t         *buffer,
	  const hb_feature_t  *features,
	  unsigned int         num_features)
{
  hb_shape_full (font, buffer, features, num_features, NULL);
}
