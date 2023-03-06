#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "dllist.h"
#include "fields.h"

typedef struct MakeFile {
    char* Executable;
    unsigned Max_C, Max_H, Max;
    Dllist _Source_Files;
    Dllist _Header_Files;
    Dllist _Libraries;
    Dllist _Compile_Flags;
}*MakeFile;

void fill_dllist(IS line, Dllist fill, unsigned type, char skip);
void file_makes(MakeFile makefile);

int main(int argc, char** argv) {
    MakeFile makefile;
    IS file;

    // if no .fm given then use fmakefile else use given
    file = new_inputstruct((argc < 2) ? "fmakefile" : argv[1]);

    // initialize a makefile
    makefile = malloc(sizeof(*makefile));
    makefile->Executable = strdup("this is not the file you're looking for");
    makefile->Max_C = 0;
    makefile->Max_H = 0;
    makefile->Max = 0;
    makefile->_Source_Files = new_dllist();
    makefile->_Header_Files = new_dllist();
    makefile->_Libraries = new_dllist();
    makefile->_Compile_Flags = new_dllist();

    // read in the file and get all the values
    while (get_line(file) >= 0) {
        switch (file->fields[0][0]) {
        case 'E':
            if (file->NF > 2 || (strcmp(makefile->Executable, "this is not the file you're looking for") != 0)) {
                fprintf(stderr, "fmakefile (%u) cannot have more than one E line\n", file->line);
                exit(1);
            }
            free(makefile->Executable);
            makefile->Executable = malloc(sizeof(file->fields[1]));
            strcpy(makefile->Executable, file->fields[1]);
            break;
        case 'C':
            fill_dllist(file, makefile->_Source_Files, makefile->Max_C, 'n');
            makefile->Max = (makefile->Max < makefile->Max_C) ? makefile->Max_C : makefile->Max;
            break;
        case 'H':
            fill_dllist(file, makefile->_Header_Files, makefile->Max_H, 'n');
            makefile->Max = (makefile->Max < makefile->Max_H) ? makefile->Max_H : makefile->Max;
            break;
        case 'F':
            fill_dllist(file, makefile->_Compile_Flags, 0, 'y');
            break;
        case 'L':
            fill_dllist(file, makefile->_Libraries, 0, 'y');
            break;
        default:
            break;
        }
    }
    jettison_inputstruct(file);

    if (strcmp(makefile->Executable, "this is not the file you're looking for") == 0) {
        fprintf(stderr, "No executable specified\n");
        exit(1);
    }

    file_makes(makefile);

    return 0;
}

void fill_dllist(IS line, Dllist fill, unsigned type, char skip) {
    struct stat buf;
    // read through the line and add to dllist
    for (unsigned words = 1; words < line->NF; words++) {
        dll_append(fill, new_jval_s(strdup(line->fields[words])));

        // if the dllist is being filled with files that need to be compiled 
        // stat and save youngest file date
        if (skip == 'n') {
            // if file does not exist
            if (stat(line->fields[words], &buf) != 0) {
                printf("file at line%d does not exist", line->NF);
                exit(1);
            }
            type = (buf.st_mtime > type) ? buf.st_mtime : type;
        }
    }
}


void file_makes(MakeFile makefile) {
    Dllist ptr;
    struct stat buf, buf2;
    // char* tmp, * oFiles, * gcc_o, * gcc_c;
    char* oFiles, * tmp, gcc_o[100], gcc_c[100];
    int make = 0;
    int chkstat;
    strcat(gcc_c, "gcc -c ");
    strcat(gcc_o, "gcc -o ");

    // creating the syscalls
    strcat(gcc_o, makefile->Executable);

    // get flags
    if (dll_empty(makefile->_Compile_Flags) == 0) {
        dll_traverse(ptr, makefile->_Compile_Flags) {
            strcat(gcc_o, ptr->val.s);
            strcat(gcc_c, ptr->val.s);
        }
    }

    // get object files;
    dll_traverse(ptr, makefile->_Source_Files) {
        oFiles = strdup(ptr->val.s);
        oFiles[strlen(ptr->val.s) - 1] = 'o';
        stat(ptr->val.s, &buf2);
        if (stat(oFiles, &buf) != 0 || (buf.st_mtime < buf2.st_mtime) || (buf.st_mtime < makefile->Max_H)) {
            make = 1;
            tmp = strdup(gcc_c);
            strcat(tmp, ptr->val.s);
            printf("%s\n", tmp);
            if (system(tmp) != 0) {
                fprintf(stderr, "Command failed.  Exiting\n");
                exit(1);
            }

        }
        makefile->Max = (buf.st_mtime < makefile->Max) ? makefile->Max : buf.st_mtime;
        strcat(gcc_o, " ");
        strcat(gcc_o, oFiles);
    }

    chkstat = stat(makefile->Executable, &buf);
    if (chkstat == 0 && makefile->Max < buf.st_mtime) {
        printf("%s up to date\n", makefile->Executable);
        exit(0);
    }
    if (chkstat != 0 || make == 1 || makefile->Max > buf.st_mtime) {
        printf("%s\n", gcc_o);
        if (system(gcc_o) != 0) {
            fprintf(stderr, "Command failed.  Exiting\n");
            exit(1);
        }
    }
}