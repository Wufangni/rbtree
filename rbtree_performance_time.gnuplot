set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'
set output 'rbtree_performance_time.png'
set title 'Red-Black Tree Performance (Time)'
set xlabel 'Node Count'
set ylabel 'Time (s)'
set key left top
set grid
plot 'rbtree_performance_time.dat' using 1:2 title 'Insertion Time' with lines,\
     'rbtree_performance_time.dat' using 1:3 title 'Search Time' with lines,\
     'rbtree_performance_time.dat' using 1:4 title 'Deletion Time' with lines
