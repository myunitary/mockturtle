module and_ary_4( a0, a1, a2, a3, b0, b1, b2, b3, d0 );
  input a0, a1, a2, a3;
  input b0, b1, b2, b3;
  output d0;
  wire c0, c1, c2, c3, c4, c5;
  assign c0 = a0 & b0 ;
  assign c1 = a1 & b1 ;
  assign c2 = a2 & b2 ;
  assign c3 = a3 & b3 ;
  assign c4 = c0 & c1 ;
  assign c5 = c2 & c3 ;
  assign d0 = c4 & c5 ;
endmodule
