/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    reconstruct and save MIFI                             */
/*    winfried ritsch  (sept 2005-)                         */
/************************************************************/
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

/* change events */
int mf_change(MIDIFile *mifi)
{
    MTrk *trk;
    int i;
    unsigned long num;
    unsigned long ret;
    boolean do_check_notes = TRUE;
    unsigned long trackcount = 0;
    unsigned long *note_ons = NULL;
    EVENT *events = NULL;
    TRK *store = firsttrack;

    if(vvv>1)
        printf("changing File %s: mode %d\n", filename,regel-1);
    trackcount=0;

    while(store != NULL)
    {
        note_ons = store->note_ons;

        trackcount++;
        pausen_changecount = 0l;
        laengen_changecount = 0l;
        pair_count = 0l;
        num = store->num;

        if(vvv>1)
            printf("changing Track %ld with %ld events\n", trackcount, num);

        ticktime = store->ticktime;
        if(ticktime <= 0.0)
            if(vvv>0)fprintf(stderr,"This track has no ticktime !\n");

        if((events = (EVENT *) malloc(sizeof(EVENT)*num)) == NULL)
        {
            if(vvv>0)fputs("Error: could not alloc memory for EVENTS\n",stderr);
            break;
        };
        store->events = events;
        trk = store->trk;

        /* 1.pass store events in array with absolute time to sort later */
        if(make_event_array(events,trk->events,num) < 0)
        {
            if(vvv>0)fprintf(stderr,
                                 "Event count changed !!!!!!!!!!!!!!!!!!!!!\n");
            continue; /* next track */
        }

        /* store sequences for each note in arrays and zero it */
        if(vvv>1)
            printf("storing in Track %ld notes in array for each note\n", trackcount);

        for(i=0; i< MAX_NOTES; i++)
        {
            if(note_ons[i] > 0)
            {
                if((store->notes[i] = (NOTE *) calloc(note_ons[i],sizeof(NOTE))) == NULL)
                {
                    if(vvv>0)
                        fprintf(stderr,"Error: could not alloc memory for NOTES[%ld]*%lu\n",
                            note_ons[i],(unsigned long) sizeof(NOTE));
                }
                if((store->dur_notes[i] = (DUR_NOTE *) calloc(note_ons[i],sizeof(DUR_NOTE))) == NULL)
                {
                    if(vvv>0)
                        fprintf(stderr,"Error: could not alloc memory for DUR_NOTES[%ld]*%lu\n",
                            note_ons[i],(unsigned long) sizeof(DUR_NOTE));
                }
            }
            else
            {
                store->dur_notes[i] = NULL;
                store->notes[i] = NULL;
            }
        }

        if(store->meta_count && (store->metas = (EVENT *) calloc(store->meta_count,sizeof(EVENT))) == NULL)
        {
            if(vvv>0)
                fprintf(stderr,"Error: could not alloc memory for METAs[%ld]*%lu\n",
                store->meta_count,(unsigned long) sizeof(MTevt));
        }

        if(make_notes_array(store,num) < 0)
        {
            if(vvv>0)fprintf(stderr,
                             "make_notes_array: something wrong !\n");
            continue;
        }

        /* ckeck for errors */
        if(vvv>1)
            printf("checking before changes... \n");
        if((ret = check_notes(store->notes,note_ons)) > 0l)
        {
            fprintf(stderr,"ERROR: before change inconsistent midi data, %ld wrong events\n",ret);
            exit(2);
        }

        if(vvv>1)
            printf("applying in Track %ld regel: %d \n", trackcount,regel);

        if (regel == simple)
        {
            if(vvv>1)
                printf("simple pairing Track %ld\n", trackcount);
            if(vvv>0)
                printf("Deprecated: Use mode 10 instead\n");
            if(!quiet)
                fprintf(stderr,"Deprecated: Use mode 10 instead\n");
            exit(1);
//            rule_pairs(store->notes,note_ons);
//
//            if(!quiet)
//                printf("simpled paired %ld notes \n",  pair_count);
        }
        else   if (regel == pairing) /* MODE 2 */
        {
            if(vvv>1)
                printf("pairing Track %ld\n", trackcount);

            if(vvv>0)
                printf("Deprecated: Use mode 20 instead\n");
            if(!quiet)
                fprintf(stderr,"Deprecated: Use mode 20 instead\n");
            exit(1);
//            rule_groups(store->notes,note_ons);
//
//            if(!quiet)
//                printf("Paired %ld notes \n",  pair_count);
        }
      //            do_grouping(store->notes,note_ons);
//            if(!quiet)
//                printf("grouping paired %ld notes \n",  pair_count);
  else   if (regel == grouping)
        {
            if(vvv>1)
                printf("grouping pairing Track %ld\n", trackcount);

            if(vvv>0)
                printf("Deprecated: Use mode 20 instead\n");
            if(!quiet)
                fprintf(stderr,"Deprecated: Use mode 20 instead\n");
            exit(1);
//            do_grouping(store->notes,note_ons);
//            if(!quiet)
//                printf("grouping paired %ld notes \n",  pair_count);
        }
        else if (regel == corr)
        {
            if(vvv>1)
                printf("correct length, pauses in Track %ld\n", trackcount);

            rule_pausen_notelen(store->notes,note_ons);

            if(!quiet)
                printf("Changed %ld pausen and %ld laengen\n",
                       pausen_changecount, laengen_changecount);
            if(!quiet && minpausen > 0l)
                printf("%ld Noten mit Pause kleiner %f ms\n",minpausen,min_pause);

            if(!quiet && minlaengen > 0l)
                printf("%ld Noten mit Notenlaenge kleiner %f ms\n",minlaengen,min_laenge);
        }
        else if (regel == pair_dn)
        {
            if(vvv>1)
                printf("pair duration notes in Track %ld\n", trackcount);
//            do_grouping(store->notes,note_ons);
//            if(!quiet)
//                printf("grouping paired %ld notes \n",  pair_count);

            pair_count = rule_pair_dur_notes(store->dur_notes,store->note_ons);
            reconstruct_events_durnote_trk(store);
            do_check_notes = FALSE;

            if(!quiet && minlaengen > 0l)
                printf("pair duration notes: %ld paired\n",pair_count);

        }
        else if (regel == pair_dyn)
        {
            if(vvv>1)
                printf("pair duration notes velocity sorted in Track %ld\n", trackcount);

            pair_count = rule_pair_dyn_notes(store->dur_notes,store->note_ons);
            reconstruct_events_durnote_trk(store);
            do_check_notes = FALSE;

            if(!quiet && minlaengen > 0l)
                printf("pair duration elocity sorted notes: %ld paired\n",pair_count);
        }
        else
        {
            fprintf(stderr,"Error: no mode choosen, try -h for help\n");
            exit(1);
        }

        /* ckeck for errors in notes */
        if(do_check_notes && (ret = check_notes(store->notes,note_ons)) > 0l)
        {
            if(!quiet)
                printf("checking afterwards... \n");

            fprintf(stderr,"ERROR: after change inconsistent midi data, %ld wrong events\n",ret);
            if(do_change_force)
            {
                fprintf(stderr,"FORCED: deletion of inconsistent midi data, %ld wrong events\n",ret);
                if((ret = check_notes(store->notes,note_ons)) > 0l)
                {
                    fprintf(stderr,"ERROR: after forced check inconsistent midi data, %ld wrong events\n",ret);
                    exit(3);
                }
            }
            else
                exit(2);
        }

        /* ------- sort table and write dts in mifi-------- */
        if(!quiet)
            printf("sorting... \n");

        /* new number of events */
        num = store->num;
        bubble_sort_events_up(events,num);

        if(!quiet)
            printf("reconstruct midifile ... \n");
        reconstruct_mifi_trk(store);

        store = store->next;
    } /* while store */

    return 1;
}
