#ifndef RULES_H
#define RULES_H

#include <stdlib.h>

/**
 * @file rules.h
 * @brief Definitions and structures for managing content filtering rules.
 *
 * This file defines the structures and functions used to manage and manipulate
 * content filtering rules, such as banning domains or words based on categories.
 */

/**
 * @brief Format of the rules configuration file.
 *
 * The rules configuration file is expected to follow this format:
 * 
 * ```
 * # Some comments
 * 
 * [categorie]
 * BAN_DOMAIN some_domain.com
 * BAN_WORD some_word
 * ```
 */

#define MAX_STRING_LEN 255

/**
 * @brief Structure representing a filtering category.
 *
 * This structure holds the name of the category and lists of banned domains
 * and words associated with that category.
 */
typedef struct {
  char name[MAX_STRING_LEN];       /**< Name of the category */
  char** ban_domain_list;          /**< List of banned domains */
  char** ban_word_list;            /**< List of banned words */
  size_t domain_count;             /**< Number of banned domains */
  size_t word_count;               /**< Number of banned words */
} categories;

/**
 * @brief Structure representing the set of rules.
 *
 * This structure contains an array of categories and the number of rules (categories)
 * loaded from the configuration file.
 */
typedef struct {
  categories* rules;               /**< Array of rules (categories) */
  size_t nb_rules;                 /**< Number of categories (rules) */
} rules_t;

/**
 * @brief Global variable holding the filtering rules.
 *
 * This global variable holds the set of rules that are loaded from the configuration
 * file and used to filter domains and words based on categories.
 */
extern rules_t rules;

int init_rules(const char* filename);
void free_rules();

#endif
