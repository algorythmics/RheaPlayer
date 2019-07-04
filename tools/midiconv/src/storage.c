/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    parse Midifile and sort in note slots                  */
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

/* statistics */
unsigned long pausen_changecount;
unsigned long laengen_changecount;
unsigned long pair_count = 0l;

/* === NEW Storage Make Array of Notes with duration and contruct new midifile afterwards === */

//int make_durnotes_storage(TRK *trk, unsigned long num)
//{
//   unsigned int i;
//   unsigned long j;
//   unsigned long time = 0l;
//   unsigned long notes_index[MAX_NOTES]; /* indexes in notes layers */
//   unsigned long old_noteofftime[MAX_NOTES];
//   int note,velo;
//   MTevt *me;
//    MIDIevt *evt;
//    EVENT *events = trk->events;
//    NOTE **notes = trk->notes;
//    unsigned long *note_ons = trk->note_ons;
//
//
//if((trk->dur_notes = (DURNOTE *) calloc(note_ons[i],sizeof(NOTE))) == NULL)
//                {
//                    if(vvv>0)
//                        fprintf(stderr,"Error: could not alloc memory for NOTES[%ld]*%ld\n",
//                            note_ons[i],sizeof(NOTE));
//                }
//
//
//}
//


/* ====================== Old Method Store references to noteon/offs ==========================
 * Store events in note arrays
 */


/* first store all events with absolut times
 *   in an array, to change times afterwards */
int make_event_array(EVENT *events,MTevt *mevt,unsigned long num)
{
    unsigned long j;
    unsigned long time = 0l;
    MTevt *me = mevt;

    /* asume events is big enough for num */
    for(j=0; j<num; j++)
    {
        if(me == NULL)
        {
            /* counted before so must not happen */
            if(vvv > 0)
                fprintf(stderr,"ERROR: make_event_array: to less events %ld/%ld for array !!!\n",j,num);
            return -1;
        }
        time += me->dt;
        events[j].time = time;
        events[j].evt = me;
        events[j].del = 0; /* no del flag */
        me = me->next;
    }
    return j;
}

/*
 *   second filter note events and store them in note layer
 *   indexed by note-numbers, generate new note off if needed
 */

