/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    change by mode                                        */
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
/* -------------------------------------------- */
int del_notes(NOTE *notes,int n)
{
    int m;

    for(m=0; m<n; m++)
    {
        notes[m].noteon->del = 1;
        notes[m].noteoff->del = 1;
        notes[m].paired = 0;
    }
    return m;
}

/* ------ RULE 1 ----------------- make pair of N notes, number ----------- */
int pair_notes(NOTE *notes,int n)
{
    int m;
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
        velo = notes[m].noteon->evt->data.data.message[1];
        vmean += velo;
        if(velo > vmax)vmax = velo;
        if(velo < vmin)vmin = velo;
    }
    vmean = (int) ((float) velo / (float) n);

    velo = vmean; /* default pair_vel_scale = 0.5 */
    if(pair_vel_scale < 0.5)
        velo = (int) ((float) vmin  * (1.0 - (2.0*pair_vel_scale)) +
                      (float) vmean * (2.0*pair_vel_scale));
    else if(pair_vel_scale > 0.5)
        velo = (int) ((float) vmean  * (1.0 - (2.0*(pair_vel_scale-0.5))) +
                      (float) vmax * (2.0*(pair_vel_scale-0.5)));

    if(velo>127)velo=127;
    if(velo<1)velo=1;

    notes[0].noteon->evt->data.data.message[1] = velo;

    for(m=1; m<n; m++)
    {
        notes[0].paired++;                         /* count paired notes */
        notes[0].noteoff->del=1;               /* del current note off */
        notes[0].noteoff=notes[m].noteoff; /* take note off from paired note */
        notes[m].noteoff=NULL;
        notes[m].noteon->del=1;                    /* del paired noteon */
        notes[m].paired=0;                     /* reset paired from paired note */
    }
    if(vvv>1)
        printf("pair %2d notes %3d at %8.1f sec (off %8.1f ms) to velo %3d\n",
               n,notes[0].noteon->evt->data.data.message[0],
               ticks_to_time(notes[0].noteon->time)/1000.0,
               ticks_to_time(notes[0].noteoff->time-notes[0].noteon->time),velo);

    pair_count++;
    return n;
}

/* find sequences of pairs and pair them */
int rule_pairs(NOTE **note_layer,unsigned long *note_ons)
{
    int i;
    int vel1,vel2;
    unsigned long time1,time2;
    unsigned long j;
    NOTE *notes;
    unsigned long pm,n;
    //   int paired;

    if(vvv>1)
        printf("do pairing using frametime %8.1f ms (%ld), pair_max %d\n",frame_time,frame_ticks,pair_max);

    /* for all notes */
    for(i=0; i<MAX_NOTES; i++)
    {
        notes=note_layer[i];
        pm = (unsigned long) pair_max;
        if(pair_max == 0) // maximum is note count for unlimited
            pm = note_ons[i];

        j=0;
        while(j < note_ons[i])
        {
            if(pm  >  (note_ons[i] - j)) /* last notes ? */
                pm = (note_ons[i] - j);
            if(pm < 2) /* not enough notes to pair */
                break;

            n=0;
            while(n < pm)
            {
                /* sequence end if no more note */
                if((j+n+1) >= note_ons[i])
                {
                    if(vvv>2)
                        printf("pair %3d:n %ld(%ld) end of notes\n",i,n,pm);
                    n++;
                    break;
                }

                time1 = notes[j+n].noteon->time;
                time2 = notes[j+n+1].noteon->time;

                /* sequence end if time to big or veldiff to big */
                if ( (time2 - time1) > frame_ticks+FRAMETICK_RANGE)   // no pair
                {
                    n++;
                    break;
                }

                vel1 = notes[j+n].noteon->evt->data.data.message[1];
                vel2 = notes[j+n+1].noteon->evt->data.data.message[1];

                if(vvv>2)
                    printf("pair %3d:n %ld(%ld) at %ld (%ld) vel: %d - %d = %d (%d) \n",
                           i,n,pm,time1,time2-time1,
                           vel2,vel1,vel2-vel1,pair_diff_vel);

                if((vel2 -vel1) > pair_diff_vel)
                {
                    /* note to loud no pair anymore */
                    if(n==0)  /* first note to quiet, delete */
                    {
                        del_notes(&notes[j],1);  /* pause: DELETE Note */
                        n++; /* advance */
                    }
                    n++;
                    break;
                }
                /* next not is too quiet */
                if((vel2-vel1) <  pair_min_diff)
                {
                    del_notes(&notes[j+n],1);  /* pause: DELETE Note */
                    n++; /* advance */
                    break;
                }

                n++;
            } /* while n find pair */
            if(n > 1)
            {
                pair_notes(&notes[j],n);
            }
            j += n;
        } /*while j: notes in layer */
    } /* while layer */
    return pair_count;
}

