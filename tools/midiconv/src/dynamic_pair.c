/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    Simple Pairing on Dur_notes                           */
/*    winfried ritsch  (sept 2005-)                         */
/************************************************************/
/*
Note:
on modification flags for events and event data is changed,
but never deleted. This has to be done by storage functions
*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#ifndef NT
#include <unistd.h>
#endif

#include "mididefs.h"
#include "midifile.h"
#include "midiconv.h"


typedef struct _vel_index
{
    int vel;
    unsigned long index;
} VEL_INDEX;

static VEL_INDEX * sort_vel_index(DUR_NOTE *note_layer,unsigned long num);

/* ------ RULE 20 ----------------- make pair of N notes, sort by dynamic number ----------- */
int pair_dyn_notes(DUR_NOTE *notes,unsigned int n,unsigned int note_nr)
{
    unsigned int m;
    int velo;
    int vmean = 0;
    int vmax = 0;
    int vmin = 127;

    if(n<2)  /* save is save */
    {
        fprintf(stderr,"tried to pair less than 2 notes !!!");
        return 0;
    }

    /* calc velo min and max mean*/
    for(m=0; m<n; m++)
    {
        velo = notes[m].vel;
        vmean += velo;
        if(velo > vmax)vmax = velo;
        if(velo < vmin)vmin = velo;
    }
    vmean = (int) ((float) vmean / (float) n);

    velo = vmean; /* default pair_vel_scale = 0.5 */
    if(pair_vel_scale < 0.5)
        velo = (int) ((float) vmin  * (1.0 - (2.0*pair_vel_scale)) +
                      (float) vmean * (2.0*pair_vel_scale));
    else if(pair_vel_scale > 0.5)
        velo = (int) ((float) vmean  * (1.0 - (2.0*(pair_vel_scale-0.5))) +
                      (float) vmax * (2.0*(pair_vel_scale-0.5)));

    if(velo>127)velo=127;
    if(velo<1)velo=1;

    notes[0].vel = velo;

    for(m=1; m<n; m++)
    {
        notes[0].paired++;                         /* count paired notes */
        notes[0].dur = notes[m].time+notes[m].dur-notes[0].time;
        notes[m].del=1;                    /* del paired noteon */
        notes[m].paired++;                     /* reset paired from paired note */
    }
    if(vvv>1)
        printf("pair %2d notes %3d at %8.1f sec (dur %8.1f ms) to velo %3d\n",
               n,note_nr, ticks_to_time(notes[0].time)/1000.00,
               ticks_to_time(notes[0].dur),velo);

    return notes[0].paired;
}

/* pairs notes in one layer and pair them */
int pair_dyn_notes_layer(DUR_NOTE *note_layer,unsigned long num, int midinote)
{
    int vel1,vel2;
    unsigned long time1,time2;
    unsigned long i,j;
    unsigned long pm,n;
    DUR_NOTE *notes = note_layer;
    unsigned long paired = 0;
    VEL_INDEX *index_table = NULL;


    if(vvv>1)
        printf("do pairing Note %d using frametime %8.1f ms (%ld), pair_max %d:",
               midinote,frame_time,frame_ticks,pair_max);

    /* search in note list */
    index_table = sort_vel_index(note_layer,num);

    for(i=0;i < num;i++)
    {
        j = index_table[i].index;

        /* already paired */
        if(notes[j].paired)
            continue;

        /* for all notes use 0, then notecount num is used */
        pm = (unsigned long) pair_max;
        if(pair_max == 0)
            pm = num;

        /* check for last notes, else reduce max pairing count */
        if(pm  >  (num - j))
            pm = (num - j);

        if(pm < 2) /* not enough notes to pair */
            break;

        /* until max pair number */
        n=0;
        while(n < pm)
        {
            if(notes[j+n+1].paired > 0)
            {
                n++;
                break;
            }

            /* outside frametime in ticks */
            time1 = notes[j+n].time + notes[j+n].dur;
            time2 = notes[j+n+1].time;
            if ( (time2 - time1) > frame_ticks - FRAMETICK_RANGE)   // no pair
            {
                n++;
                break;
            }

            vel1 = notes[j+n].vel;
            vel2 = notes[j+n+1].vel;

            if(vvv>2)
                printf("pair %3ld:n %ld(%ld) at %ld (%ld) vel: %d - %d = %d (%d) \n",
                       j,n,pm,time1,time2-time1,
                       vel2,vel1,vel2-vel1,pair_diff_vel);

            if((vel2 -vel1) > pair_diff_vel)
            {
                n++;
                break;
            }
            n++;
        } /* while n find pair */
        if(n > 1)
        {
            paired += pair_dyn_notes(&notes[j],n,midinote);
        }
    } /*while j: notes in layer */

     if(vvv>1)
        printf(" %ld paired\n",paired);

    return paired;
}

unsigned long rule_pair_dyn_notes(DUR_NOTE **dur_notes,unsigned long *note_ons)
{
    int i;
    pair_count = 0;

    for(i=0; i< MAX_NOTES; i++)
    {
        if(note_ons[i] > 0)
            pair_count += pair_dyn_notes_layer(dur_notes[i],note_ons[i],i);
    }
    if(vvv>1)
        printf("dynmaic pairing using duration table: %ld paired\n",pair_count);

    return pair_count;
}


/* ======== Sorting by velocity od dur_notetable =============== */
static VEL_INDEX *sorted_index = NULL; // sort table
/*
 *   Sort the events in velocity
 *   algorithm: bubble sort
 *     a array (<VEL_INDEX *a>,<pointer>) of <int num>
 *     is sorted using the vel value
 */

int bubble_sort_vel_index(VEL_INDEX *a, unsigned long num)
{
    unsigned long i,j;
    unsigned long n = num;
    unsigned long tmp_index;
    int tmp_vel;
    int changed;

    for (i=0; i<n-1; i++)
    {
        changed = 0;
        for (j=0; j<n-1-i; j++)
        {    /* compare the two neighbors for time and first note off */
            if (a[j+1].vel > a[j].vel){
                changed = 1;
                tmp_index = a[j].index;         /* swap a[j] and a[j+1]      */
                tmp_vel = a[j].vel;
                a[j].index = a[j+1].index;
                a[j].vel = a[j+1].vel;
                a[j+1].index = tmp_index;
                a[j+1].vel = tmp_vel;
            }
        }
        if(changed == 0)
            break;
    }
    return 1;
}

VEL_INDEX *sort_vel_index(DUR_NOTE *note_layer,unsigned long num)
{
    unsigned long i;

    if(num < 1) /* to get sure */
        return NULL;

    /* make index */
    if(sorted_index)
        free(sorted_index);

    if((sorted_index = (VEL_INDEX *)calloc(num,sizeof(VEL_INDEX)))==NULL)
    {
        if(vvv>0)
            fprintf(stderr,"Error: could not alloc memory for sorting layer with %lu indexes\n",num);
        exit(9);
    }
    for(i=0;i<num;i++)
    {
        sorted_index[i].index = i;
        sorted_index[i].vel = note_layer[i].vel;
    }

    /* sort with bubblesort */
    bubble_sort_vel_index(sorted_index,num);

    return sorted_index;
}