int make_notes_array(TRK *trk, unsigned long num)
{
    unsigned int i;
    unsigned long j,k;
    unsigned long time = 0l;
    unsigned long notes_index[MAX_NOTES]; /* indexes in notes layers */
    unsigned long old_noteofftime[MAX_NOTES];
    int note,velo;
    MTevt *me;
    MIDIevt *evt;
    EVENT *events = trk->events;
    NOTE **notes = trk->notes;
    DUR_NOTE **dur_notes = trk->dur_notes;
    unsigned long *note_ons = trk->note_ons;

    for(i=0; i< MAX_NOTES; i++)
    {
        notes_index[i]=0l;
        old_noteofftime[i]=0l;
    }

    for(j=0l,k=0l; j<num; j++)
    {
        me = events[j].evt;
        time = events[j].time;
        switch(me->data.status)
        {
        case MF_META_EVENT:
            /* tempo is important for time conversion */
            /* already done ERROR: after change inconsistent midi data, 12 wrong events
            in check_in converted to absolute time see there */

            if(k<trk->meta_count)
                trk->metas[k++] = events[j];
            break;

        default: /* MIDIEVEERROR: after change inconsistent midi data, 12 wrong events
NT */
            evt = &(me->data);
            switch(evt->status & 0xF0) /* ignore channel */
            {
            case MD_NOTE_ON :
                note = evt->data.message[0];
                velo = evt->data.message[1];
                if(velo > 0)
                {
                    if(notes_index[note] >= note_ons[note])
                    {
                        if(vvv > 0)
                            fprintf(stderr,"notecount %5ld >= %5ld of noteons %3d %3d wrong !!!\n",notes_index[note],note_ons[note],note,velo);
                        break;
                    }
                    if(notes[note] && notes[note][notes_index[note]].noteon != NULL)
                    {
                        if(vvv>1)
                            printf("W: found overlapping noteon %d at %8.1f, make noteoff !\n",
                                   note,ticks_to_time(time));
                        /* NOTE: In future a noteoff should be generated and
                            a new note generated and the next note off ignored
                            Now: make note off */
//                        evt->data.message[1] = 0;
                    }
                    else
                    {
                        if(notes[note])
                        {
                            notes[note][notes_index[note]].noteon=&events[j];
                            notes[note][notes_index[note]].org_noteon=&events[j];
                            notes[note][notes_index[note]].new_vel = velo;
                            notes[note][notes_index[note]].shift_frames = 0;
                        }
                        if(dur_notes[note])
                        {
                            dur_notes[note][notes_index[note]].time = time;
                            dur_notes[note][notes_index[note]].dur = 0l;
                            dur_notes[note][notes_index[note]].vel = velo;
                            dur_notes[note][notes_index[note]].org_vel = velo;
                            dur_notes[note][notes_index[note]].del = 0;
                            dur_notes[note][notes_index[note]].paired = 0;
                        }
                        break;
                    }

                }  /* else note off */

            case MD_NOTE_OFF :
                note = evt->data.message[0];
                velo = 0;   /* ignore noteoff velocity */
                if(notes[note] && notes[note][notes_index[note]].noteon == NULL)
                {
                    if(vvv>1)
                        printf("W: found noteoff %d at %8.1f before noteon, ignore !\n",
                               note,ticks_to_time(time));
                    break;
                }
                if(notes[note][notes_index[note]].noteoff != NULL)
                {
                    if(vvv>1)
                        printf("W: found noteoff %d at %8.1f with previous noteoff, ignore !\n",
                               note,ticks_to_time(time));
                }

                notes[note][notes_index[note]].noteoff=&events[j];
                notes[note][notes_index[note]].org_noteoff=&events[j];
                /* if there is also a note on then continue to next */

                if(notes[note][notes_index[note]].noteon != NULL)
                {

                    /* moving noteon not needed anymore, since sorting will take noteoffs first
                                        if(old_noteofftime[note] &&
                                                notes[note][notes_index[note]].noteon->time == old_noteofftime[note])
                                        {
                                            if(vvv>1)
                                                printf("W: note pause to previous is zero at %8.1f (%8.1f) add 1!\n",
                                                       ticks_to_time(notes[note][notes_index[note]].noteon->time),
                                                       ticks_to_time(old_noteofftime[note]));
                                            notes[note][notes_index[note]].noteon->time += 1l;
                                        }
                    */
                    /* check for overlapping or so */
                    if(old_noteofftime[note] && notes[note][notes_index[note]].noteon->time < old_noteofftime[note])
                    {
                        if(vvv>0)
                            printf("ERROR: note pause to previous is negative at %8.1f (%8.1f) add %8.1f, shifting noteon!\n",
                                   ticks_to_time(notes[note][notes_index[note]].noteon->time),
                                   ticks_to_time(old_noteofftime[note]),
                                   ticks_to_time(notes[note][notes_index[note]].noteon->time - old_noteofftime[note]+1l));
                        notes[note][notes_index[note]].noteon->time = old_noteofftime[note];
                    }

                    old_noteofftime[note] = notes[note][notes_index[note]].noteoff->time;

                    /* check if duration is positive (always should) */
                    if(notes[note][notes_index[note]].noteoff->time <
                            notes[note][notes_index[note]].noteon->time)
                    {
                        if(vvv>1)
                            printf("W: note len is negative at %8.1f (%8.1f) add set to zero!\n",
                                   ticks_to_time(notes[note][notes_index[note]].noteon->time),
                                   ticks_to_time(notes[note][notes_index[note]].noteoff->time));
                        notes[note][notes_index[note]].noteoff->time = notes[note][notes_index[note]].noteon->time;
                    }

                    dur_notes[note][notes_index[note]].dur = time - dur_notes[note][notes_index[note]].time;
                    notes_index[note]++;
                }
            default:
                break;
            } /*switch event*/
        } /* switch data status (metaeven) */
    } /* for j,...num */

    /* correct if noton offs count wrong */
    for(i=0; i< MAX_NOTES; i++)
        if(notes_index[i] < note_ons[i])
            note_ons[i] = notes_index[i];

    /* TODO: CHECK for correct noteon/offs at end and correct num of notes */
    /* maybe use checknotes routine */
    return num;
}

