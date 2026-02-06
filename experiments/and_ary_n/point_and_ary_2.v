module point_and_ary_2( a0, a1, b0, b1, d0 );
  input a0, a1;
  input b0, b1;
  output d0;
  wire e0, e1;
  assign e0 = a0 ^ b0 ;
  assign e1 = a1 ^ b1 ;
  assign d0 = e0 & e1 ;
endmodule
