#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/rules.h"
#include "../includes/utils.h"

void create_test_rules_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    assert(file != NULL);

    fprintf(file, "# Test rules configuration file\n");
    fprintf(file, "[Categorie1]\n");
    fprintf(file, "BAN_DOMAIN example.com\n");
    fprintf(file, "BAN_WORD secret\n");
    fprintf(file, "[Categorie2]\n");
    fprintf(file, "BAN_DOMAIN another-example.com\n");
    fprintf(file, "BAN_WORD forbidden\n");

    fclose(file);
}

void test_init_rules() {
    INFO("Testing init_rules...\n");

    const char* rules_filename = "test_rules.rules";
    create_test_rules_file(rules_filename);

    int result = init_rules(rules_filename);
    assert(result == 0);
    INFO("\tsuccess: Rules file has been loaded successfully\n");

    assert(rules.nb_rules == 2);
    INFO("\tsuccess: Correct number of categories loaded\n");

    categories* cat1 = &rules.rules[0];
    assert(strcmp(cat1->name, "Categorie1") == 0);
    assert(cat1->domain_count == 1);
    assert(cat1->word_count == 1);
    assert(strcmp(cat1->ban_domain_list[0], "example.com") == 0);
    assert(strcmp(cat1->ban_word_list[0], "secret") == 0);
    INFO("\tsuccess: Category 1 has been loaded correctly\n");

    categories* cat2 = &rules.rules[1];
    assert(strcmp(cat2->name, "Categorie2") == 0);
    assert(cat2->domain_count == 1);
    assert(cat2->word_count == 1);
    assert(strcmp(cat2->ban_domain_list[0], "another-example.com") == 0);
    assert(strcmp(cat2->ban_word_list[0], "forbidden") == 0);
    INFO("\tsuccess: Category 2 has been loaded correctly\n");

    remove(rules_filename);
}

void test_free_rules() {
    INFO("Testing free_rules...\n");

    const char* rules_filename = "test_rules.rules";
    create_test_rules_file(rules_filename);

    init_rules(rules_filename);
    free_rules();

    assert(rules.rules == NULL);
    INFO("\tsuccess: Rules have been freed correctly\n");

    remove(rules_filename);
}

int main() {
    INFO("Running rules.c tests...\n");

    test_init_rules();
    test_free_rules();

    INFO("Cleaning up after rules tests...\n");

    return 0;
}