/* after processing one trtests/test_note_out.midack reconstruct the MIDI */
/* Reconstruct means delete deleted and order in time of events */
int reconstruct_mifi_trk(TRK *trk)
{
    unsigned long j;
    unsigned long dt;
    unsigned long num = trk->num;
    unsigned long time = 0l;
    unsigned long nons,noffs;
    MTevt *prev_evt = NULL;
    MTevt *evt = NULL;
    EVENT *events = trk->events;
    MTevt *meta_endoftrack = NULL;

    if(!quiet)
        printf("reconstruct MIDI event-list... \n");

    /* start new event list */
    trk->trk->events = NULL;
    nons = noffs = 0l;
    for(j=0; j<num; j++)
    {

        /* filter events not to save  */
        if(events[j].del == 1)
        {
            continue;
        }

        if(events[j].evt->data.status == MF_META_EVENT &&
                events[j].evt->data.data.meta_data->type == MF_META_ENDOFTRK)
        {
            meta_endoftrack = events[j].evt;
            continue;
        }

        if((events[j].evt->data.status & 0xF0) == MD_NOTE_ON)
        {
            if(events[j].evt->data.data.message[1] == 0)
                noffs++;
            else
                nons++;
        }
        else  if((events[j].evt->data.status & 0xF0) == MD_NOTE_OFF)
            noffs++;

        evt = events[j].evt;
        if(trk->trk->events == NULL) /* first event in track */
            trk->trk->events = evt;

        dt = events[j].time - time;
        evt->dt = dt;
        time += dt;

        /* concanate list */
        evt->next = NULL;
        if(prev_evt != NULL)
        {
            evt->prev = prev_evt;
            prev_evt->next = evt;
        }
        prev_evt=evt;
    } /* for j... */

    if(meta_endoftrack) // add optional endoftrack event
    {
        meta_endoftrack->next = NULL;
        meta_endoftrack->prev = prev_evt;
        prev_evt->next = meta_endoftrack;
    }

    if(vvv>2)
        printf("counted: %ld note ons, %ld note offs\n",nons,noffs);
    if(noffs != nons && vvv>1)
        printf("Warning: wrong number of noteons (%ld) and noteoffs(%ld) reconstructed\n",
               nons,noffs);

    return j;
}

/* --------- reconstruct dur_notes storage to mifi -------------- */
/* after processing reconstruct the MIDI event arry*/
/* using the dur_notes array and meta array */
unsigned long reconstruct_events_durnote_trk(TRK *trk)
{
    int i;
    unsigned long j;
    unsigned long nons;
    unsigned long event_index;
    EVENT *events = trk->events;
    DUR_NOTE **notes = trk->dur_notes;
    unsigned int notes_count;
    MTevt *mtevents = NULL;

    if(!quiet)
        printf("reconstruct MIDI event-list... \n");

    event_index = 0l; /* index forevents[evt_count] event storage */

    /* first write META events into events array */
    for(i=0; i<trk->meta_count; i++)
    {
        events[event_index++] = trk->metas[i];
    }

    /* count notes in layers */
    notes_count = 0l;
    for(i=0; i<MAX_NOTES; i++)
    {
        nons = trk->note_ons[i];
        for(j=0; j<nons; j++)
            if(notes[i][j].del == 0)
                notes_count++;
    }

    /* need 2 events per note */
    if((mtevents = (MTevt *) calloc(2ul * notes_count,sizeof(MTevt))) == NULL)
    {
        fputs("maclloc failed\n",stderr);
        exit(-5);
    };

    for(i=0; i<MAX_NOTES; i++)
    {
        nons = trk->note_ons[i];
        for(j=0; j<nons; j++)
        {
            if(notes[i][j].del > 0)
                continue;

            events[event_index].time = notes[i][j].time;
            events[event_index].del = 0;
            events[event_index].evt = &mtevents[event_index];
            events[event_index].evt->data.status = MD_NOTE_ON;
            events[event_index].evt->data.data.message[0] = i;
            events[event_index].evt->data.data.message[1] = notes[i][j].vel;
            event_index++;

            events[event_index].time = notes[i][j].time + notes[i][j].dur;
            events[event_index].del = 0;
            events[event_index].evt = &mtevents[event_index];
            events[event_index].evt->data.status = MD_NOTE_ON;
            events[event_index].evt->data.data.message[0] = i;
            events[event_index].evt->data.data.message[1] = 0;
            event_index++;
        };
    };
    trk->num = event_index;
    return event_index;
}

