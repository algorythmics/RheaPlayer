/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    check Midifile and count notes, extract info          */
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
/* ====================== 1.pass ==========================
  *   CHECK MIDIFILE for tracks and store them in local
  *   TRK list (firsttrack, previoustrack).
  *   count events and analyse for velocity, framerate
  *   no modifications and storage outside the mifi file
  */

TRK *firsttrack;
TRK *previoustrack;

/* count number of note-ons -offs for phase 2*/
unsigned int min_velo = 127;   /* statistics, get minimum of velos */
unsigned int max_velo = 0;     /* statistics, get maximum of velos */
unsigned long frame_ticks = 0l;      /* find frameticks */
unsigned long minpausen = 0;   /* statistics: how often to adjust */
unsigned long minlaengen = 0; /* statistics: how often to adjust */
unsigned long min_repetition = 0;  /*statistics: min repetition detected (should be frame rate) */

int mf_check_in(MIDIFile *mifi)
{
    MThd *mifihd = mifi->MThd;
    MTrk *trk;
    MTevt *me;
    METAstr *meta;
    MIDIevt *evt;

    int i,j;
    unsigned long time;
    unsigned long evtcount;
    unsigned long trackcount;
    unsigned long reptime = 0l;
    unsigned int note,velo;
    unsigned long last_note_on_time[MAX_NOTES];

    /* first define timebase from header */
    if(mifihd->divisions & 0x8000)
    {
        if(vv>1)printf("Time:   SMPTE, %d fr/s,%d ticks/fr\n",
                           -((mifihd->divisions & 0x7F00)>>8),mifihd->divisions & 0x00FF);

        timeformat = MF_MTC;
        ticktime =  (float) 60.0 / ((float) tempo * (float) (mifihd->divisions & 0x7FFF));

    }
    else
    {
        if(vv>1)
            printf("Time: MIDI, Ticks/Quarter:  0x%x = %d \n",
                   mifihd->divisions,mifihd->divisions & 0x7FFF );

        timeformat = MF_MC;
        ticksperquarter = mifihd->divisions & 0x7FFF;
        ticktime =  (float) 60.0 / ((float) tempo * (float) (mifihd->divisions & 0x7FFF));

        ticktime = 0.0; /* wait for tempo setting */
        /*       set_ticktime(tempo); */
    }

    if (mifihd->ntrks < 1)
    {
        fprintf(stderr, "ERROR: midifile has no track ????, bye");
        exit(-1);
    }

    /* second examin first track */
    trk = mifihd->starttrack;
    trackcount = 0;
    firsttrack= NULL;
    previoustrack = NULL;

    while(trk != NULL)
    {

        TRK *store = malloc(sizeof(TRK));
        unsigned long *note_ons = store->note_ons;
        unsigned long *note_offs = store->note_offs;

        if(firsttrack == NULL)
            firsttrack = store;
        else
            previoustrack->next = store;

        previoustrack = store;

        store->num = 0l;
        store->next = NULL;
        store->events = NULL;
        store->metas = NULL;
        store->meta_count = 0l;

        for(i=0;i<MAX_NOTES;i++){
            store->note_ons[i] = 0l;
            store->note_offs[i] = 0l;
            store->notes[i] = NULL;
            store->dur_notes[i] = NULL;
        }

        /* if MTC then already set, else with first tempoevent */
        store->ticktime = ticktime;
        store->trk = trk;
        trackcount++;
        me = trk->events;
        j=0;
        time = 0l;
        evtcount = 0l;
        min_repetition = 0l; /* maximum of time */

        for(i=0; i<MAX_NOTES; i++){
            last_note_on_time[i]=0l;
            note_ons[i] = note_offs[i] = 0l;
        }

        while(me != NULL)
        {
            time += me->dt;
            evtcount++;

            switch(me->data.status)
            {
            case MF_META_EVENT :
                if(vv > 2)
                {
                    printf("%5d=%7ld:",j,time);
                    mf_print_meta(me->data.data.meta_data);
                }

                store->meta_count++;

                meta = me->data.data.meta_data;
                switch(meta->type)
                {
                case MF_META_SETTEMPO : // sec/quarternote
                    sec_per_quarter = (float)((int) (meta->data[0]<<16)+
                                              (int) (meta->data[1]<<8)
                                              +meta->data[3] ) / (float) 1000000.0;
                    tempo =  (float) 60.0/sec_per_quarter;
                    set_ticktime(tempo);
                    store->ticktime = ticktime;

                    if(vv>1)
                    {
                        printf("set tempo to %f, which is %f secs per quarter\n",tempo,sec_per_quarter);
                        printf("this results with %d ticks/quarter in time resolution of %f ms per tick\n", ticksperquarter, ticktime);
                    }
                    break;
                }
                break;

            default: /* filter channel MIDI event */
                evt = &(me->data);
                switch(evt->status & 0xF0)
                {
                case MD_NOTE_ON :
                    if(evt->data.message[1] > 0)  /* velocity 0 is note off */
                    {
                        /* NOTE ON ---------------- */
                        note = evt->data.message[0];
                        velo = evt->data.message[1];
                        note_ons[note]++;

                        if(vv > 2)
                            printf("check: found NOTE ON  %3d %3d at %9ld\n",note,velo,time);

                        /* detect min_repetition */
                        reptime = (time - last_note_on_time[note]);
                        if (time == 0)
                        {
                            /* start; reset */
                            min_repetition = 0l;
                        }
                        else if (min_repetition == 0l && reptime > 0)  /* first repetition time, store */
                        {
                            min_repetition = reptime;
                        }
                        else if ( reptime < min_repetition )
                            min_repetition = reptime;

                        last_note_on_time[note] = time;

                        /* min max velo check */
                        if(velo < min_velo)min_velo=velo;
                        if(velo > max_velo)max_velo=velo;

                        break;
                    }  /* else note off */

                case MD_NOTE_OFF :
                    note = evt->data.message[0];
                    velo = evt->data.message[1];
                    note_offs[note]++;
                    if(vv > 2)
                        printf("check: found NOTE OFF %3d %3d at %9ld\n",note,velo,time);
                    break;
                }
            }; /* status */
            j++;
            me = me->next;
        } /* while MTevts */
        store->num = evtcount;
        store->ticktime = ticktime;

        if(pminmax)
            printf("velocityrange %d %d\n",min_velo,max_velo);

        if (frame_time == 0)  /* no from commandline take measured one */
        {
            frame_time = min_repetition * ticktime;
            frame_ticks = min_repetition;
        }
        else
            frame_ticks = (unsigned long) (frame_time / ticktime);

        if(pframetimes)
            printf("repetition time: %f (%f measured) ms\n",frame_time, (float) min_repetition * ticktime);

        trk = trk->next;

        if(vv > 2)
            for(i=0; i<MAX_NOTES; i++)
            {
                printf("%3d:%5ld-%5ld ",i,note_ons[i],note_offs[i]);
                if((i+1)%8 == 0)printf("\n");
            }
    } /* while trk */
    return evtcount;
}


/* put all storage in this module (from change) */
//int mf_store(MIDIFile *mifi)
//{
//    MTrk *trk;
//    int i;
//    unsigned long trackcount = 0;
//    unsigned long *note_ons;
//
//    EVENT *events;
//    unsigned long num;
//    unsigned long ret;
//
//    TRK *store = firsttrack;
//
//    if(vvv>1)
//        printf("changing File %s: mode %d\n", filename,regel-1);
//    trackcount=0;
//
//    while(store != NULL)
//    {
//    }
//    return 1;
//}



/* ---- HELPERS --- */

/* sec_per_quarter is for getting tempo and not rounded */
float set_ticktime(float tempo)
{
    if(tempo > 0.0)
        sec_per_quarter = (float) 60.0 / tempo;

    if(timeformat == MF_MC)
        ticktime = (float) 1000.0 * sec_per_quarter / (float) ticksperquarter;
    return ticktime;
}
