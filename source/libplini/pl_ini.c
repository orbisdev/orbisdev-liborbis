// Copyright 2007-2015 Akop Karapetyan
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pl_ini.h"
#include <ps4sdk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <debugnet.h>
#include <orbisNfs.h>
#include <sys/fcntl.h>
#define PL_MAX_LINE_LENGTH 512

typedef struct pl_ini_pair_t
{
  char *key;
  char *value;
  struct pl_ini_pair_t *next;
} pl_ini_pair;

typedef struct pl_ini_section_t
{
  char *name;
  struct pl_ini_pair_t *head;
  struct pl_ini_section_t *next;
} pl_ini_section;

pl_ini_section* create_section(const char *name);
pl_ini_pair* create_pair(char *string);
pl_ini_section* find_section(const pl_ini_file *file,
                                    const char *section_name);
pl_ini_pair* find_pair(const pl_ini_section *section,
                              const char *key_name);
pl_ini_pair* locate(const pl_ini_file *file,
                           const char *section_name,
                           const char *key_name);
pl_ini_pair* locate_or_create(pl_ini_file *file,
                                     const char *section_name,
                                     const char *key_name);

int pl_ini_create(pl_ini_file *file)
{
  file->head = NULL;
  return 1;
}
int pl_ini_load_from_nfs(pl_ini_file *file, const char *path)
{
    file->head = NULL;

    int fd;
    fd = orbisNfsOpen(path,O_RDONLY, 0777);
  if(fd<0)
      return 0;

    pl_ini_section *current_section = NULL;
    pl_ini_pair *tail;

    char string[PL_MAX_LINE_LENGTH],
         name[PL_MAX_LINE_LENGTH];
    char *ptr;
    int len;

    /* Create unnamed section */
    current_section = NULL;
    tail = NULL;
  int ret=orbisNfsRead(fd,string,sizeof(string));
    if(ret>0)
    {
      /* TODO: Skip whitespace */
      /* New section */
      if (string[0] == '[')
      {
        if ((ptr = strrchr(string, ']')))
        {
          len = ptr - string - 1;
          strncpy(name, string + 1, len);
          name[len] = '\0';

          if (!current_section)
            current_section = file->head = create_section(name);
          else
          {
            current_section->next = create_section(name);
            current_section = current_section->next;
          }

          tail = NULL;
        }
      }
      else if (string[0] =='#'); /* Do nothing - comment */
      else
      {
        /* No section defined - create empty section */
        if (!current_section)
        {
          current_section = create_section(strdup(""));
          file->head = current_section;
          tail = NULL;
        }

        pl_ini_pair *pair = create_pair(string);
        if (pair)
        {
          if (tail) tail->next = pair;
          else current_section->head = pair;
          tail = pair;
        }
      }
    }

    orbisNfsClose(fd);
    return 1; 
  
}
int pl_ini_load_from_data(pl_ini_file *file,
                const char *path)
{ 
  file->head = NULL;

  int fd;
  fd=open(path,O_RDONLY, 0777);
  if(fd<0) 
    return 0;

  pl_ini_section *current_section = NULL;
  pl_ini_pair *tail;

  char string[PL_MAX_LINE_LENGTH],
       name[PL_MAX_LINE_LENGTH];
  char *ptr;
  int len;

  /* Create unnamed section */
  current_section = NULL;
  tail = NULL;
  int ret=read(fd,string,sizeof(string));
  if(ret>0)
  {
    /* TODO: Skip whitespace */
    /* New section */
    if (string[0] == '[')
    {
      if ((ptr = strrchr(string, ']')))
      {
        len = ptr - string - 1;
        strncpy(name, string + 1, len);
        name[len] = '\0';

        if (!current_section)
          current_section = file->head = create_section(name);
        else
        {
          current_section->next = create_section(name);
          current_section = current_section->next;
        }

        tail = NULL;
      }
    }
    else if (string[0] =='#'); /* Do nothing - comment */
    else
    {
      /* No section defined - create empty section */
      if (!current_section)
      {
        current_section = create_section(strdup(""));
        file->head = current_section;
        tail = NULL;
      }

      pl_ini_pair *pair = create_pair(string);
      if (pair)
      {
        if (tail) tail->next = pair;
        else current_section->head = pair;
        tail = pair;
      }
    }
  }

  close(fd);
  return 1;
}



