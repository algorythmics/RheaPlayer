/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    winfried ritsch  (sept 2005-)                         */
/************************************************************/

#ifndef MIDICONV_H_INCLUDED
#define MIDICONV_H_INCLUDED

#define TEXT_HEADER "MIDI-File checking and optional converting for Autoklavierspieler\n" \
"V 0.97, (c) IEM+Algorythmics 2005-2016\n\n"

/* ---- HELPERS --- */
/* MidiClock or MidiTimeCode */
#define MF_MC  0
#define MF_MTC 1

#define ticks_to_time(t) (ticktime*(float)(t))
#define time_to_ticks(t) ((unsigned long)((t)/ticktime))
float set_ticktime(float tempo);


 #define MAX_NOTES MD_DATAMAX /* 128 MIDI note numbers */

 typedef struct _evt {
     unsigned long time;
     MTevt *evt;
     int del;
 } EVENT;

 /* a note consists of note on and note off */
 typedef struct _note {
     EVENT *noteon;
     EVENT *noteoff;
     EVENT *org_noteon; /* for undo purposes hold a copy */
     EVENT *org_noteoff;
     int paired;
     int new_vel;
     int shift_frames;
 } NOTE;

typedef struct _dur_note {
     unsigned long time;
     int vel;
     unsigned long dur;
     int del;
     int paired;
     int org_vel;
} DUR_NOTE;


 typedef struct _trk {
     MTrk *trk;
     float ticktime;

     unsigned long num;
     EVENT *events;

     unsigned long note_ons[MAX_NOTES];
     unsigned long note_offs[MAX_NOTES];
     NOTE *notes[MAX_NOTES];
     DUR_NOTE *dur_notes[MAX_NOTES];

     unsigned long meta_count;
     EVENT *metas; //track header messages

     struct _trk *next;
 } TRK;

typedef enum regel
    {none=0, corr=1, simple=2, pairing=3, grouping=4,
     pair_dn=10,pair_dyn=20}
    REGEL;

    /* parameter args */
extern int quiet, v, vv, vvv;
extern int pminmax, ppause, plaenge, pframetimes, ppair;
extern char *filename;
extern boolean do_change, do_change_force;
extern float min_pause, min_laenge;
extern boolean do_corr_del;
extern REGEL regel;
extern float frame_time;
extern int pair_max, pair_diff_vel;
extern float pair_vel_scale;
extern int pair_min_diff;

#define FRAMETICK_RANGE 1l

/* ====================== 1.pass ====================================*/

extern TRK *firsttrack;
extern TRK *previoustrack;
int mf_check_in(MIDIFile *mifi);

extern unsigned int min_velo ,max_velo;
extern unsigned long frame_ticks,minpausen,minlaengen,min_repetition;

/* ---- timings with defaults ------ */
extern int timeformat, ticksperquarter; /* for MC */
extern float sec_per_quarter, tempo ,ticktime;

/* storage of info*/
extern unsigned long pausen_changecount,laengen_changecount,pair_count;

int make_event_array(EVENT *events,MTevt *mevt,unsigned long num);
EVENT *make_eventoff(unsigned long time,int note,int chan);
int make_notes_array(TRK *trk, unsigned long num);
int reconstruct_mifi_trk(TRK *trk);
unsigned long reconstruct_events_durnote_trk(TRK *trk);
unsigned long check_notes(NOTE **notes,unsigned long *note_ons);

/* modify */
int del_notes(NOTE *notes,int n);
int pair_notes(NOTE *notes,int n);
int rule_pairs(NOTE **note_layer,unsigned long *note_ons);
int rule_groups(NOTE **note_layer,unsigned long *note_ons);
int do_grouping(NOTE **note_layer,unsigned long *note_ons);
int rule_pausen_notelen(NOTE **notes,unsigned long *note_ons);
int bubble_sort_events_up(EVENT *a, unsigned long num);

/* 2.pass read in arrays */
int mf_change(MIDIFile *mifi);

unsigned long rule_pair_dur_notes(DUR_NOTE **dur_notes,unsigned long *note_ons);
unsigned long rule_pair_dyn_notes(DUR_NOTE **dur_notes,unsigned long *note_ons);

#endif // MIDICONV_H_INCLUDED
