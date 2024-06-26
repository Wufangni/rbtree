#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include "rbtree.h"

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

static int comparison_count = 0;

static struct rb_node *my_search(struct rb_root *root, int key) {
    struct rb_node *node = root->rb_node;
    comparison_count = 0;

    while (node) {
        comparison_count++;
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
    comparison_count = 0;

    // Figure out where to put new node
    while (*new) {
        comparison_count++;
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
    comparison_count = 0;
    rb_erase(&data->rb, root);
    free(data);
}

void generate_gnuplot_script(const char* time_data_filename, const char* rotation_data_filename, const char* cache_data_filename, const char* time_script_filename, const char* rotation_script_filename, const char* cache_script_filename) {
    FILE* time_file = fopen(time_script_filename, "w");
    if (!time_file) {
        perror("Error opening file for writing");
        return;
    }

    fprintf(time_file, "set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'\n");
    fprintf(time_file, "set output 'rbtree_performance_time.png'\n");
    fprintf(time_file, "set title 'Red-Black Tree Performance (Time)'\n");
    fprintf(time_file, "set xlabel 'Node Count'\n");
    fprintf(time_file, "set ylabel 'Time (s)'\n");
    fprintf(time_file, "set key left top\n");
    fprintf(time_file, "set grid\n");
    fprintf(time_file, "plot '%s' using 1:2 title 'Insertion Time' with lines,\\\n", time_data_filename);
    fprintf(time_file, "     '%s' using 1:3 title 'Search Time' with lines,\\\n", time_data_filename);
    fprintf(time_file, "     '%s' using 1:4 title 'Deletion Time' with lines\n", time_data_filename);

    fclose(time_file);

    FILE* rotation_file = fopen(rotation_script_filename, "w");
    if (!rotation_file) {
        perror("Error opening file for writing");
        return;
    }

    fprintf(rotation_file, "set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'\n");
    fprintf(rotation_file, "set output 'rbtree_performance_rotation.png'\n");
    fprintf(rotation_file, "set title 'Red-Black Tree Performance (Rotations)'\n");
    fprintf(rotation_file, "set xlabel 'Node Count'\n");
    fprintf(rotation_file, "set ylabel 'Rotations'\n");
    fprintf(rotation_file, "set key left top\n");
    fprintf(rotation_file, "set grid\n");
    fprintf(rotation_file, "plot '%s' using 1:2 title 'Insertion Rotations' with lines,\\\n", rotation_data_filename);
    fprintf(rotation_file, "     '%s' using 1:3 title 'Search Rotations' with lines,\\\n", rotation_data_filename);
    fprintf(rotation_file, "     '%s' using 1:4 title 'Deletion Rotations' with lines\n", rotation_data_filename);

    fclose(rotation_file);

    FILE* cache_file = fopen(cache_script_filename, "w");
    if (!cache_file) {
        perror("Error opening file for writing");
        return;
    }

    fprintf(cache_file, "set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'\n");
    fprintf(cache_file, "set output 'rbtree_performance_cache.png'\n");
    fprintf(cache_file, "set title 'Red-Black Tree Performance (Cache)'\n");
    fprintf(cache_file, "set xlabel 'Node Count'\n");
    fprintf(cache_file, "set ylabel 'Cache Misses'\n");
    fprintf(cache_file, "set key left top\n");
    fprintf(cache_file, "set grid\n");
    fprintf(cache_file, "plot '%s' using 1:2 title 'Insertion Cache Misses' with lines,\\\n", cache_data_filename);
    fprintf(cache_file, "     '%s' using 1:3 title 'Search Cache Misses' with lines,\\\n", cache_data_filename);
    fprintf(cache_file, "     '%s' using 1:4 title 'Deletion Cache Misses' with lines\n", cache_data_filename);

    fclose(cache_file);
}

void run_perf_command(const char* command) {
    int ret = system(command);
    if (ret == -1) {
        perror("Error running perf command");
        exit(1);
    }
}

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


int main() {
    struct rb_root tree = RB_ROOT;
    clock_t start, end;
    int max_node_count = 5000; // Increase the range for better analysis
    int step_size = 500;
    int iterations = 25; // Increase iterations for more stable results

    // Print time results to a file
    FILE* time_file = fopen("rbtree_performance_time.dat", "w");
    if (!time_file) {
        perror("Error opening file for writing");
        return 1;
    }
    fprintf(time_file, "# NodeCount InsertionTime SearchTime DeletionTime\n");

    // Print rotation results to a file
    FILE* rotation_file = fopen("rbtree_performance_rotation.dat", "w");
    if (!rotation_file) {
        perror("Error opening file for writing");
        return 1;
    }
    fprintf(rotation_file, "# NodeCount InsertionRotations SearchRotations DeletionRotations\n");

    // Print cache results to a file
    FILE* cache_file = fopen("rbtree_performance_cache.dat", "w");
    if (!cache_file) {
        perror("Error opening file for writing");
        return 1;
    }
    fprintf(cache_file, "# NodeCount InsertionCacheMisses SearchCacheMisses DeletionCacheMisses\n");

    for (int n = step_size; n <= max_node_count; n += step_size) {
        double total_insert_time = 0.0, total_delete_time = 0.0, total_search_time = 0.0;
        long total_insert_rotations = 0, total_search_rotations = 0, total_delete_rotations = 0;
        long total_insert_cache_misses = 0, total_search_cache_misses = 0, total_delete_cache_misses = 0;

        for (int iter = 0; iter < iterations; iter++) {
            // Generate random keys
            int* keys = (int*)malloc(n * sizeof(int));
            for (int i = 0; i < n; i++) {
                keys[i] = rand() % 1000000;
            }

            // Measure insertion time, rotations, and cache misses
            for (int i = 0; i < n; i++) {
                struct my_node *new_node = (struct my_node *)malloc(sizeof(struct my_node));
                new_node->key = keys[i];

                start = clock();
                reset_rb_rotation_count();
                my_insert(&tree, new_node);
                end = clock();

                total_insert_time += get_time_in_seconds(start, end);
                total_insert_rotations += rb_rotation_count;
            }
            long insert_cache_misses;
            measure_cache_misses("insert", n, &insert_cache_misses);
            total_insert_cache_misses += insert_cache_misses;

            // Measure search time, rotations, and cache misses
            for (int i = 0; i < n; i++) {
                start = clock();
                reset_rb_rotation_count();
                my_search(&tree, keys[i]);
                end = clock();

                total_search_time += get_time_in_seconds(start, end);
                total_search_rotations += rb_rotation_count; // This should typically be zero
            }
            long search_cache_misses;
            measure_cache_misses("search", n, &search_cache_misses);
            total_search_cache_misses += search_cache_misses;

            // Measure deletion time, rotations, and cache misses
            for (int i = 0; i < n; i++) {
                struct rb_node *node = my_search(&tree, keys[i]);
                if (node) {
                    struct my_node *data = rb_entry(node, struct my_node, rb);

                    start = clock();
                    reset_rb_rotation_count();
                    my_delete(&tree, data);
                    end = clock();

                    total_delete_time += get_time_in_seconds(start, end);
                    total_delete_rotations += rb_rotation_count;
                }
            }
            long delete_cache_misses;
            measure_cache_misses("delete", n, &delete_cache_misses);
            total_delete_cache_misses += delete_cache_misses;

            free(keys);
        }

        fprintf(time_file, "%d %f %f %f\n", n, total_insert_time / iterations, total_search_time / iterations, total_delete_time / iterations);
        fprintf(rotation_file, "%d %ld %ld %ld\n", n, total_insert_rotations / iterations, total_search_rotations / iterations, total_delete_rotations / iterations);
        fprintf(cache_file, "%d %ld %ld %ld\n", n, total_insert_cache_misses / iterations, total_search_cache_misses / iterations, total_delete_cache_misses / iterations);
    }

    fclose(time_file);
    fclose(rotation_file);
    fclose(cache_file);

    // Generate the gnuplot scripts
    generate_gnuplot_script("rbtree_performance_time.dat", "rbtree_performance_rotation.dat", "rbtree_performance_cache.dat", "rbtree_performance_time.gnuplot", "rbtree_performance_rotation.gnuplot", "rbtree_performance_cache.gnuplot");

    // Call gnuplot to generate the graphs
    system("gnuplot rbtree_performance_time.gnuplot");
    system("gnuplot rbtree_performance_rotation.gnuplot");
    system("gnuplot rbtree_performance_cache.gnuplot");

    return 0;
}