int pl_ini_load(pl_ini_file *file,
                const char *path)
{ 
  file->head = NULL;

  FILE *stream;
  if (!(stream = fopen(path, "r"))) 
    return 0;

  pl_ini_section *current_section = NULL;
  pl_ini_pair *tail;

  char string[PL_MAX_LINE_LENGTH],
       name[PL_MAX_LINE_LENGTH];
  char *ptr;
  int len;

  /* Create unnamed section */
  current_section = NULL;
  tail = NULL;
  char *retfget= fgets(string, sizeof(string), stream);
  int retfeof=feof(stream);
  //debugNetPrintf(3,"[PL] before while refget=%s retfeof=%d\n",retfget,retfeof);

  while(retfget)
  {


    /* TODO: Skip whitespace */
    /* New section */
    //debugNetPrintf(3,"[PL] %s\n%s\n",__FUNCTION__,string);
    if (string[0] == '[')
    {
      if ((ptr = strrchr(string, ']')))
      {
        len = ptr - string - 1;
        strncpy(name, string + 1, len);
        name[len] = '\0';

        if (!current_section)
          current_section = file->head = create_section(name);
        else
        {
          current_section->next = create_section(name);
          current_section = current_section->next;
        }

        tail = NULL;
      }
    }
    else if (string[0] =='#'); /* Do nothing - comment */
    else
    {
      /* No section defined - create empty section */
      if (!current_section)
      {
        current_section = create_section(strdup(""));
        file->head = current_section;
        tail = NULL;
      }

      pl_ini_pair *pair = create_pair(string);
      if (pair)
      {
        if (tail) tail->next = pair;
        else current_section->head = pair;
        tail = pair;
      }
    }
      retfget= fgets(string, sizeof(string), stream);
   retfeof=feof(stream);
   //debugNetPrintf(3,"[PL] refget=%s retfeof=%d\n",retfget,retfeof);
  }

  fclose(stream);
  return 1;
}

int pl_ini_save(const pl_ini_file *file,
                const char *path)
{
  FILE *stream;
  if (!(stream = fopen(path, "w")))
    return 0;

  pl_ini_section *section;
  pl_ini_pair *pair;

  for (section = file->head; section; section = section->next)
  {
    fprintf(stream, "[%s]\n", section->name);
    for (pair = section->head; pair; pair = pair->next)
      fprintf(stream, "%s=%s\n", pair->key, pair->value);
  }

  fclose(stream);
  return 1;
}

int pl_ini_get_int(const pl_ini_file *file,
                   const char *section,
                   const char *key,
                   int default_value)
{
  pl_ini_pair *pair = locate(file, section, key);
  return (pair) ? atoi(pair->value) : default_value;
}

int pl_ini_get_string(const pl_ini_file *file, 
                      const char *section, 
                      const char *key,
                      const char *default_value,
                      char *copy_to,
                      int dest_len)
{
  pl_ini_pair *pair = locate(file, section, key);
  if (pair)
  {
    strncpy(copy_to, pair->value, dest_len);
    return 1;
  }
  else if (default_value)
  {
    strncpy(copy_to, default_value, dest_len);
    return 1;
  }

  return 0;
}

void pl_ini_set_int(pl_ini_file *file,
                    const char *section,
                    const char *key,
                    int value)
{
  pl_ini_pair *pair;
  if (!(pair = locate_or_create(file, section, key)))
    return;

  /* Replace the value */
  if (pair->value)
    free(pair->value);

  char temp[64];
  snprintf(temp, 63, "%i", value);
  pair->value = strdup(temp);
}

