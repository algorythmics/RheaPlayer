/************************************************************/
/*  Midifile Correction and Tester for robotic piano player */
/*    Optimierung zur wiedergabe bei sprache                */
/*    winfried ritsch  (sept 2005-)                         */
/************************************************************/
/*
 *  1.00 by wini - correction of length and pauses
 *  2.00 February 2008 - added additional modes like pairing,...
 *  3.00 added discard of non playable notes for Huddersfield pieces
 *
 *  Dependency: midilib for reading, printing and writing midifiles
 *
 * short functional description:
 * -----------------------------
 *
 * 1a) pass: a midifile is read and checked for consistency
 *
 * 2.pass:
 *   For each track Events stored in an event-array with absolute times.
 *   Events are organized in note layers.
 *   Each note layer is processed with specific rules
 *   Events are sorted and midifile is reconstructed
 *
 * 3.pass  (V3.0)
 *   Check for consistency and delete not playado_corr_delble notes.
 *
 * 4.pass:
 *   midifile is saved on a filed
 *
 */

/* A) systemdefines */

/* B) includes */
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


/* C) globals and defaults */

/* flags for verbosity, what things to print */
int quiet = 0;
int v = 1;         /* default, errors only */
int vv = 1;        /* checking, errors only */
int vvv = 1;       /* changing, errors only */
int pminmax=1;     /* print mixmax statistics */
int ppause=1;      /* print mixmax statistics */
int plaenge=1;     /* print mixmax statistics */
int pframetimes=1; /* print frametimes */
int ppair=1;       /* print pairing */

/* command line arguments */
char *filename = NULL;  /* filename of midifile */
boolean do_change = 0;  /* execute changes */
boolean do_change_force = 0;  /* execute changes */

float min_pause = 40l;          /* minimal pause to adjust in ms */
float min_laenge = 30l;         /* minimal length to adjust in ms */
boolean do_corr_del = FALSE;    /* delete if length cannot be minimal length */

REGEL regel = corr; /* RULE to apply */
float frame_time = 0.0;
int pair_max = 2;
int pair_diff_vel = 0;
float pair_vel_scale = 0;
int pair_min_diff = -127;

/* ---- timings with defaults ------ */
int timeformat = 0;
int ticksperquarter = 96; /* for MC */
float sec_per_quarter = 0.5; /* Tempo 120 */
float tempo = 120.0;
float ticktime = 1.0;

void usage(char **argv);
void parse_args(int argc,char *argv[]);

MIDIFile *mymifi;
char *outfile = NULL;