/* ------ RULE 2 ----------------- make groups of 2 notes sorting velocity ----------- */

/* find sequences of pairs and pair them */
int rule_groups(NOTE **note_layer,unsigned long *note_ons)
{
    int i,n,m,k,k_max;
    int vel_max;
    unsigned long time1,time2;
    unsigned long j;
    NOTE *notes;
    unsigned long pm;
    //   int paired;

    if(vvv>1)
        printf("do grouping 2 using frametime %8.1f ms (%ld), pair_max %d\n",frame_time,frame_ticks,pair_max);

    /* for all notes */
    for(i=0; i<MAX_NOTES; i++)
    {
        notes=note_layer[i];
        pm = (unsigned long) pair_max;
        if(pair_max == 0)
            pm = note_ons[i];

        j=0;
        //     paired = 0;
        while((j + 1l) < note_ons[i])
        {
            if(pm  >  (note_ons[i] - j)) /* last notes ? */
                pm = (note_ons[i] - j);
            if(pm < 2)
                break;

            /* first find sequences: j..j+n */
            n=0;
            while((j+n+1) < note_ons[i])
            {
                time1 = notes[j+n].noteon->time;
                time2 = notes[j+n+1].noteon->time;

                /* sequence end if time to big or veldiff to big */
                if ( (time2 - time1) > (frame_ticks + FRAMETICK_RANGE))   // no pair
                {
                    n++;
                    break;
                }
                n++;
            }

            /* single note, end */
            if(n < 2)
            {
                if(vvv>2)
                    printf("no sequence %3d: %d(%ld) at %ld - %8.1f  \n", i,n,pm, j,ticks_to_time(time1));

                j += n;
                continue;
            }
            if(vvv>2)
                printf("sequence %3d: %d(%ld) at %ld - %8.1f  \n", i,n,pm, j,ticks_to_time(notes[j].noteon->time));

            k_max = 0;
            while(k_max < n)
            {
                /* find loudest note not processed */
                vel_max=0;
                k_max = n;
                for(k=0; k<n; k++)
                    if(!notes[j+k].paired &&
                            notes[j+k].noteon->evt->data.data.message[1] > vel_max)
                    {
                        vel_max = notes[j+k].noteon->evt->data.data.message[1];
                        k_max = k;
                    }
                if(k_max == n) /* nothing found */
                    break;

                if(vvv>2)
                    printf("            found k_max %d (n=%d) vel %d  \n", k_max,n, vel_max);

                /* next notes searching */
                m=1;
                while((k_max+m) < n && (unsigned long) m < pm)
                {

                    if(notes[j+k_max+m].paired > 0)
                        break; /*already paired */

                    if(vvv>2)printf("            next: %d vel: %d (pd=%d):",
                                        m+k_max,notes[j+k_max+m].noteon->evt->data.data.message[1],pair_diff_vel);

                    /* if diff more than vel_diff -> pause */
                    if((notes[j+k_max+m].noteon->evt->data.data.message[1] + pair_diff_vel) < vel_max)
                    {
                        notes[j+k_max+m].noteon->del = 1;
                        notes[j+k_max+m].noteoff->del = 1;
                        notes[j+k_max+m].paired++;
                        if(vvv>2)printf("paused vel=%d (%d)\n",
                                            notes[j+k_max+m].noteon->evt->data.data.message[1],vel_max);
                        break;
                    };

                    notes[j+k_max].noteoff->del = 1;
                    notes[j+k_max].noteoff = notes[j+k_max+m].noteoff;
                    notes[j+k_max+m].noteon->del = 1;
                    notes[j+k_max+m].paired++;
                    notes[j+k_max].paired++;
                    if(vvv>2)printf("grouped vel=%d\n",notes[j+k_max+m].noteon->evt->data.data.message[1]);
                    pair_count++;
                    m++;
                }

                if((unsigned long) m >= pm) /* pairing done */
                    continue;

                /* if next is a pause let it */

                time1 =  notes[j+k_max].noteon->time;
                time2 = notes[j+k_max+m].noteon->time;
                while((time1 + (frame_ticks*m)+FRAMETICK_RANGE) <= time2 && (unsigned long) m < pm)
                {
                    m++;
                    /* found pause afterwards */
                    if(vvv>2)printf("            found pause afterwards\n");
                    notes[j+k_max].paired++;
                }

                if((unsigned long) m >= pm) /* pairing done */
                    continue;


                m=1; /* search backwards */
                while((k_max-m) >= 0 && (unsigned long) m < pm)
                {

                    if(vvv>2)printf("            prev: %d vel: %d:",
                                        k_max-m,notes[j+k_max-m].noteon->evt->data.data.message[1]);

                    if(notes[j+k_max-m].paired > 0)
                    {
                        /* already paired */
                        if(vvv>2)
                            printf("already paired\n");
                        break;
                    }

                    notes[j+k_max].noteon->del = 1;
                    notes[j+k_max-m].noteoff->del = 1;
                    notes[j+k_max-m].noteoff = notes[j+k_max].noteoff;
                    notes[j+k_max].paired++;
                    notes[j+k_max-m].paired++;
                    /* velocity is average of paired */
                    notes[j+k_max-m].noteon->evt->data.data.message[1] =
                        (notes[j+k_max-m].noteon->evt->data.data.message[1] + (notes[j+k_max].paired*vel_max))/
                        (notes[j+k_max].paired+1);
                    if(vvv>2)
                        printf("paired %d to %d\n",k_max-m,notes[j+k_max].noteon->evt->data.data.message[1]);

                    pair_count++;
                    m++;
                }
                /* if this is single -> pause*/
                if(m == 1 && !notes[j+k_max].paired)
                {
                    if(vvv>2)printf("            single: %d deleted\n",k_max);
                    notes[j+k_max].noteon->del = 1;
                    notes[j+k_max].noteoff->del = 1;
                    notes[j+k_max].paired++;
                }
            } /* sequence prozessed */

            j += n;
        } /*while j: notes in layer */
    } /* while layer */
    return pair_count;
}


