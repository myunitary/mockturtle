module bitonic_sort_inc_1_4_1( x0 , x1 , x2 , x3 , y0 );
  input x0 , x1 , x2 , x3 ;
  output y0 ;
  wire n5 , n6 , n7 , n8 , n9 , n10 , n11 , n12 , n13 , n14 ;
  assign n5 = ~x0 & ~x1 ;
  assign n6 = x2 & x3 ;
  assign n7 = ~n5 & ~n6 ;
  assign n11 = n7 ^ n6 ;
  assign n8 = x0 & x1 ;
  assign n9 = ~x2 & ~x3 ;
  assign n10 = n8 & n9 ;
  assign n12 = n10 ^ n9 ;
  assign n13 = n11 & n12 ;
  assign n14 = n13 ^ n11 ;
  assign y0 = n14 ;
endmodule
