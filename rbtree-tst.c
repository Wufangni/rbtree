#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rbtree.h"

// Function to measure time in seconds
double get_time_in_seconds(clock_t start, clock_t end) {
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Custom structure for nodes
struct my_node {
    int key;
    struct rb_node rb;
};

static struct rb_node *my_search(struct rb_root *root, int key) {
    struct rb_node *node = root->rb_node;

    while (node) {
        struct my_node *data = rb_entry(node, struct my_node, rb);

        if (key < data->key)
            node = node->rb_left;
        else if (key > data->key)
            node = node->rb_right;
        else
            return node;
    }
    return NULL;
}

static int my_insert(struct rb_root *root, struct my_node *data) {
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    // Figure out where to put new node
    while (*new) {
        struct my_node *this = rb_entry(*new, struct my_node, rb);

        parent = *new;
        if (data->key < this->key)
            new = &((*new)->rb_left);
        else if (data->key > this->key)
            new = &((*new)->rb_right);
        else
            return 0;
    }

    // Add new node and rebalance tree.
    rb_link_node(&data->rb, parent, new);
    rb_insert_color(&data->rb, root);

    return 1;
}

static void my_delete(struct rb_root *root, struct my_node *data) {
    rb_erase(&data->rb, root);
    free(data);
}

void generate_gnuplot_script(const char* data_filename, const char* script_filename) {
    FILE* file = fopen(script_filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        return;
    }

    fprintf(file, "set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'\n");
    fprintf(file, "set output 'rbtree_performance.png'\n");
    fprintf(file, "set title 'Red-Black Tree Performance'\n");
    fprintf(file, "set xlabel 'Node Count'\n");
    fprintf(file, "set ylabel 'Time (s)'\n");
    fprintf(file, "set key left top\n");
    fprintf(file, "set grid\n");
    fprintf(file, "plot '%s' using 1:2 title 'Insertion' with lines,\\\n", data_filename);
    fprintf(file, "     '%s' using 1:3 title 'Search' with lines,\\\n", data_filename);
    fprintf(file, "     '%s' using 1:4 title 'Deletion' with lines\n", data_filename);

    fclose(file);
}

int main() {
    struct rb_root tree = RB_ROOT;
    clock_t start, end;
    int max_node_count = 250;
    int step_size = 10;
    int iterations = 10;

    // Print results to a file
    FILE* file = fopen("rbtree_performance.dat", "w");
    if (!file) {
        perror("Error opening file for writing");
        return 1;
    }
    fprintf(file, "# NodeCount Insertion Search Deletion\n");

    for (int n = step_size; n <= max_node_count; n += step_size) {
        double total_insert_time = 0.0, total_delete_time = 0.0, total_search_time = 0.0;

        for (int iter = 0; iter < iterations; iter++) {
            // Generate random keys
            int* keys = (int*)malloc(n * sizeof(int));
            for (int i = 0; i < n; i++) {
                keys[i] = rand() % 1000000;
            }

            // Measure insertion time
            for (int i = 0; i < n; i++) {
                struct my_node *new_node = (struct my_node *)malloc(sizeof(struct my_node));
                new_node->key = keys[i];

                start = clock();
                my_insert(&tree, new_node);
                end = clock();

                total_insert_time += get_time_in_seconds(start, end);
            }

            // Measure search time
            for (int i = 0; i < n; i++) {
                start = clock();
                my_search(&tree, keys[i]);
                end = clock();

                total_search_time += get_time_in_seconds(start, end);
            }

            // Measure deletion time
            for (int i = 0; i < n; i++) {
                struct rb_node *node = my_search(&tree, keys[i]);
                if (node) {
                    struct my_node *data = rb_entry(node, struct my_node, rb);

                    start = clock();
                    my_delete(&tree, data);
                    end = clock();

                    total_delete_time += get_time_in_seconds(start, end);
                }
            }

            free(keys);
        }

        fprintf(file, "%d %f %f %f\n", n, total_insert_time / iterations, total_search_time / iterations, total_delete_time / iterations);
    }

    fclose(file);

    // Generate the gnuplot script
    generate_gnuplot_script("rbtree_performance.dat", "rbtree_performance.gnuplot");

    // Call gnuplot to generate the graph
    system("gnuplot rbtree_performance.gnuplot");

    return 0;
}
