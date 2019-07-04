/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    Simple Pairing on Dur_notes                           */
/*    winfried ritsch  (sept 2005-)                         */
/************************************************************/
/* in modify flags for events and event data is changed,
   but never deleted. This will be done by storage functions */

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

/* ------ RULE 10 ----------------- make pair of N notes, number ----------- */
/* use the note-dur table */

/* pair next n notes with midi note number (for verbose) */
int pair_dur_notes(DUR_NOTE *notes,unsigned int n,unsigned int note_nr)
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
        notes[m].paired=0;                     /* reset paired from paired note */
    }
    if(vvv>1)
        printf("pair %2d notes %3d at %8.1f sec (dur %8.1f ms) to velo %3d\n",
               n,note_nr, ticks_to_time(notes[0].time)/1000.00,
               ticks_to_time(notes[0].dur),velo);

    return notes[0].paired;
}

/* pairs num notes in one midinote layer */
int pair_dur_notes_layer(DUR_NOTE *note_layer,unsigned long num, int midinote)
{
    int vel1,vel2;
    unsigned long time1,time2;
    unsigned long j;
    unsigned long pm,n;
    DUR_NOTE *notes = note_layer;
    unsigned long paired = 0;

    if(vvv>1)
        printf("do pairing Note %d using frametime %8.1f ms (%ld), pair_max %d:",
               midinote,frame_time,frame_ticks,pair_max);

    /* for all notes use 0, then notecount num is used */
    pm = (unsigned long) pair_max;
    if(pair_max == 0)
        pm = num;

    /* search in note list */
    j=0;
    while(j < num)
    {
        /* check for last notes, else reduce max pairing count */
        if(pm  >  (num - j))
            pm = (num - j);

        if(pm < 2) /* not enough notes to pair */
            break;

        /* until max pair number */
        n=0;
        while(n < pm)
        {

            time1 = notes[j+n].time;
            time2 = notes[j+n+1].time;

            /* sequence end if time to big  */
            /* note FRAMETIC_RANGE is a fixed parameter, should be arg */

            if ( (time2 - time1) > frame_ticks + FRAMETICK_RANGE)   // no pair
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
            paired += pair_dur_notes(&notes[j],n,midinote);
        }
        j += n;
    } /*while j: notes in layer */

     if(vvv>1)
        printf(" %ld paired\n",paired);

    return paired;
}

unsigned long rule_pair_dur_notes(DUR_NOTE **dur_notes,unsigned long *note_ons)
{
    int i;
    pair_count = 0;

    for(i=0; i< MAX_NOTES; i++)
    {
        if(note_ons[i] > 0)
            pair_count += pair_dur_notes_layer(dur_notes[i],note_ons[i],i);
    }
    if(vvv>1)
        printf("simple pairing using duration table: %ld paired\n",pair_count);

    return pair_count;
}