/* ------ RULE 3 ----------------- make groups of N notes, sorting velocities ----------- */

/* find sequences of pairs and pair them */
int do_grouping(NOTE **note_layer,unsigned long *note_ons)
{
    int i,n,m,l,k,k_max;
    unsigned int vel,vel_max;
    NOTE *notes;
    EVENT *tmp_event;
    unsigned long pm;
    unsigned long time;
    unsigned long paired;

    if(vvv>1)
        printf("do grouping N=%d notes sorted by velocity using frametime %8.1f ms (%ld)\n",
               pair_max,frame_time,frame_ticks);

    /* for all note numbers */
    for(i=0; i<MAX_NOTES; i++)
    {

        notes=note_layer[i];
        pm = (unsigned long) pair_max;

        /* if pair_max 0 use all as pairs */
        if(pair_max == 0)
            pm = note_ons[i];

        if(note_ons[i] == 0l)
            continue;

        if(vvv>2)
            printf("note %3d:\n",i);

        /* 1. pass find loudest pair forward  and mark */
        k_max = 0;
        while((unsigned long) k_max < note_ons[i])
        {

            /* find loudest note not processed */
            vel_max=0;
            k_max = note_ons[i];
            for(k=0; (unsigned long)k<note_ons[i]; k++)
                if(notes[k].paired < 1 &&
                        notes[k].noteon->evt->data.data.message[1] > vel_max)
                {
                    vel_max = notes[k].noteon->evt->data.data.message[1];
                    k_max = k;
                }
            if(k_max == note_ons[i]) /* nothing found anymore */
                break;

            notes[k_max].paired++; /* mark paired to one single */
            paired=1; /* no pairing done until now */

            /* a) find forward notes */

            time = notes[k_max].noteon->time;

            if(vvv>2)
                printf("   %8.1f k_max %d vel %d", ticks_to_time(time),k_max, vel_max);

            for(n=1,m=1; (unsigned long) n < pm; n++) /*n time frames, m notes */
            {

                if(vvv>2)printf(":ff");
                if((unsigned long)(k_max+m) >= note_ons[i])  /* no more notes, always group */
                {
                    if(vvv>2)printf("-no more notes paired true");
                    paired = pm;
                    break;
                }

                if(vvv>2)printf("-n %8.1f", ticks_to_time(n*frame_ticks + FRAMETICK_RANGE));

                if((notes[k_max+m].noteon->time - time) <= (n*frame_ticks + FRAMETICK_RANGE))
                {

                    if(notes[k_max+m].paired > 0)  /* aleady paired */
                    {
                        if(vvv>2)printf(" already paired");
                        break; /*stop foward search */
                    }

                    vel = notes[k_max+m].noteon->evt->data.data.message[1];
                    if(vvv>2)printf(" vel %3d",vel);

                    if( (int) (vel_max - vel) > pair_diff_vel )  /* if vel diff to big pause */
                    {
                        if(vvv>2)
                            printf(" del at   %8.1f",ticks_to_time(notes[k_max+m].noteon->time));
                        notes[k_max+m].noteon->del=1;
                        notes[k_max+m].noteoff->del=1;
                        notes[k_max+m].paired++;
                    }
                    else
                    {
                        if(vvv>2)
                            printf(" grp with %8.1f",ticks_to_time(notes[k_max+m].noteon->time));
                        tmp_event=notes[k_max].noteoff;
                        notes[k_max].noteoff=notes[k_max+m].noteoff;
                        notes[k_max+m].noteoff=tmp_event;
                        notes[k_max+m].noteoff->del=1;
                        notes[k_max+m].noteon->del=1;
                        notes[k_max+m].paired++;
                        notes[k_max].paired++;
                    }
                    m++; /* note found */
                }
                else    /* pause found */
                {
                    if(vvv>2)printf(" pause ");
                };
                paired++;
            } /* forward search */

            if((unsigned long) paired == pm)  /* enough paired */
            {
                if(vvv>2)printf(":f end");
            }

            /* find backwards only one note/pause */
            for(l=1,n=1; n < 2 && paired < pm; n++) /*n time frames, m notes */
            {

                if(vvv>2)printf(":fb ");

                if(vvv>2)printf("-p %8.1f", ticks_to_time(n*frame_ticks) + FRAMETICK_RANGE);

                /* if not enough time before event */
                if(time <= frame_ticks)
                {
                    if(vvv>2)printf(" - not enough time not pairing\n");
                    break;
                }

                if((k_max-l) >= 0 &&
                        (time - notes[k_max-l].noteon->time) <= (frame_ticks*l + FRAMETICK_RANGE))
                {
                    /* note before event */


                    if(notes[k_max-l].paired > 0)  /* aleady paired */
                    {
                        if(vvv>2)printf(":prev already paired\n");
                    }
                    else  /* pair note */
                    {

                        vel = notes[k_max-l].noteon->evt->data.data.message[1];
                        if(vvv>2)printf(":prev vel %d ",vel);

                        if( (int)(vel_max - vel) >  pair_diff_vel )  /* if vel diff to big pause */
                        {
                            if(vvv>2)
                                printf("-del note at %8.1f",ticks_to_time(notes[k_max-l].noteon->time));
                            notes[k_max-l].noteon->del=1;
                            notes[k_max-l].noteoff->del=1;
                            notes[k_max-l].paired++;
                            notes[k_max].shift_frames = n; /* shift note before */
                        }
                        else   /* group */
                        {
                            tmp_event=notes[k_max-l].noteoff;
                            notes[k_max-l].noteoff=notes[k_max].noteoff;
                            notes[k_max].noteoff = tmp_event;
                            notes[k_max].noteon->del=1;
                            notes[k_max].noteoff->del=1;
                            notes[k_max-l].paired++;
                            notes[k_max-l].new_vel=(vel+notes[k_max].paired*vel_max)/
                                                   (notes[k_max].paired+1);
                            if(vvv>2)
                                printf("-grp note(paired=%d->%d) at %8.1f",notes[k_max].paired,notes[k_max-l].new_vel,
                                       ticks_to_time(notes[k_max-l].noteon->time));
                            notes[k_max].paired++;
                        }
                        l++; /* note found */
                    } /* pair note */
                }
                else    /* pause found */
                {
                    notes[k_max].shift_frames = n; /* shift note */
                    if(vvv>2)printf(":bpause");
                }
                paired++;
            } /* for backward search */

            if(paired < pm)
            {

                /* revers all paired stuff and mark k_max as single */
                if(vvv>2)
                    printf("revers all paired %3ld (%2d,%2d)\n",paired,l-1,m-1);

                for(n=1; n<l; n++) /* undo backward pairs */
                {
                    notes[k_max-n].noteon = notes[k_max-n].org_noteon;
                    notes[k_max-n].noteoff = notes[k_max-n].org_noteoff;
                    notes[k_max-n].noteon->del=0;
                    notes[k_max-n].noteoff->del=0;
                    notes[k_max-n].paired = 0;
                    notes[k_max-n].shift_frames = 0;
                    notes[k_max-n].new_vel = 0;
                }
                for(n=1; n<m; n++) /* undo forward pairs */
                {
                    notes[k_max+n].noteon = notes[k_max+n].org_noteon;
                    notes[k_max+n].noteoff = notes[k_max+n].org_noteoff;
                    notes[k_max+n].noteon->del=0;
                    notes[k_max+n].noteoff->del=0;
                    notes[k_max+n].paired = 0;
                    notes[k_max+n].shift_frames = 0;
                    notes[k_max+n].new_vel = 0;
                }
                notes[k_max].paired = 1;
                notes[k_max].noteon = notes[k_max].org_noteon;
                notes[k_max].noteoff = notes[k_max].org_noteoff;
                notes[k_max].noteon->del=0;
                notes[k_max].noteoff->del=0;
                notes[k_max].shift_frames = 0;
                notes[k_max].new_vel = 0;

            }
            else   /* mark all paired with paired number */
            {

                for(n=1; n<l; n++) /* undo backward pairs */
                {
                    notes[k_max-n].paired = paired;
                }
                for(n=1; n<m; n++) /* undo forward pairs */
                {
                    notes[k_max+n].paired = paired;
                }
                notes[k_max].paired = paired;
                pair_count += paired;

                /* execute new vel */
                if(l>1 && notes[k_max-(l-1)].new_vel > 0)
                {
                    notes[k_max-(l-1)].noteon->evt->data.data.message[1] = notes[k_max-(l-1)].new_vel;
                    if(vvv > 2)printf("newvel[%d-(%d-1)](%d -> %8.1f)",k_max,l,notes[k_max-(l-1)].new_vel,
                                          ticks_to_time(notes[k_max-(l-1)].noteon->time));
                    notes[k_max-(l-1)].new_vel=0;
                }
                /* execute shift */
                if(notes[k_max].shift_frames > 0)
                {
                    notes[k_max].noteon->time -= notes[k_max].shift_frames*frame_ticks;
                    notes[k_max].noteon->evt->data.data.message[1] =
                        notes[k_max].noteon->evt->data.data.message[1]*(notes[k_max].paired-1)
                        / notes[k_max].paired;
                    if(vvv > 2)printf("shift(%d -> %8.1f(%8.1f))",notes[k_max].shift_frames,
                                          ticks_to_time(notes[k_max].noteon->time),ticks_to_time(notes[k_max].noteon->time+notes[k_max].shift_frames*frame_ticks));
                    notes[k_max].shift_frames = 0;
                }
                if(vvv>2)printf(":group end\n");
            } /* search for kmax */
        }
        /* 2.pass eliminate single notes not paired */
        for(n=0; n<note_ons[i]; n++)
        {
            if(notes[n].paired == 1)
            {
                if(vvv>2)printf(" %8.1f single del\n",ticks_to_time(notes[n].noteon->time));
                notes[n].noteon->del=1;
                notes[n].noteoff->del=1;
            }
        }
    } /* while layer */
    return pair_count;
}