/* ============ main ======================== */
int main(int argc,char *argv[])
{
    fprintf(stderr,TEXT_HEADER); /* say who am I */

    parse_args(argc,argv);
    if(!quiet)fprintf(stderr,"MC: MIDI-FILE read verbose mode %d:\n",v);
    mf_verbose(v);

    if((mymifi = mf_read(filename)) != NULL)
    {
        if(!quiet)fputs("MC: done\n",stderr);

        if(!quiet)fputs("MC: printing:\n",stderr);
        // ptime=1;
        ptime=0;
        mf_print(mymifi);
        if(!quiet)fputs("MC: printing done\n",stderr);

        if(!quiet)fputs("MC: check in ...\n",stderr);
        mf_check_in(mymifi);
        if(!quiet)fputs("MC: check in done\n",stderr);

        if(!quiet)fputs("MC: changing:\n",stderr);

        mf_change(mymifi);

        if(!quiet)fputs("MC: changing done\n",stderr);

        /* only write if really change is activated */
        if(do_change)
        {
            if(outfile)
            {
                if(!quiet)fprintf(stderr,"MC: saving on %s ...",outfile);
                mf_save(mymifi,outfile);
                if(!quiet)fputs("MC:  done\n",stderr);
            }
        }
        /* Since exchanged midievent storade, do not try to free them
        mf_free(mymifi);
        */
        if(!quiet)fputs("MC: all done ...\n    ... good bye !\n",stderr);
        return(0);
    };

    if(!quiet)
        fputs("MC: something wrong happened trying to read ...\n    ... good bye !\n",
              stderr);
    return -1;
}
/* ==== USAGE and args === */
void usage(char **argv)
{
    fprintf(stderr,"\n usage: %s [-<options>] [<midi-file-in>]\n\n", argv[0]);

    fprintf(stderr,
            "options:\n\n"
            " -v <n> verbosity during reading"
            " (0=no,1=errors,2=+trks,3=debug)\n"
            " -w <n> verbosity during check in"
            " (0=no,1=errors,2=+warnings,3=debug)\n"
            " -y <n> verbosity during changing"
            " (0=no,1=errors,2=+warnings,3=debug)\n"
            " -q ... quiet, no control output\n\n");

    fprintf(stderr,
            "Verbosity for analyzing during read:\n"
            " -f[h|H][t|T][m|M][s|S][e|E] ... output print format\n"
            "        h/H ... no header/print header\n"
            "        t/T ... no track header/print track header\n"
            "        m/M ... no Metaevents/print Metaevents\n"
            "        s/S ... no SYSEX/print SYSEX\n"
            "        e/E ... no MIDI Event/print MIDI Event\n"
            "        q/A ... no printing/printing ALL\n"
            "        w/W ... no warnings/Warnings"
            "\n\n");

    fprintf(stderr,
            "Verbosity during checking for notelength and pauselength:\n"
            " -c[p|P][v|V][c|C][q|A] ... check print format\n"
            "        p/P ... no pause-warnings/print pausen-warnings\n"
            "        v/V ... no velocity-statistics/print velocity-statistics\n"
            "        r/R ... no repetition times/ print repetition times\n"
            "        c/C ... no pairing details/print pairing details\n"
            "        q/A ... no printing/printing ALL\n"
            "\n\n");
    fprintf(stderr,
            "Output if execute is set \n"
            " -x ... execute changes for file output (do change)\n"
            " -xf ... execute changes for file output (do change) drop check-errors\n"
            " -o <midi-file-out> ... output to file output\n\n");

    fprintf(stderr,
            "Parameter for checking and changing the MIDIFile:\n"
            " -m <mode> ...  modification mode (default 0)\n"
            "    mode 0: pause, notelength correction\n"
            "       -p <minimum note pause> ... set minimal note pause (default=40) in ms\n"
            "       -l <minimum note duration> ... set minimal note duration (default=70) ms\n"
            "       -d  delete note if minimum length cannot be set instead of leaving to short\n"
//            "    mode 1: pairing, notes in sequence are combined\n"
//            "       -r <repetion time> ... set repetition time (frametime) ms, if 0 measured\n"
//            "       -s <pair veldiff> ... set velocity difference to detect pair (default = 0, -127...127)\n"
//            "       -n <pair mindiff> ... pause if note is quieter with mindiff than note before (default: -127) \n"
//            "       -t <max pair count> ... set maximum pair count (default=2, infinite=0)\n"
//            "       -u <scale vel> ... scale pair velocity (default=0.5, 0..min vil, 1...max_vel)\n"
//            "    mode 2: grouping with sorted velocities over sequences\n"
//            "       -r <repetion time> ... set repetition time (frametime) ms, if 0 measured\n"
//            "       -s <pair veldiff> ... set velocity difference to detect pair (default = 0, -127...127)\n"
//            "       -t <max pair count> ... set maximum pair count (default=2, infinite=0)\n"
//            "    mode 3: grouping with sorted velocities over whole notes\n"
//            "       -r <repetion time> ... set repetition time (frametime) ms, if 0 measured\n"
//            "       -s <pair veldiff> ... set velocity difference to detect pair (default = 0, -127...127)\n"
//            "       -t <max pair count> ... set maximum pair count (default=2, infinite=0)\n"
            "    mode 10: pairing, sequence of notes are combined using duration_table\n"
            "       -r <repetion time> ... set repetition time (frametime) ms, if 0 measured\n"
            "       -s <pair veldiff> ... set velocity difference to detect pair (default = 0, -127...127)\n"
            "       -t <max pair count> ... set maximum pair count (default=2, infinite=0)\n"
            "       -u <scale vel> ... scale pair velocity (default=0.5, 0..min vil, 1...max_vel)\n"
            "    mode 20: dyn pairing, sequence of notes are combined using duration_table sorted by dyn \n"
            "       -r <repetion time> ... set repetition time (frametime) ms, if 0 measured\n"
            "       -s <pair veldiff> ... set velocity difference to detect pair (default = 0, -127...127)\n"
            "       -t <max pair count> ... set maximum pair count (default=2, infinite=0)\n"
            "       -u <scale vel> ... scale pair velocity (default=0.5, 0..min vil, 1...max_vel)\n"
           );

//           "       -n <pair mindiff> ... pause if note is quieter with mindiff than note before (default: -127) \n"
//            "       -n <pair mindiff> ... pause if note is quieter with mindiff than note before (default: -127) \n"

    return;
}


