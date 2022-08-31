module top(x0, x1, x2, y0, y1);
  input x0, x1, x2;
  output y0, y1;
  wire n0, n1, n2, n3, n4;
  assign n0 = x0 ^ x1;
  assign y0 = x2 ^ n0;
  assign n1 = x0 & x1;
  assign n2 = x1 & x2;
  assign n3 = x0 & x2;
  assign n4 = n1 | n2;
  assign y1 = n3 | n4;
endmodule