/* ------------------ RULE 0 - correct laengen, pausen ---------------- */
int rule_pausen_notelen(NOTE **notes,unsigned long *note_ons)
{
    int i;
    unsigned long j;
    NOTE *noten;
    long notenpause,notenlaenge;
    unsigned long min_pause_ticks = time_to_ticks(min_pause);
    unsigned long min_laenge_ticks = time_to_ticks(min_laenge);

    /* first pausen */
    if(vvv>2)
        printf("mod_dcorr: adjusting pausen\n");
    for(i=0; i<MAX_NOTES; i++)
    {
        noten=notes[i];
        j=0;
        while(j + 1l < note_ons[i])
        {
            notenpause = noten[j+1].noteon->time - noten[j].noteoff->time;
            if(notenpause  < min_pause_ticks)
            {
                noten[j].noteoff->time =
                    noten[j+1].noteon->time - min_pause_ticks;

                if(vvv > 2)
                {
                    printf("note %03d: change pause at %8.1f  from %8.1f to %8.1f\n",
                           i,ticks_to_time(noten[j].noteoff->time),
                           ticks_to_time(notenpause), min_pause);
                }
                pausen_changecount++;
            }
            j++;
        }
    } /* for i */


    if(min_laenge_ticks > 0)
    {
        if(vvv>2)
            printf("mod_dcorr: adjusting laengen\n");

        for(i=0; i<MAX_NOTES; i++)
        {
            noten=notes[i];
            j=0;

            while(j < note_ons[i])
            {
                notenlaenge = noten[j].noteoff->time - noten[j].noteon->time;
                if(notenlaenge < min_laenge_ticks)
                {
                    noten[j].noteoff->time = noten[j].noteon->time + min_laenge_ticks;
                    if(vvv > 2)
                        printf("note %03d: change laenge at %8.1f  from %8.1f(%ld) to %8.1f(%ld)\n",
                               i,ticks_to_time(noten[j].noteon->time),
                               ticks_to_time(notenlaenge),notenlaenge, min_laenge,min_laenge_ticks);

                    if(j+1<note_ons[i]) /* not last note */
                    {
                        if(noten[j].noteoff->time > noten[j+1].noteon->time)
                        {
                            noten[j].noteoff->time = noten[j+1].noteon->time;
                            if(do_corr_del)
                                noten[j].noteon->del = noten[j].noteoff->del = 1;
                        }
                    }
                    laengen_changecount++;
                }
                j++;
            }
        } /* for i */
    }

    return laengen_changecount+pausen_changecount;
}


/* ========== HELPERS =============== */


/* --------------------------------------------- */
/*
 *   Sort the events in time after changes for saving.
 *   algorithm: bubble sort
 *     a array (<EVENT *a>,<pointer>) of <int num> floats-pairs
 *     is sorted using the float value plus noteon first
 */
int bubble_sort_events_up(EVENT *a, unsigned long num)
{
    unsigned long i,j;
    unsigned long n = num;
    unsigned long tmp_time;
    MTevt *tmp_evt;
    int changed = 1;

    for (i=0; i<n-1; i++)
    {

        changed = 0;
        for (j=0; j<n-1-i; j++)
        {
            /* compare the two neighbors for time and first note off */
            if (a[j+1].time < a[j].time ||
                    (a[j+1].time == a[j].time && a[j+1].evt && a[j+1].evt->data.status == MD_NOTE_OFF) )
            {
                changed = 1;
                tmp_time = a[j].time;         /* swap a[j] and a[j+1]      */
                tmp_evt = a[j].evt;
                a[j].time = a[j+1].time;
                a[j].evt = a[j+1].evt;
                a[j+1].time = tmp_time;
                a[j+1].evt = tmp_evt;
            }
        }
        if(changed == 0)
            break;
    }
    return 1;
}
