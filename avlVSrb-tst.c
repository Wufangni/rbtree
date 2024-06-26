#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include <string.h>
#include "rbtree.h"
#include "avltree.h"

// Function to measure time in seconds
double get_time_in_seconds(clock_t start, clock_t end) {
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Function to measure memory usage in kilobytes
long get_memory_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

// Custom structure for nodes
struct my_node {
    int key;
    struct rb_node rb;
};

// RB tree functions
static int rb_comparison_count = 0;

static struct rb_node *rb_search(struct rb_root *root, int key) {
    struct rb_node *node = root->rb_node;
    rb_comparison_count = 0;

    while (node) {
        rb_comparison_count++;
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

static int rb_insert(struct rb_root *root, struct my_node *data) {
    struct rb_node **new = &(root->rb_node), *parent = NULL;
    rb_comparison_count = 0;

    // Figure out where to put new node
    while (*new) {
        rb_comparison_count++;
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

static void rb_delete(struct rb_root *root, struct my_node *data) {
    rb_comparison_count = 0;
    rb_erase(&data->rb, root);
    free(data);
}

// AVL tree functions
static int avl_comparison_count = 0;

Node* avl_search(AVLTree tree, int key) {
    avl_comparison_count = 0;
    while (tree != NULL) {
        avl_comparison_count++;
        if (key < tree->key)
            tree = tree->left;
        else if (key > tree->key)
            tree = tree->right;
        else
            return tree;
    }
    return NULL;
}

Node* avl_insert(AVLTree tree, int key) {
    avl_comparison_count = 0;
    return avltree_insert(tree, key);
}

Node* avl_delete(AVLTree tree, int key) {
    avl_comparison_count = 0;
    return avltree_delete(tree, key);
}

// Generate gnuplot script
void generate_gnuplot_script(const char* data_filename, const char* script_filename, const char* output_filename, const char* title, const char* ylabel) {
    FILE* script_file = fopen(script_filename, "w");
    if (!script_file) {
        perror("Error opening file for writing");
        return;
    }

    fprintf(script_file, "set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'\n");
    fprintf(script_file, "set output '%s'\n", output_filename);
    fprintf(script_file, "set title '%s'\n", title);
    fprintf(script_file, "set xlabel 'Node Count'\n");
    fprintf(script_file, "set ylabel '%s'\n", ylabel);
    fprintf(script_file, "set key left top\n");
    fprintf(script_file, "set grid\n");
    fprintf(script_file, "plot '%s' using 1:2 title 'Red-Black Tree' with lines,\\\n", data_filename);
    fprintf(script_file, "     '%s' using 1:3 title 'AVL Tree' with lines\n", data_filename);

    fclose(script_file);
}

// Measure cache misses
void measure_cache_misses(const char* operation, int n, long* cache_misses) {
    char command[256];
    sprintf(command, "perf stat -e cache-misses ./rbtree-tst-perf-helper %s %d 2>&1 | grep 'cache-misses' | awk '{print $1}'", operation, n);

    FILE* perf_output = popen(command, "r");
    if (!perf_output) {
        perror("Error running perf command");
        exit(1);
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), perf_output) != NULL) {
        *cache_misses = atol(buffer);
    } else {
        *cache_misses = 0; // Default to 0 if parsing fails
    }

    pclose(perf_output);
}

void generate_unique_random_keys(int* keys, int n) {
    for (int i = 0; i < n; i++) {
        keys[i] = i;
    }

    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = keys[i];
        keys[i] = keys[j];
        keys[j] = temp;
    }
}

void measure_performance(const char* operation, int max_node_count, int step_size, int iterations) {
    char time_data_file[256];
    sprintf(time_data_file, "%s_time.dat", operation);
    FILE* time_file = fopen(time_data_file, "w");
    if (!time_file) {
        perror("Error opening file for writing");
        return;
    }
    fprintf(time_file, "# NodeCount RBTreeTime AVLTreeTime\n");

    for (int n = step_size; n <= max_node_count; n += step_size) {
        double total_rb_time = 0.0, total_avl_time = 0.0;

        for (int iter = 0; iter < iterations; iter++) {
            struct rb_root rb_tree = RB_ROOT;
            AVLTree avl_tree = NULL;

            int* keys = (int*)malloc(n * sizeof(int));
            generate_unique_random_keys(keys, n);

            clock_t start, end;

            if (strcmp(operation, "insert") == 0) {
                // Measure Insert
                for (int i = 0; i < n; i++) {
                    // RB Tree
                    struct my_node* new_node = malloc(sizeof(struct my_node));
                    new_node->key = keys[i];
                    start = clock();
                    rb_insert(&rb_tree, new_node);
                    end = clock();
                    total_rb_time += get_time_in_seconds(start, end);

                    // AVL Tree
                    start = clock();
                    avl_tree = avl_insert(avl_tree, keys[i]);
                    end = clock();
                    total_avl_time += get_time_in_seconds(start, end);
                }
            } else if (strcmp(operation, "search") == 0) {
                // Insert nodes first
                for (int i = 0; i < n; i++) {
                    struct my_node* new_node = malloc(sizeof(struct my_node));
                    new_node->key = keys[i];
                    rb_insert(&rb_tree, new_node);
                    avl_tree = avl_insert(avl_tree, keys[i]);
                }
                // Measure Search
                for (int i = 0; i < n; i++) {
                    // RB Tree
                    start = clock();
                    rb_search(&rb_tree, keys[i]);
                    end = clock();
                    total_rb_time += get_time_in_seconds(start, end);

                    // AVL Tree
                    start = clock();
                    avl_search(avl_tree, keys[i]);
                    end = clock();
                    total_avl_time += get_time_in_seconds(start, end);
                }
            } else if (strcmp(operation, "delete") == 0) {
                // Insert nodes first
                for (int i = 0; i < n; i++) {
                    struct my_node* new_node = malloc(sizeof(struct my_node));
                    new_node->key = keys[i];
                    rb_insert(&rb_tree, new_node);
                    avl_tree = avl_insert(avl_tree, keys[i]);
                }
                // Measure Delete
                for (int i = 0; i < n; i++) {
                    // RB Tree
                    struct my_node* node_to_delete = rb_entry(rb_search(&rb_tree, keys[i]), struct my_node, rb);
                    start = clock();
                    rb_delete(&rb_tree, node_to_delete);
                    end = clock();
                    total_rb_time += get_time_in_seconds(start, end);

                    // AVL Tree
                    start = clock();
                    avl_tree = avl_delete(avl_tree, keys[i]);
                    end = clock();
                    total_avl_time += get_time_in_seconds(start, end);
                }
            }

            free(keys);
            // Free AVL tree
            destroy_avltree(avl_tree);
        }

        fprintf(time_file, "%d %f %f\n", n, total_rb_time / iterations, total_avl_time / iterations);
    }

    fclose(time_file);
}

int main() {
    int max_node_count = 10000; // Increase the range for better analysis
    int step_size = 1000; // Increase step size
    int iterations = 50; // Increase iterations for more stable results

    measure_performance("insert", max_node_count, step_size, iterations);
    measure_performance("search", max_node_count, step_size, iterations);
    measure_performance("delete", max_node_count, step_size, iterations);

    // Generate gnuplot scripts for each operation
    generate_gnuplot_script("insert_time.dat", "insert_time.gnuplot", "insert_time.png", "Insertion Performance (Time)", "Time (s)");
    generate_gnuplot_script("search_time.dat", "search_time.gnuplot", "search_time.png", "Search Performance (Time)", "Time (s)");
    generate_gnuplot_script("delete_time.dat", "delete_time.gnuplot", "delete_time.png", "Deletion Performance (Time)", "Time (s)");

    // Run gnuplot for each operation
    system("gnuplot insert_time.gnuplot");
    system("gnuplot search_time.gnuplot");
    system("gnuplot delete_time.gnuplot");

    return 0;
}
