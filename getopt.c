/*  SSNES - A Super Nintendo Entertainment System (SNES) Emulator frontend for libsnes.
 *  Copyright (C) 2010-2011 - Hans-Kristian Arntzen
 *
 *  Some code herein may be based on code found in BSNES.
 * 
 *  SSNES is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  SSNES is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with SSNES.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "getopt_ssnes.h"

#ifndef HAVE_GETOPT_LONG

#include <string.h>
#include "boolean.h"
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include "msvc/msvc_compat.h"

char *optarg;
int optind, opterr, optopt;

static bool is_short_option(const char *str)
{
   return str[0] == '-' && str[1] != '-';
}

static bool is_long_option(const char *str)
{
   return str[0] == '-' && str[1] == '-';
}

static int find_short_index(char * const *argv)
{
   for (int index = 0; argv[index]; index++)
   {
      if (is_short_option(argv[index]))
         return index;
   }

   return -1;
}

static int find_long_index(char * const *argv)
{
   for (int index = 0; argv[index]; index++)
   {
      if (is_long_option(argv[index]))
         return index;
   }

   return -1;
}

static int parse_short(const char *optstring, char * const *argv)
{
   char arg = argv[0][1];
   if (arg == ':')
      return '?';

   const char *opt = strchr(optstring, arg);
   if (!opt)
      return '?';

   bool multiple_opt = argv[0][2];
   bool takes_arg = opt[1] == ':';
   if (multiple_opt && takes_arg) // Makes little sense to allow.
      return '?';

   if (takes_arg)
   {
      optarg = argv[1];
      optind += 2;
      return optarg ? opt[0] : '?';
   }
   else if (multiple_opt)
   {
      memmove(&argv[0][1], &argv[0][2], strlen(&argv[0][2]) + 1);
      return opt[0];
   }
   else
   {
      optind++;
      return opt[0];
   }
}

static int parse_long(const struct option *longopts, char * const *argv)
{
   const struct option *opt = NULL;
   for (size_t indice = 0; longopts[indice].name; indice++)
   {
      if (strcmp(longopts[indice].name, &argv[0][2]) == 0)
      {
         opt = &longopts[indice];
         break;
      }
   }

   if (!opt)
      return '?';
   
   // getopt_long has an "optional" arg, but we don't bother with that.
   if (opt->has_arg && !argv[1])
      return '?';

   if (opt->has_arg)
   {
      optarg = argv[1];
      optind += 2;
   }
   else
      optind++;

   if (opt->flag)
   {
      *opt->flag = opt->val;
      return 0;
   }
   else
      return opt->val;
}

static void shuffle_block(char **begin, char **last, char **end)
{
   ptrdiff_t len = last - begin;
   const char **tmp = (const char**)calloc(len, sizeof(const char*));
   assert(tmp);

   memcpy(tmp, begin, sizeof(tmp));
   memmove(begin, last, (end - last) * sizeof(char*));
   memcpy(end - len, tmp, sizeof(tmp));

   free(tmp);
}

int getopt_long(int argc, char *argv[],
      const char *optstring, const struct option *longopts, int *longindex)
{
   (void)longindex;
   if (argc == 1)
      return -1;

   if (optind == 0)
      optind = 1;

   int short_index = find_short_index(&argv[optind]);
   int long_index  = find_long_index(&argv[optind]);

   // We're done here.
   if (short_index == -1 && long_index == -1)
      return -1;

   // Reorder argv so that non-options come last.
   // Non-POSIXy, but that's what getopt does by default.
   if ((short_index > 0) && ((short_index < long_index) || (long_index == -1)))
   {
      shuffle_block(&argv[optind], &argv[optind + short_index], &argv[argc]);
      short_index = 0;
   }
   else if ((long_index > 0) && ((long_index < short_index) || (short_index == -1)))
   {
      shuffle_block(&argv[optind], &argv[optind + long_index], &argv[argc]);
      long_index = 0;
   }

   assert(short_index == 0 || long_index == 0);

   if (short_index == 0)
      return parse_short(optstring, &argv[optind]);
   else if (long_index == 0)
      return parse_long(longopts, &argv[optind]);
   else
      return '?';
}

#endif

