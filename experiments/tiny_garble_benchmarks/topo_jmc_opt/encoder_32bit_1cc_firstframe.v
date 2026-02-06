// Benchmark "/tmp/tmp" written by ABC on Wed Nov 12 17:32:04 2025

module encoder_32bit_1cc_firstframe ( 
    n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16,
    n17, n18, n19, n20, n21, n22, n23, n24, n25, n26, n27, n28, n29, n30,
    n31, n32,
    po0, po1, po2, po3, po4  );
  input  n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14,
    n15, n16, n17, n18, n19, n20, n21, n22, n23, n24, n25, n26, n27, n28,
    n29, n30, n31, n32;
  output po0, po1, po2, po3, po4;
  wire new_n33, new_n34, new_n35, new_n36, new_n37, new_n38, new_n39,
    new_n40, new_n41, new_n42, new_n43, new_n44, new_n45, new_n46, new_n47,
    new_n48, new_n49, new_n50, new_n51, new_n52, new_n53, new_n54, new_n55,
    new_n56, new_n57, new_n58, new_n59, new_n60, new_n61, new_n62, new_n63,
    new_n64, new_n65, new_n66, new_n67, new_n68, new_n69, new_n70, new_n71,
    new_n72, new_n73, new_n74, new_n75, new_n76, new_n77, new_n78, new_n79,
    new_n80, new_n81, new_n82, new_n83, new_n84, new_n85, new_n86, new_n87,
    new_n88, new_n89, new_n90;
  assign new_n33 = ~n29 & ~n31;
  assign new_n34 = ~n25 & ~n27;
  assign new_n35 = new_n33 & new_n34;
  assign new_n36 = ~n21 & ~n23;
  assign new_n37 = ~n17 & ~n19;
  assign new_n38 = new_n36 & new_n37;
  assign new_n39 = new_n35 & new_n38;
  assign new_n40 = ~n18 & ~n20;
  assign new_n41 = ~n26 & ~n28;
  assign new_n42 = new_n40 & new_n41;
  assign new_n43 = ~n22 & ~n32;
  assign new_n44 = ~n24 & ~n30;
  assign new_n45 = new_n43 & new_n44;
  assign new_n46 = new_n42 & new_n45;
  assign new_n47 = new_n39 & new_n46;
  assign new_n48 = ~n18 & ~n22;
  assign new_n49 = new_n48 ^ n20;
  assign new_n50 = ~n24 & ~new_n49;
  assign new_n51 = new_n50 ^ n24;
  assign new_n52 = new_n50 ^ new_n38;
  assign new_n53 = new_n48 & new_n52;
  assign new_n54 = new_n53 ^ new_n48;
  assign new_n55 = new_n54 ^ n20;
  assign new_n56 = ~new_n51 & ~new_n55;
  assign new_n57 = ~n5 & ~n7;
  assign new_n58 = ~n1 & ~n3;
  assign new_n59 = new_n57 & new_n58;
  assign new_n60 = ~n2 & ~n6;
  assign new_n61 = ~n4 & ~n8;
  assign new_n62 = new_n60 & new_n61;
  assign new_n63 = new_n59 & new_n62;
  assign new_n64 = new_n56 & new_n63;
  assign new_n65 = ~n9 & ~n11;
  assign new_n66 = ~n2 & ~n4;
  assign new_n67 = ~n10 & ~n12;
  assign new_n68 = new_n66 & new_n67;
  assign new_n69 = new_n65 & new_n68;
  assign new_n70 = new_n58 & new_n69;
  assign new_n71 = new_n37 & new_n42;
  assign new_n72 = new_n34 & new_n71;
  assign new_n73 = new_n70 & new_n72;
  assign new_n74 = ~n26 & ~n30;
  assign new_n75 = new_n48 & new_n74;
  assign new_n76 = ~n9 & ~n13;
  assign new_n77 = ~n1 & ~n5;
  assign new_n78 = new_n76 & new_n77;
  assign new_n79 = new_n75 & new_n78;
  assign new_n80 = ~n25 & ~n29;
  assign new_n81 = ~n17 & ~n21;
  assign new_n82 = new_n80 & new_n81;
  assign new_n83 = ~n10 & ~n14;
  assign new_n84 = new_n60 & new_n83;
  assign new_n85 = new_n82 & new_n84;
  assign new_n86 = new_n79 & new_n85;
  assign new_n87 = ~n13 & ~n15;
  assign new_n88 = new_n65 & new_n87;
  assign new_n89 = new_n59 & new_n88;
  assign new_n90 = new_n39 & new_n89;
  assign po0 = ~new_n47;
  assign po1 = ~new_n64;
  assign po2 = ~new_n73;
  assign po3 = ~new_n86;
  assign po4 = ~new_n90;
endmodule