void pl_ini_set_string(pl_ini_file *file,
                       const char *section,
                       const char *key,
                       const char *string)
{
  pl_ini_pair *pair;
  if (!(pair = locate_or_create(file, section, key)))
    return;

  /* Replace the value */
  if (pair->value)
    free(pair->value);
  pair->value = strdup(string);
}

void pl_ini_destroy(pl_ini_file *file)
{
  pl_ini_section *section, *next_section;
  pl_ini_pair *pair, *next_pair;

  for (section = file->head; section; section = next_section)
  {
    next_section = section->next;

    if (section->name) 
      free(section->name);
    for (pair = section->head; pair; pair = next_pair)
    {
      next_pair = pair->next;
      if (pair->key)
        free(pair->key);
      if (pair->value)
        free(pair->value);
      free(pair);
    }

    free(section);
  }
}

pl_ini_section* find_section(const pl_ini_file *file,
                                    const char *section_name)
{
  pl_ini_section *section;
  for (section = file->head; section; section = section->next)
    if (strcmp(section_name, section->name) == 0)
      return section;

  return NULL;
}

pl_ini_pair* find_pair(const pl_ini_section *section,
                              const char *key_name)
{
  pl_ini_pair *pair;
  for (pair = section->head; pair; pair = pair->next)
    if (strcmp(pair->key, key_name) == 0)
      return pair;

  return NULL;
}

pl_ini_pair* locate(const pl_ini_file *file,
                           const char *section_name,
                           const char *key_name)
{
  pl_ini_section *section;
  if (!(section = find_section(file, section_name)))
    return NULL;

  return find_pair(section, key_name);
}

pl_ini_pair* locate_or_create(pl_ini_file *file,
                                     const char *section_name,
                                     const char *key_name)
{
  pl_ini_section *section = find_section(file, section_name);
  pl_ini_pair *pair = NULL;

  if (section)
    pair = find_pair(section, key_name);
  else
  {
    /* Create section */
    section = create_section(section_name);

    if (!file->head)
      file->head = section;
    else
    {
      pl_ini_section *s;
      for (s = file->head; s->next; s = s->next); /* Find the tail */
      s->next = section;
    }
  }

  if (!pair)
  {
    /* Create pair */
    pair = (pl_ini_pair*)malloc(sizeof(pl_ini_pair));
    pair->key = strdup(key_name);
    pair->value = NULL;
    pair->next = NULL;

    if (!section->head)
      section->head = pair;
    else
    {
      pl_ini_pair *p;
      for (p = section->head; p->next; p = p->next); /* Find the tail */
      p->next = pair;
    }
  }

  return pair;
}

pl_ini_section* create_section(const char *name)
{
  pl_ini_section *section
    = (pl_ini_section*)malloc(sizeof(pl_ini_section));
  section->head = NULL;
  section->next = NULL;
  section->name = strdup(name);

  return section;
}

pl_ini_pair* create_pair(char *string)
{
  char *ptr;
  if (!(ptr = strchr(string, '=')))
    return NULL;

  int len;
  char *name, *value;

  /* Copy NAME */
  len = ptr - string;
  if (!(name = (char*)malloc(sizeof(char) * (len + 1))))
    return NULL;
  strncpy(name, string, len);
  name[len] = '\0';

  /* Copy VALUE */
  if (!(value = strdup(ptr + 1)))
  {
    free(name);
    return NULL;
  }

  len = strlen(value);
  if (value[len - 1] == '\n') value[len - 1] = '\0';

  /* Create struct */
  pl_ini_pair *pair = (pl_ini_pair*)malloc(sizeof(pl_ini_pair));

  if (!pair)
  {
    free(name);
    free(value);
    return NULL;
  }

  pair->key = name;
  pair->value = value;
  pair->next = NULL;

  return pair;
}