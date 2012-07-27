/*
 * Copyright © 2012  Google, Inc.
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
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_SHAPER_PRIVATE_HH
#define HB_SHAPER_PRIVATE_HH

#include "hb-private.hh"

#include "hb-shape-plan.h" /* TODO remove */

typedef hb_bool_t hb_shape_func_t (hb_shape_plan_t    *shape_plan,
				   hb_font_t          *font,
				   hb_buffer_t        *buffer,
				   const hb_feature_t *features,
				   unsigned int        num_features);

#define HB_SHAPER_IMPLEMENT(name) \
	extern "C" HB_INTERNAL hb_shape_func_t _hb_##name##_shape;
#include "hb-shaper-list.hh"
#undef HB_SHAPER_IMPLEMENT

struct hb_shaper_pair_t {
  char name[16];
  hb_shape_func_t *func;
};

HB_INTERNAL const hb_shaper_pair_t *
_hb_shapers_get (void);


/* For embedding in face / font / ... */
struct hb_shaper_data_t {
#define HB_SHAPER_IMPLEMENT(shaper) void *shaper;
#include "hb-shaper-list.hh"
#undef HB_SHAPER_IMPLEMENT
};

#define HB_SHAPERS_COUNT (sizeof (hb_shaper_data_t) / sizeof (void *))

/* Means: succeeded, but don't need to keep any data. */
#define HB_SHAPER_DATA_SUCCEEDED ((void *) +1)

/* Means: tried but failed to create. */
#define HB_SHAPER_DATA_INVALID ((void *) -1)
#define HB_SHAPER_DATA_IS_INVALID(data) ((void *) (data) == HB_SHAPER_DATA_INVALID)

#define HB_SHAPER_DATA_TYPE(shaper, object)		struct hb_##shaper##_shaper_##object##_data_t
#define HB_SHAPER_DATA(shaper, object)			((HB_SHAPER_DATA_TYPE(shaper, object) *) (object)->shaper_data.shaper)
#define HB_SHAPER_DATA_GET_FUNC(shaper, object)		_hb_##shaper##_shaper_get_##object##_data
#define HB_SHAPER_DATA_CREATE_FUNC(shaper, object)	_hb_##shaper##_shaper_##object##_data_create
#define HB_SHAPER_DATA_DESTROY_FUNC(shaper, object)	_hb_##shaper##_shaper_##object##_data_destroy

#define HB_SHAPER_DATA_PROTOTYPE(shaper, object) \
	HB_SHAPER_DATA_TYPE (shaper, object); /* Type forward declaration. */ \
	static inline HB_SHAPER_DATA_TYPE (shaper, object) * \
	HB_SHAPER_DATA_GET_FUNC (shaper, object) (hb_##object##_t *object) \
	{ return HB_SHAPER_DATA (shaper, object); } \
	extern "C" HB_INTERNAL HB_SHAPER_DATA_TYPE (shaper, object) * \
	HB_SHAPER_DATA_CREATE_FUNC (shaper, object) (hb_##object##_t *object); \
	extern "C" HB_INTERNAL void \
	HB_SHAPER_DATA_DESTROY_FUNC (shaper, object) (HB_SHAPER_DATA_TYPE (shaper, object) *data)

#define HB_SHAPER_DATA_DESTROY(shaper, object) \
	if (object->shaper_data.shaper && \
	    object->shaper_data.shaper != HB_SHAPER_DATA_INVALID && \
	    object->shaper_data.shaper != HB_SHAPER_DATA_SUCCEEDED) \
	  HB_SHAPER_DATA_DESTROY_FUNC (shaper, object) (HB_SHAPER_DATA (shaper, object));


#endif /* HB_SHAPER_PRIVATE_HH */
