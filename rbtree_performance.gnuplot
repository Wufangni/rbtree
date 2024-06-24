set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'
set output 'rbtree_performance.png'
set title 'Red-Black Tree Performance'
set xlabel 'Node Count'
set ylabel 'Time (s)'
set key left top
set grid
plot 'rbtree_performance.dat' using 1:2 title 'Insertion' with lines,\
     'rbtree_performance.dat' using 1:3 title 'Search' with lines,\
     'rbtree_performance.dat' using 1:4 title 'Deletion' with lines