/* === parse arguments === */

void parse_args(int argc,char *argv[])
{
    int i;
    int ret;
    char *pchar;

    for(i=1; i < argc; i++)
    {/* MODE 2 */
        /* option or filename */
        if(argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
            case 'q': /* verbosity */
                quiet=1;
                break;

            case 'f': /* what to print */
                pchar = &(argv[i][1]);
                while(*(pchar++))
                {
                    switch(*pchar)
                    {
                    case 'h':
                        pheader=0;
                        break;
                    case 'H':
                        pheader=1;
                        break;
                    case 't':
                        ptracks=0;
                        break;
                    case 'T':
                        ptracks=1;
                        break;
                    case 'm':
                        pmeta=0;
                        break;
                    case 'M':
                        pmeta=1;
                        break;
                    case 's':
                        psysex=0;
                        break;
                    case 'S':
                        psysex=1;
                        break;
                    case 'e':
                        pmidi=0;
                        break;
                    case 'E':
                        pmidi=1;
                        break;
                    case 'w':
                        pwarn=0;
                        break;
                    case 'W':
                        pwarn=1;
                        break;
                    case 'q':
                        pmidi=psysex=pmeta=ptracks=pheader=0;
                        break;
                    case 'A':
                        pmidi=psysex=pmeta=ptracks=pheader=1;
                        break;
                    default:
                        break;
                    };
                };
                break;

            case 'c': /* what to print */

                pchar = &(argv[i][1]);

                while(*(pchar++))
                {
                    switch(*pchar)
                    {
                    case/* MODE 2 */ 'p':
                        ppause=0;
                        break;
                    case 'P':
                        ppause=1;
                        break;
                    case 'l':
                        plaenge=0;
                        break;
                    case 'L':
                        plaenge=1;
                        break;
                    case 'v':
                 //            "    mode 1: pairing, notes in sequence are combined\n"
//            "       -r <repetion time> ... set repetition time (frametime) ms, if 0 measured\n"
//            "       -s <pair veldiff> ... set velocity difference to detect pair (default = 0, -127...127)\n"
//            "       -n <pair mindiff> ... pause if note is quieter with mindiff than note before (default: -127) \n"
//            "       -t <max pair count> ... set maximum pair count (default=2, infinite=0)\n"
//            "       -u <scale vel> ... scale pair velocity (default=0.5, 0..min vil, 1...max_vel)\n"
//            "    mode 2: grouping with sorted velocities over sequences\n"
//            "       -r <repetion time> ... set repetition time (frametime) ms, if 0 measured\n"
//            "       -s <pair veldiff> ... set velocity difference to detect pair (default = 0, -127...127)\n"
//            "       -t <max pair count> ... set maximum pair count (default=2, infinite=0)\n"
       pminmax=0;
                        break;
                    case 'V':
                        pminmax=1;
                        break;
                    case 'r':
                        pframetimes=0;
                        break;
                    case 'R':
                        pframetimes=1;
                        break;
                    case 'c':
                        ppair=0;
                        break;
                    case 'C':
                        ppair=1;
                        break;
                    case 'q':
                        ppause=pminmax=0;
                        break;
                    case 'A':
                        ppause=pminmax=1;
                        break;
                    };
                };
                break;

            case 'v': /* verbosity */
                if(sscanf(&(argv[i][2]),"%d",&v)  != 1)
                {
                    if((i+1) >= argc)
                    {
                        fprintf(stderr,"-v <n>: no argument n, using n=1");
                        v = 1;
                    }
                    else if(sscanf(&(argv[i+1][0]),"%d",&v)  != 1)
                    {
                        fprintf(stderr,"-v <n>: no n, using n=1");
                        v = 1;
                    }
                    else
                        i++;
                };

                if(v < 0 || v > 3)
                {
                    if(v > 3)v=3;
                    else v=0;
                    fprintf(stderr,"-v <n>: n should be between 0...3 using %d\n",v);
                };
                break;

            case 'w': /* checking verbosity */

                if(sscanf(&(argv[i][2]),"%d",&vv)  != 1)
                {

                    if((i+1) >= argc)
                    {
                        fprintf(stderr,"-w <n>: no argument n, using n=1");
                        vv = 1;
                    }
                    else if(sscanf(&(argv[i+1][0]),"%d",&vv)  != 1)
                    {
                        fprintf(stderr,"-w <n>: no n, using n=1");
                        vv = 1;
                    }
                    else
                        i++;
                };
                if(vv < 0 || vv > 9)
                {
                    if(vv > 9)vv=3;
                    else vv=0;
                    fprintf(stderr,"-v <n>: n should be between 0...3 using %d\n",v);
                };
                break;

            case 'y': /* changing verbosity */

                if(sscanf(&(argv[i][2]),"%d",&vvv)  != 1)
                {

                    if((i+1) >= argc)
                    {
                        fprintf(stderr,"-y <n>: no argument n, using n=1");
                        vvv = 1;
                    }
                    else if(sscanf(&(argv[i+1][0]),"%d",&vvv)  != 1)
                    {
                        fprintf(stderr,"-y <n>: no n, using n=1");
                        vvv = 1;
                    }
                    else
                        i++;
                };

                if(vv < 0 || vv > 9)
                {
                    if(vv > 9)vv=3;
                    else vv=0;
                    fprintf(stderr,"-v <n>: n should be between 0...3 using %d\n",v);
                };
                break;

            case 'o':

                if(argv[i][2] != 0)
                    outfile = &(argv[i][2]);
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-o:cannot get outputname \n");
                    usage(argv);
                    exit(1);
                }
                else
                    outfile = argv[++i];

                break;

            case 'm':

                if(argv[i][2] != 0)
                    ret = atoi(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-m:cannot get mode (must be int) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    ret  = atoi(argv[++i]);

                switch(ret) /* MODES */
                {
                case 0:
                    regel=corr;    /* MODE 0 */
                    break;
                case 1:
                    regel=simple;  /* MODE 1 */
                    break;
                case 2:
                    regel=pairing; /* MODE 2 */
                    break;
                case 3:
                    regel=grouping; /* MODE 3 */
                    break;
                case 10:            /*do_corr_del MODE 10 */
                    regel=pair_dn;
                    break;
                case 20:            /* MODE 20 */
                    regel=pair_dyn;
                    break;
                default:
                    regel=none;
                }
                break;

            case 'p':

                if(argv[i][2] != 0)
                    min_pause = (float) atof(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-p:cannot get minimum pause (must be float) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    min_pause = (float) atof(argv[++i]);
                break;

            case 'l':

                if(argv[i][2] != 0)
                    min_laenge = (float) atof(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-l:cannot get minimum laenge (must be float) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    min_laenge = (float) atof(argv[++i]);

                break;

            case 'd':
                do_corr_del = TRUE;
                break;

            case 'r':

                if(argv[i][2] != 0)
                    frame_time = (float) atof(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-l:cannot get repetition time (must be float) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    frame_time = (float) atof(argv[++i]);
                break;

            case 's':

                if(argv[i][2] != 0)
                    pair_diff_vel = atoi(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-l:cannot get pair difference vel (must be int) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    pair_diff_vel = atoi(argv[++i]);
                break;

            case 't':

                if(argv[i][2] != 0)
                    pair_max = atoi(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-l:cannot get pair max (must be int) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    pair_max = atoi(argv[++i]);
                break;

            case 'u':
                if(argv[i][2] != 0)
                    pair_vel_scale = (float) atof(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-l:cannot get pair scale vel (must be float) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    pair_vel_scale = (float) atof(argv[++i]);
                break;

            case 'n':
                if(argv[i][2] != 0)
                    pair_min_diff = atoi(&(argv[i][2]));
                else if((i+1) >= argc || argv[i+1][0] == 0)
                {
                    fprintf(stderr,"-l:cannot get pair min diff (must be int) \n");
                    usage(argv);
                    exit(1);
                }
                else
                    pair_min_diff = atoi(argv[++i]);
                break;

            case 'x': /* execute change */

                do_change=1;
                if(argv[i][2] == 'f')
                    do_change_force = 1;
                break;

            default:
                fprintf(stderr,"Unkown option -%c \n",argv[i][0]);
            case 'h':
                usage(argv);
                exit(1);
            }
        }
        else
        {
            /* filename */
            if(filename == NULL)
                filename = argv[i];
            else
            {
                fprintf(stderr,"Dont know what to do with %s\n",argv[i]);
                usage(argv);
                exit(1);
            }
        }
    };

}
