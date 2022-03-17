/***************************************************************************
RespirAVO/UnisciMisure
Copyright 2022 3C A. Avogadro & RespirAVO Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
***************************************************************************/


#ifndef CSVLIB_H
#define CSVLIB_H

    #include <filelib.h>
    #include <stdbool.h>

    #define MAX_STR_LEN 200     // Dimensione massima di una stringa


    typedef struct CSVFileDefinition {
        FILE*        File;
        const char** Columns;
        const char   Separator;
        const bool   ReadOnly;
    } csv_t;


    csv_t       csvOpenRead         (const char* __path);
    csv_t       csvOpenWrite        (const char* __path, const char* headingCols, ...);
    csv_t       csvAppendEntry      (csv_t* __csv, const char* entryCols, ...);
    char**      csvGetEntry         (csv_t* __csv, int n);
    char***     csvGetEntrys        (csv_t* __csv);

#endif // CSVLIB_H