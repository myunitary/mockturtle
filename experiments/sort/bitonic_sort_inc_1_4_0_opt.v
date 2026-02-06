module bitonic_sort_inc_1_4_0_opt( x0 , x1 , x2 , x3 , y0 );
  input x0 , x1 , x2 , x3 ;
  output y0 ;
  wire n5 , n6 , n7 , n8 ;
  assign n5 = ~x0 & ~x2 ;
  assign n6 = ~x1 & ~x3 ;
  assign n7 = n5 & ~n6 ;
  assign n8 = n7 ^ n5 ;
  assign y0 = ~n8 ;
endmodule
