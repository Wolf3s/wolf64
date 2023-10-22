#ifndef __WL_DATA_H_
#define __WL_DATA_H_

#include "version.h"

#ifndef SPEAR
    #include "audiowl6.h"
    #ifdef UPLOAD
        #include "gfxv_apo.h"
    #else
		#ifdef JAPAN
			#include "gfxv_jap.h"
		#else
			#ifdef GOODTIMES
	            #include "gfxv_wl6.h"
		    #else
			    #include "gfxv_apo.h"
			#endif
        #endif
    #endif
#else
    #include "audiosod.h"
    #include "gfxv_sod.h"
    #include "f_spear.h"
#endif

#endif
