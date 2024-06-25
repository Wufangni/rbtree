set terminal pngcairo size 1280,960 enhanced font 'Verdana,10'
set output 'rbtree_performance_rotation.png'
set title 'Red-Black Tree Performance (Rotations)'
set xlabel 'Node Count'
set ylabel 'Rotations'
set key left top
set grid
plot 'rbtree_performance_rotation.dat' using 1:2 title 'Insertion Rotations' with lines,\
     'rbtree_performance_rotation.dat' using 1:3 title 'Search Rotations' with lines,\
     'rbtree_performance_rotation.dat' using 1:4 title 'Deletion Rotations' with lines
