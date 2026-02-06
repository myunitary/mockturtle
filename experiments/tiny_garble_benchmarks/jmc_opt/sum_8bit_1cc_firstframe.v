// Benchmark "/tmp/tmp" written by ABC on Sun Nov  9 23:21:38 2025

module \/tmp/tmp  ( 
    n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16,
    po0, po1, po2, po3, po4, po5, po6, po7  );
  input  n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14,
    n15, n16;
  output po0, po1, po2, po3, po4, po5, po6, po7;
  wire new_n17, new_n18, new_n19, new_n20, new_n21, new_n22, new_n23,
    new_n24, new_n25, new_n26, new_n27, new_n28, new_n29, new_n30, new_n31,
    new_n32, new_n33, new_n34, new_n35, new_n36, new_n37, new_n38, new_n39,
    new_n40, new_n41, new_n42, new_n43, new_n44, new_n45, new_n46, new_n47,
    new_n48, new_n49, new_n50;
  assign new_n17 = n8 & n16;
  assign new_n18 = new_n17 ^ n15;
  assign new_n19 = n15 ^ n7;
  assign new_n20 = ~new_n18 & new_n19;
  assign new_n21 = new_n20 ^ n7;
  assign new_n22 = new_n21 ^ n14;
  assign new_n23 = n14 ^ n6;
  assign new_n24 = ~new_n22 & new_n23;
  assign new_n25 = new_n24 ^ n6;
  assign new_n26 = new_n25 ^ n13;
  assign new_n27 = n13 ^ n5;
  assign new_n28 = ~new_n26 & new_n27;
  assign new_n29 = new_n28 ^ n5;
  assign new_n30 = new_n29 ^ n12;
  assign new_n31 = n12 ^ n4;
  assign new_n32 = ~new_n30 & new_n31;
  assign new_n33 = new_n32 ^ n4;
  assign new_n34 = new_n33 ^ n11;
  assign new_n35 = n11 ^ n3;
  assign new_n36 = ~new_n34 & new_n35;
  assign new_n37 = new_n36 ^ n3;
  assign new_n38 = new_n37 ^ n10;
  assign new_n39 = n10 ^ n2;
  assign new_n40 = ~new_n38 & new_n39;
  assign new_n41 = new_n40 ^ n2;
  assign new_n42 = new_n41 ^ n9;
  assign new_n43 = new_n42 ^ n1;
  assign new_n44 = new_n38 ^ n2;
  assign new_n45 = new_n34 ^ n3;
  assign new_n46 = new_n30 ^ n4;
  assign new_n47 = new_n26 ^ n5;
  assign new_n48 = new_n22 ^ n6;
  assign new_n49 = new_n18 ^ n7;
  assign new_n50 = n16 ^ n8;
  assign po0 = new_n43;
  assign po1 = new_n44;
  assign po2 = new_n45;
  assign po3 = new_n46;
  assign po4 = new_n47;
  assign po5 = new_n48;
  assign po6 = new_n49;
  assign po7 = new_n50;
endmodule


