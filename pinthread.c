/*
 * Copyright (c) 2015 Franco (nextime) Lanza <nextime@nexlab.it>
 *
 * pinthread [https://git.devuan.org/packages-base/pinthread]
 *
 * pinthread is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *************************************************************************************
 *
 * compile with :
 * $ gcc -Wall -D_GNU_SOURCE -fpic -shared -o pinthread.so pinthread.c -ldl -lpthread
 *
 * and test it with :
 * LD_PRELOAD=/path/to/pinthread.so  something
 * 
 * Environment variables:
 *   - PINTHREAD_CORE: set to the core number you want to pin to.
 *                     if omitted, it defaults to the result of 
 *                     sched_getcpu()
 *
 *   - PINTHREAD_PNAMES: accept a space separated list of strings,
 *                       if any string match the process name or
 *                       PINTHREAD_PNAMES is empty/omitted, it will 
 *                       pin all thread according to PINTHREAD_CORE, 
 *                       otherwise it will just call the "real" 
 *                       pthread_create without any pinning.
 *
 */

//#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h> 
#include <dlfcn.h>
#include <sched.h>   //cpu_set_t , CPU_SET
#include <libgen.h>
#include <stdbool.h>
#include <string.h>

static char *procname;
static bool pinthread_override = false;
static unsigned int ncore;
static unsigned int setcore;

static int (*real_pthread_create)(pthread_t  *thread,
                          const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);

static void main_pinthread(int argc, char* argv[], char* envp[])
{
   char *pch;
   char *msg;
   cpu_set_t mask;        /* Define your cpu_set bit mask. */
   CPU_ZERO(&mask);       /* Initialize it all to 0, i.e. no CPUs selected. */

   procname = basename(argv[0]);
    
   msg = getenv("PINTHREAD_PNAMES"); 
   if(msg == NULL)
   {
      pinthread_override = true;
   } else {
      pch = strtok (msg," ");
      while (pch != NULL)
      {
         if (!strcmp(procname,pch))
         {
            pinthread_override = true;
            pch = NULL;
         } else {
            pch = strtok (NULL," ");
         }
      }
   }  

   ncore = sysconf (_SC_NPROCESSORS_CONF);
   msg = getenv("PINTHREAD_CORE");
   if (msg != NULL)
   {
      setcore = (unsigned int) strtoul(msg, (char **)NULL, 10);
      if(setcore >= ncore)
      {
         fprintf(stderr, "E:PINTHREAD wrong value for PINTHREAD_CORE: %u - using default.\n", setcore);
         setcore = sched_getcpu();
      } 
   } else {
      setcore = sched_getcpu();
   } 
  
   CPU_SET(setcore, &mask);
 
   // make sure the main thread is running on the same core:
   sched_setaffinity(getpid(), sizeof(mask), &mask);

   real_pthread_create = dlsym(RTLD_NEXT,"pthread_create");
   if ((msg=dlerror())!=NULL)
      fprintf(stderr, "E:PINTHREAD pthread_create dlsym failed : %s\n", msg);
  //printf("*wrapping done\n");

}

__attribute__((section(".init_array"))) void (* p_main_pinthread)(int,char*[],char*[]) = &main_pinthread;


int pthread_create(pthread_t  *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg)
{
  int ret;
  cpu_set_t mask;        /* Define your cpu_set bit mask. */
  CPU_ZERO(&mask);       /* Initialize it all to 0, i.e. no CPUs selected. */
  CPU_SET(setcore, &mask);

  //printf("*about to call original pthread_create\n");
  ret = real_pthread_create(thread,attr,start_routine,arg);
  if(pinthread_override)
     pthread_setaffinity_np(*thread, sizeof(mask), &mask);
  return ret;
}




