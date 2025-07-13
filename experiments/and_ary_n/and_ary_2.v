module and_ary_2( a0, a1, b0, b1, d0 );
  input a0, a1;
  input b0, b1;
  output d0;
  wire c0, c1;
  assign c0 = a0 & b0 ;
  assign c1 = a1 & b1 ;
  assign d0 = c0 & c1 ;
endmodule