/* ----------- CHECK notes for noteon/off order --------------- */
unsigned long check_notes(NOTE **notes,unsigned long *note_ons)
{
    unsigned long i,j,k;
    unsigned long old_offtime;
    unsigned long old_ontime;
    unsigned long errors = 0l;
    NOTE *noten;
    EVENT *noteon, *noteoff;

    for (i=0; i<MAX_NOTES; i++)
    {
        old_offtime = old_ontime = 0l;
        noten=notes[i];

        for (j=0; j<note_ons[i]; j++)
        {
            noteon = noten[j].noteon;
            noteoff = noten[j].noteoff;

            if(!noteon || !noteoff)
            {
                errors++;
                if(vvv>=1)
                    printf("check:  notes[%ld][%ld]:[ noteon (%s) or noteoff (%s) not set \n",i,j,
                           (noteon?"set":"NULL"),(noteoff?"set":"NULL"));
                if(noteon)noteon->del = 1;
                if(noteoff)noteoff->del = 1;
                continue;
            }

            /* first if noteon and notoff activ */
            if(noteon->del == 0 && noteoff->del == 1)
            {
                errors++;
                if(vvv>1)
                    printf("check: noteon[%ld][%ld] %d with deleted noteoff %d found at %8.1f-%8.1f ms\n",i,j,
                           noteon->evt->data.data.message[0],noteoff->evt->data.data.message[0],
                           ticks_to_time(noteon->time),ticks_to_time(noteon->time));
                if(do_change_force)
                {
                    noteon->del = 1;
                }
            }

            if(noteon->del == 1 && noteoff->del == 0)
            {
                errors++;
                if(vvv>1)
                    printf("check: deleted noteon[%ld][%ld] %d with noteoff %d found at %8.1f-%8.1f ms\n",i,j,
                           noteon->evt->data.data.message[0],noteoff->evt->data.data.message[0],
                           ticks_to_time(noteon->time),ticks_to_time(noteon->time));
                if(do_change_force)
                    noteoff->del = 1;
            }

            if(noteon->del == 1) /* deleted dont care anymor */
                continue;

            /* check for length (new: zero length is valid) */
            if(noteon->time > noteoff->time)
            {
                errors++;
                if(vvv>1)
                    printf("check: note%ld[%ld] %d-%d, has less length at %8.1f-%8.1f ms\n",i,j,
                           noteon->evt->data.data.message[0],noteoff->evt->data.data.message[0],
                           ticks_to_time(noteon->time),ticks_to_time(noteoff->time));

                if(do_change_force)
                    noteon->del = noteoff->del = 1;
            }

            /* check for pause to previous */
            if(old_offtime > 0l && noteon->time < old_offtime)
            {
                errors++;
                if(vvv>1)
                    printf("check: note%ld[%ld] %d-%d at %8.1f has no pause to previous note at %8.1f (%8.1f) ms\n",i,j,
                           noteon->evt->data.data.message[0],noteoff->evt->data.data.message[0],
                           ticks_to_time(noteon->time),
                           ticks_to_time(old_ontime),ticks_to_time(old_offtime));
//                      if(do_change_force)
//                          noteon->del = noteoff->del = 1;
            }
            old_offtime = noteoff->time;
            old_ontime = noteon->time;

            /* check if any note is overlapping */
            for(k=0; k<note_ons[i]; k++)
            {
                if(k != j  && noten[k].noteon->del == 0
                        && ((noten[k].noteon->time >= noteon->time && noten[k].noteon->time < noteoff->time)
                            || (noten[k].noteoff->time > noteon->time && noten[k].noteoff->time <= noteoff->time)))
                {
                    errors++;
                    if(vvv>1)
                        printf("check: note%ld[%ld] %d-%d at %8.1f-%8.1f overlaps note%ld[%ld] at %8.1f-%8.1f ms\n",i,j,
                               noteon->evt->data.data.message[0],noteoff->evt->data.data.message[0],
                               ticks_to_time(noteon->time),ticks_to_time(noteoff->time),i,k,
                               ticks_to_time(noten[k].noteon->time),ticks_to_time(noten[k].noteoff->time));

                    if(do_change_force)
                        noteon->del = noteoff->del = 1;
                }
            }
        }
    }
    return errors;
}

/* ======== Helpers needed + code grave =========== */


