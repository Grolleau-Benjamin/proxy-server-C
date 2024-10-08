#include "../includes/server_helper.h"
#include "../includes/utils.h"

#include <regex.h>
#include <stdlib.h>
#include <string.h>

static regex_t ip_port_regex;
static int regex_compiled = 0;

int init_regex() {
  const char* pattern = 
    "^((25[0-5]|"                                 // from 250 to 255
    "2[0-4][0-9]|"                                // from 200 to 249
    "1[0-9]{2}|"                                  // from 100 to 199
    "[1-9]?[0-9]"                                  // from 1 to 99
    ")\\.){3}"                                     // number + '.' 3 times
    "(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9]):" // another number with same rules
    "(6553[0-5]|"                                 // from 65530 to 65535
    "655[0-2][0-9]|"                              // from 65500 to 65529
    "65[0-4][0-9]{2}|"                            // from 65000 to 65499
    "6[0-4][0-9]{3}|"                              // from 60000 to 64999
    "[1-5][0-9]{4}|"                               // fron 10000 to 59999
    "[1-9][0-9]{0,3})$";                           // from 1 to 9999

  int ret = regcomp(&ip_port_regex, pattern, REG_EXTENDED); 
  if (ret != 0) {
    ERROR("Echec during the compilation of the regex\n");
    print_error(ret, "regex");
  }
  
  regex_compiled = 1;
  return 0;
}

void free_regex() {
    if (regex_compiled) {
        regfree(&ip_port_regex);
        regex_compiled = 0;
    }
}

int is_ip_port_format(const char *host, char **ip, char **port) {
  if (!regex_compiled) {
    ERROR("Regex is not compile. Call init_regex() before.\n");
    print_error(-1, "regex_not_compiled");
  }
  
  if (regexec(&ip_port_regex, host, 0, NULL, 0) == 0) {
    char* split_pos = strchr(host, ':');
    if(split_pos == NULL) return 0;
    size_t ip_len = split_pos - host;
    *ip = strndup(host, ip_len);
    *port = strdup(split_pos + 1); 
    return 1;
  }
  return 0;
}
