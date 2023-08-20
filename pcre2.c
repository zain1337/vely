// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// Regex (pcre2)-related module
//


#include "vely.h"
#include <dlfcn.h>
#include <gnu/lib-names.h>

static bool pcre2_resolved = false; // used for both pcre2 and libc regex.
// These are dlopen-loaded for both pcre2 and libc regex
static int (*vv_pcre2_regcomp)(regex_t *preg, const char *regex, int cflags);
static int (*vv_pcre2_regexec)(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
static size_t (*vv_pcre2_regerror)(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
static void (*vv_pcre2_regfree)(regex_t *preg);


// Free preg for regex, this is because for pcre2 vs glibc, it's called from v1 parser and it must be 
// compiled into 2 different so libs (velypcre2 and velypcre2glibc), and the implementations are different.
void vely_regfree(regex_t *preg)
{
    vv_pcre2_regfree (preg); 
}

//
// Regex support. look_here is the string examined. find_this is what we look to find in look_here.
// replace is the replacement regex string for find_this if found. res is the result of this
// replacement.
// If find_this is found in look_here, then we return number of matches found (and replaced if replace is !NULL).
// If 'replace' is NULL, then we don't replace. Otherwise, all occurances are replaced.
// If case_insensitive is 1, then case doesn't matter (0 otherwise).
// If single_match is 1, then only a single match/replacement is done.
// If utf8 is 1, then characters are treated as UTF8
// reslen is the length of the replaced result, or it could be NULL if not needed
// cached is a compiled regex_t. If NULL, it's not used. If *cached is NULL, it's assigned. If *cached is not-NULL, it's used.
//
num vely_regex(char *look_here, char *find_this, char *replace, char **res, num utf8, num case_insensitive, num single_match, regex_t **cached, num *reslen)
{
    VV_TRACE("");
    num reg_ret=0;
    regex_t *reg;
    regex_t lreg; // local compiled regex_t used when cache is not used

    // If not using glibc-regex, load symbols with dlopen
#ifndef VV_C_POSIXREGEX
    // Since PCRE2 in earlier days used the same symbols as glibc regex, you could never know which library will
    // be used, resulting in a guaranteed SIGSEGV. We will read the actual library and use symbols from it, making
    // sure regex from glibs is never called.
    if (!pcre2_resolved) 
    {
        // Open libpcre2-posix.so, should be in path if installed
        void *lpcre2 = dlopen("lib" PCRE2_LIB_DL, RTLD_NOW);
        if (lpcre2 == NULL) vely_report_error ("Cannot find PCRE2 library [%s]", dlerror());
        // Get symbols, check for non pcre2_ in case of earlier version (ubuntu 18, debian 10, redhat 8 etc)
        vv_pcre2_regcomp = dlsym(lpcre2, "pcre2_regcomp");
        if (vv_pcre2_regcomp == NULL) vv_pcre2_regcomp = dlsym(lpcre2, "regcomp");
        vv_pcre2_regexec = dlsym(lpcre2, "pcre2_regexec");
        if (vv_pcre2_regexec == NULL) vv_pcre2_regexec = dlsym(lpcre2, "regexec");
        vv_pcre2_regerror = dlsym(lpcre2, "pcre2_regerror");
        if (vv_pcre2_regerror == NULL) vv_pcre2_regerror = dlsym(lpcre2, "regerror");
        vv_pcre2_regfree = dlsym(lpcre2, "pcre2_regfree");
        if (vv_pcre2_regfree == NULL) vv_pcre2_regfree = dlsym(lpcre2, "regfree");
        // Check we got them all
        if (vv_pcre2_regcomp == NULL || vv_pcre2_regexec == NULL || vv_pcre2_regerror == NULL || vv_pcre2_regfree == NULL) vely_report_error ("Cannot resolve PCRE2 library symbols");
        pcre2_resolved = true; // do this only once per process
    }
#else
    // Some distros will link in pcre2 even though libc is present, because regcomp/... is the same function names used
    // for both for prior versions of pcre2. It was not the best idea to duplicate function names in pcre2 prior to 
    // version 10.37.
    if (!pcre2_resolved) 
    {
        // Open libpcre2-posix.so, should be in path if installed
        void *lposix = dlopen(LIBC_SO, RTLD_NOW);
        if (lposix == NULL) vely_report_error ("Cannot find libc library [%s]", dlerror());
        // Get symbols for libc regex
        vv_pcre2_regcomp = dlsym(lposix, "regcomp");
        vv_pcre2_regexec = dlsym(lposix, "regexec");
        vv_pcre2_regerror = dlsym(lposix, "regerror");
        vv_pcre2_regfree = dlsym(lposix, "regfree");
        // Check we got them all
        if (vv_pcre2_regcomp == NULL || vv_pcre2_regexec == NULL || vv_pcre2_regerror == NULL || vv_pcre2_regfree == NULL) vely_report_error ("Cannot resolve libc regex library symbols");
        pcre2_resolved = true; // do this only once per process
    }
#endif

#if defined(VV_C_POSIXREGEX)
    num rflags = REG_EXTENDED;
#else
    num rflags = REG_EXTENDED|REG_DOTALL|REG_UCP|REG_NEWLINE;
#endif
    if (case_insensitive) rflags |= REG_ICASE;
#if defined(VV_C_POSIXREGEX)
    VV_UNUSED(utf8);
#else
    if (utf8) rflags |= REG_UTF;
#endif

    // no cache, use local regex_t
    if (cached == NULL)
    {
        reg = &lreg;
        reg_ret = vv_pcre2_regcomp(reg, find_this, rflags);
        VV_TRACE("No regex caching");
    }
    // make compiled regex and cache it
    else if (cached != NULL && *cached == NULL)
    {
        //
        // Allocating cached regex CANNOT be vely_malloc as it MUST survive cross-request caching!
        // It is only released when the program ends
        //
        reg = (regex_t*) malloc (sizeof(regex_t));
        reg_ret = vv_pcre2_regcomp(reg, find_this, rflags);
        *cached = reg;
        VV_TRACE("Creating new regex for future caching");
    }
    // this is using cached compiled regex, no need to do anything other than use it
    else if (cached != NULL && *cached != NULL)
    {
        reg_ret = 0;
        reg = *cached;
        VV_TRACE("Using cached regex");
    }

    // see if regex 'compiled' structure can be done
    if(!reg_ret) {
        // get the number of () subexpressions in the pattern
        num sub_num = (num)(reg->re_nsub); 

        // regmatch_t is used for subexpressions, i.e. () expressions
        regmatch_t subexp[sub_num + 1];

        char *current_replace, *head_replace;

        // this is used to build current replacement
        char *replace_res;
        num replace_res_len = 0; // track length of replacing, it's faster than strcat and such
        num replace_res_offset = 0; // offset in *res where the above replacing ended (and new will begin), but because we will re-alloc
                                    // *res, we need this offset to get a correct pointer to where to continue again after the realloc
        num res_allocated = 0; // how many bytes allocated for the result
        num res_needed = 0; // how many bytes actually currently needed for the result
        if (replace != NULL) 
        {
            // start with some minimal memory for replacement, more than we need, but okay
            replace_res = (char *)vely_malloc(res_allocated = res_needed = 1 );
            *replace_res= 0;
            // res is the final result
            *res = replace_res;
        }
        else replace_res = ""; // no purpose other than to satisfy a compiler

        // a macro to add memory every time replacement grows. We add 512 bytes to allow growth of replacement string evertime it comes close
        // we need to do this because the final length of replacement string can be quite large. 
        // For example 'replace' could have in it \1\1\1\.... many times
        // and \1 from look_here (as directed by find_this) could be a big chunk of look_here.
        // So it could go at least square in size. The only way to allocate
        // enough but not go crazy with memory is something like this.
#define VV_ALLOC_REPLACE(addlen) {res_needed+=((addlen)+1); if (res_allocated <= res_needed) { replace_res_offset = replace_res-*res; *res = (char*)vely_realloc (*res, res_allocated=(res_needed+512)); /* **8** */ replace_res = *res+replace_res_offset; /* **9** */ } }

        num tot_match = 0; // total number of matches
        char *last_look_here = (char*)look_here; // this is what we currently look at (a substring of original look_here)

        //
        // subexp[0] is special, it show start(rm_so) and end(rm_eo) of the matched pattern.
        // then subexp[1,2,..] relate to subexpressions (meaning () ) within the matched pattern.
        //
        // we look for pattern matches until there are none left
        // last_look_here gets adjusted to just beyond the last found pattern match with each loop
        // Also, the array subexp now gets filled to have the actual locations of each subexpression with
        // last_look_here
        // We are in a loop here because there can be many matches, each having a bunch of subexpressions inside.
        //
        // So subexp[0] is beginning and ending offset of the whole match; subexp[1], subexp[2] etc are the locations
        // of subexpressions within said match (i.e. within subexp[0].rm_so and subexp[0].rm_eo)
        //
        // last_look_here is either the beginning of look_here or just passed the previous match found in look_here,
        // it's the current place where we search for the next match.
        //
        //
        while (*last_look_here != 0 && vv_pcre2_regexec(reg, last_look_here, sub_num + 1, subexp, 0) == 0) 
        {
            tot_match++; 
            if (replace == NULL) // just search, no replace
            {
                last_look_here = last_look_here + subexp[0].rm_eo; // just move past the first match and look for next
                if (single_match == 1) break;
                continue; //look for next match if not replacing
            }

            current_replace = replace; // points to end of \1 or \2... in replace (subexpression)
            head_replace = current_replace; // points to current beginning of \1 or \2... in replace

            // Copy from look_for to replace everything from the end of the previous match right up until the 
            // beginning of the next match
            // rm_so is offsets to where \1, \2 etc. are
            VV_ALLOC_REPLACE(subexp[0].rm_so);
            strncpy(replace_res+replace_res_len, last_look_here, subexp[0].rm_so); // copy from current search point to where match is 
            replace_res[replace_res_len += subexp[0].rm_so] = 0;

            //
            // copy current_replace (replacement) to output, basically copy transformed match. 
            // We stop at each subexpression, and we copy matches for them
            // and if some not used, that's fine
            //
            // HERE is how Vely statement is stacked up here:
            //
            // pattern-match 'find_this' in 'look_here' replace-with 'replace'
            //
            // search-string "SOME (.*) IS ([^ ]+)" in "WOW SOME THING IS GOOD HERE FOR SURE AND SOME THING IS NOT"  
            //          replace-with "THINGS ARE  \2 YES!" result define res status define st
            //
            // so 'find_this' is "SOME (.*) IS ([^ ]+)"
            //
            // '(last_)look_here' is "WOW SOME THING IS GOOD HERE FOR SURE" and next one is "SOME THING IS NOT"
            //
            // 'replace' is "THINGS ARE  \2 YES!"
            //
            // *res is result
            //
            // So in general:
            //
            // 'replace' is like replace1-\2-replace2-\1-replace3-\3...replace-N where replaceX are copied verbatim
            // and \N are replaced with (subN) below
            //
            // 'find_this' is find1(sub1)find2(sub2)find3(sub3)...
            //
            // 'last_look_here' is within look_here where 'find_this' is found, and subext[0] points to matches found (for each 
            // loop above), and subext[].rm_so/rm_eo point to sub1,sub2...  within each match
            // but with subext[1] being the first sub1 (subext[0] is the whole matched pattern).
            //


            //
            // Note: 'replace' can have \1 \2 \3 etc. in any order, AND some can be missed, so for example \2 may not be used. 
            // We allow for any of it.
            //
            char sdig; // single digit backreference, 1 if yes 0 if not
            num actsub; // the actual subexpression backreference (1-23), if 0 there's no backreference
            // 
            // we do not count how many subexpressions are here, nor compare to the total, because the same one can be repeated
            // many times i.e. ab\1cd\1ef\2gh\1 etc.
            //
            // In this loop, we go through replacement string, and look for \1 \2 etc. We copy all literal
            // text and we substitute \1 with subexpression1 etc. for all of them within a single match, and
            // the above loop gives us one match after the next
            while (1)
            {
                sdig = 0;
                actsub = 0;
                while (*head_replace) // make sure we stop at the end of string
                {
                    if (*head_replace == '\\' && isdigit(*(head_replace+1))) // check for \<digit>
                    {
                        // this may be a backreference to subexpression
                        if (sub_num > 9) // can there be two digits after \  ?
                        {
                            // check for two digits if there's more than 9 subexpressions
                            if (isdigit(*(head_replace+2)))
                            {
                                actsub = (head_replace[1]-'0')*10 +  (head_replace[2]-'0'); // value in \<digit><digit>
                                if (actsub > sub_num) sdig = 1; // if value out of range of subexpressions, look at one digit only
                            } else sdig = 1; // no second digit, so single digit in \<digit>
                        } else sdig = 1; // we only have less than 10 subexpressions, so it must be a single digit \<digit>
                        if (sdig == 1)
                        {
                            actsub = head_replace[1]-'0'; // calculate single digit subexpression index
                            if (actsub > sub_num) actsub = 0; // could be \0 which is invalid, check if out of range
                        }
                        if (actsub < 1) { head_replace+=2; continue; } // move on if \N not valid, or it's \0 or \00
                                                                       // this is if no proper \XY found, we get passed first digit
                        else 
                        {
                            // regardless of whether there's one or two digits, we do not advance head_replace here
                            // because copying below relies on head_replace being the very first char where backreference
                            // was found. Advancing 2 or 3 bytes is done below.
                            break; // found valid \X(Y) reference
                        }
                    } else head_replace++; 
                }
                if (*head_replace == 0) // we reached the end of replace string 
                {
                    VV_ALLOC_REPLACE(head_replace - current_replace);
                    // copy all from previous subexpression to the end of replace to result, or the whole thing if no subexpressions
                    strncpy(replace_res+replace_res_len, current_replace, head_replace - current_replace); 
                    replace_res[replace_res_len+=head_replace - current_replace] = 0;
                    break; // we reached the end of replace, move on the next match
                }
                if (actsub != 0) // we found a subexpression within this particular match
                {
                    VV_ALLOC_REPLACE(head_replace - current_replace);
                    // first copy from the previous end of subexpression (or the beginning) to the beginning of this new subexpression
                    strncpy(replace_res+replace_res_len, current_replace, head_replace - current_replace); 
                    replace_res[replace_res_len+=head_replace - current_replace] = 0;
                    // now copy the actual subexpression found in the last_look_here to result
                    // rm_eo is offset of first char after found subexpression, 
                    // rm_so is where it starts, rm_eo-rm_so is the length of found subexpression
                    VV_ALLOC_REPLACE(subexp[actsub].rm_eo - subexp[actsub].rm_so);
                    strncpy(replace_res+replace_res_len, last_look_here + subexp[actsub].rm_so, subexp[actsub].rm_eo - subexp[actsub].rm_so); // **4**
                    replace_res[replace_res_len+=subexp[actsub].rm_eo - subexp[actsub].rm_so] = 0;
                    head_replace += (sdig == 1 ? 2:3); // advance properly based on whether it's \X or \XY, so 2 or 3 chars
                    current_replace = head_replace; //  skip current subexpression and  move on 
                }
            }

            last_look_here = last_look_here + subexp[0].rm_eo; // continue matching from here, this is just passed the recognized string in '(last_)look_here'
            if (single_match == 1) break;

        }
        if (replace != NULL) // do this if we're replacing
        {
            // whatever is left over after the last match must be copied over to the result
            num len_of_reminder;
            VV_ALLOC_REPLACE(len_of_reminder = strlen(last_look_here));
            memcpy(replace_res+replace_res_len, last_look_here, len_of_reminder+1); 
            if (reslen != NULL) *reslen = replace_res_len+len_of_reminder; // +1 in memcpy above is to copy null-char
                                                                           // which we don't count in string length
        }

        // DONE with replacement

        if (cached == NULL) vv_pcre2_regfree (reg); // free vv_pcre2_regcomp() resources, but only if not cached
        return tot_match; 
        
    }
    else
    {
        // this means the patterns was not syntactically correct
        char regmsg[400];
        vv_pcre2_regerror(reg_ret, reg, regmsg, sizeof(regmsg)); // get error message and error out
        // free reg *after* vv_pcre2_regerror
        if (cached != NULL) *cached = NULL; // if tried to cache, invalidate as it is ... invalid
        // Should not free it if it's a failure. This is different from stdlibc's regex, where you do need to.
#ifdef VV_C_POSIXREGEX
        vv_pcre2_regfree (reg); // do not free vv_pcre2_regcomp resources when it's an error, unless glibc regex
#endif
        if (cached != NULL) free (reg); // if tried to cache, dealloc it
        vely_report_error ("Error in regular expression [%lld], message [%s]", reg_ret, regmsg);
        return -1; // will never actually get here; just to satisfy pedantic compiler
    }
}


