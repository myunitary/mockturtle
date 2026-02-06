module point_and_ary_4( a0, a1, a2, a3, b0, b1, b2, b3, d0 );
  input a0, a1, a2, a3;
  input b0, b1, b2, b3;
  output d0;
  wire e0, e1, e2, e3,c0, c1, c2, c3;
  assign e0 = a0 ^ b0;
  assign e1 = a1 ^ b1;
  assign e2 = a2 ^ b2;
  assign e3 = a3 ^ b3;
  assign c0 = e0 & e1;
  assign c1 = e2 & e3;
  assign d0 = c0 & c1;
endmodule
