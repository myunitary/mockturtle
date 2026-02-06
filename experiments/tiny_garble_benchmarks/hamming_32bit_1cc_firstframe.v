// Benchmark "/tmp/tmp" written by ABC on Sat Nov  8 22:47:27 2025

module hamming_32bit_1cc_firstframe ( 
    n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16,
    n17, n18, n19, n20, n21, n22, n23, n24, n25, n26, n27, n28, n29, n30,
    n31, n32, n33, n34, n35, n36, n37, n38, n39, n40, n41, n42, n43, n44,
    n45, n46, n47, n48, n49, n50, n51, n52, n53, n54, n55, n56, n57, n58,
    n59, n60, n61, n62, n63, n64,
    po0, po1, po2, po3, po4, po5  );
  input  n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14,
    n15, n16, n17, n18, n19, n20, n21, n22, n23, n24, n25, n26, n27, n28,
    n29, n30, n31, n32, n33, n34, n35, n36, n37, n38, n39, n40, n41, n42,
    n43, n44, n45, n46, n47, n48, n49, n50, n51, n52, n53, n54, n55, n56,
    n57, n58, n59, n60, n61, n62, n63, n64;
  output po0, po1, po2, po3, po4, po5;
  wire new_n65, new_n66, new_n67, new_n68, new_n69, new_n70, new_n71,
    new_n72, new_n73, new_n74, new_n75, new_n76, new_n77, new_n78, new_n79,
    new_n80, new_n81, new_n82, new_n83, new_n84, new_n85, new_n86, new_n87,
    new_n88, new_n89, new_n90, new_n91, new_n92, new_n93, new_n94, new_n95,
    new_n96, new_n97, new_n98, new_n99, new_n100, new_n101, new_n102,
    new_n103, new_n104, new_n105, new_n106, new_n107, new_n108, new_n109,
    new_n110, new_n111, new_n112, new_n113, new_n114, new_n115, new_n116,
    new_n117, new_n118, new_n119, new_n120, new_n121, new_n122, new_n123,
    new_n124, new_n125, new_n126, new_n127, new_n128, new_n129, new_n130,
    new_n131, new_n132, new_n133, new_n134, new_n135, new_n136, new_n137,
    new_n138, new_n139, new_n140, new_n141, new_n142, new_n143, new_n144,
    new_n145, new_n146, new_n147, new_n148, new_n149, new_n150, new_n151,
    new_n152, new_n153, new_n154, new_n155, new_n156, new_n157, new_n158,
    new_n159, new_n160, new_n161, new_n162, new_n163, new_n164, new_n165,
    new_n166, new_n167, new_n168, new_n169, new_n170, new_n171, new_n172,
    new_n173, new_n174, new_n175, new_n176, new_n177, new_n178, new_n179,
    new_n180, new_n181, new_n182, new_n183, new_n184, new_n185, new_n186,
    new_n187, new_n188, new_n189, new_n190, new_n191, new_n192, new_n193,
    new_n194, new_n195, new_n196, new_n197, new_n198, new_n199, new_n200,
    new_n201, new_n202, new_n203, new_n204, new_n205, new_n206, new_n207,
    new_n208, new_n209, new_n210, new_n211, new_n212, new_n213, new_n214,
    new_n215, new_n216, new_n217, new_n218, new_n219, new_n220, new_n221,
    new_n222, new_n223, new_n224, new_n225, new_n226, new_n227, new_n228,
    new_n229, new_n230, new_n231, new_n232, new_n233, new_n234, new_n235,
    new_n236;
  assign new_n65 = n64 ^ n32;
  assign new_n66 = n58 ^ n26;
  assign new_n67 = n59 ^ n27;
  assign new_n68 = n60 ^ n28;
  assign new_n69 = new_n68 ^ new_n67;
  assign new_n70 = new_n69 ^ new_n66;
  assign new_n71 = n56 ^ n24;
  assign new_n72 = n57 ^ n25;
  assign new_n73 = new_n72 ^ new_n71;
  assign new_n74 = n61 ^ n29;
  assign new_n75 = new_n74 ^ new_n73;
  assign new_n76 = n55 ^ n23;
  assign new_n77 = new_n76 ^ new_n75;
  assign new_n78 = new_n77 ^ new_n70;
  assign new_n79 = n49 ^ n17;
  assign new_n80 = n50 ^ n18;
  assign new_n81 = new_n80 ^ new_n79;
  assign new_n82 = n54 ^ n22;
  assign new_n83 = new_n82 ^ new_n81;
  assign new_n84 = n48 ^ n16;
  assign new_n85 = new_n84 ^ new_n83;
  assign new_n86 = n62 ^ n30;
  assign new_n87 = new_n86 ^ new_n85;
  assign new_n88 = n51 ^ n19;
  assign new_n89 = n52 ^ n20;
  assign new_n90 = n53 ^ n21;
  assign new_n91 = new_n90 ^ new_n89;
  assign new_n92 = new_n91 ^ new_n88;
  assign new_n93 = new_n92 ^ new_n87;
  assign new_n94 = new_n93 ^ new_n78;
  assign new_n95 = new_n94 ^ new_n65;
  assign new_n96 = n34 ^ n2;
  assign new_n97 = n35 ^ n3;
  assign new_n98 = new_n97 ^ new_n96;
  assign new_n99 = n39 ^ n7;
  assign new_n100 = new_n99 ^ new_n98;
  assign new_n101 = n33 ^ n1;
  assign new_n102 = new_n101 ^ new_n100;
  assign new_n103 = n47 ^ n15;
  assign new_n104 = new_n103 ^ new_n102;
  assign new_n105 = n36 ^ n4;
  assign new_n106 = n37 ^ n5;
  assign new_n107 = n38 ^ n6;
  assign new_n108 = new_n107 ^ new_n106;
  assign new_n109 = new_n108 ^ new_n105;
  assign new_n110 = new_n109 ^ new_n104;
  assign new_n111 = n63 ^ n31;
  assign new_n112 = new_n111 ^ new_n110;
  assign new_n113 = n43 ^ n11;
  assign new_n114 = n44 ^ n12;
  assign new_n115 = n45 ^ n13;
  assign new_n116 = new_n115 ^ new_n114;
  assign new_n117 = new_n116 ^ new_n113;
  assign new_n118 = n41 ^ n9;
  assign new_n119 = n42 ^ n10;
  assign new_n120 = new_n119 ^ new_n118;
  assign new_n121 = n46 ^ n14;
  assign new_n122 = new_n121 ^ new_n120;
  assign new_n123 = n40 ^ n8;
  assign new_n124 = new_n123 ^ new_n122;
  assign new_n125 = new_n124 ^ new_n117;
  assign new_n126 = new_n125 ^ new_n112;
  assign new_n127 = new_n126 ^ new_n95;
  assign new_n128 = ~new_n127 & new_n65;
  assign new_n129 = new_n76 ^ new_n72;
  assign new_n130 = ~new_n129 & new_n73;
  assign new_n131 = new_n130 ^ new_n71;
  assign new_n132 = ~new_n70 & new_n77;
  assign new_n133 = ~new_n74 & ~new_n77;
  assign new_n134 = new_n133 ^ new_n132;
  assign new_n135 = new_n134 ^ new_n131;
  assign new_n136 = new_n67 ^ new_n66;
  assign new_n137 = ~new_n69 & new_n136;
  assign new_n138 = new_n137 ^ new_n66;
  assign new_n139 = new_n138 ^ new_n135;
  assign new_n140 = ~new_n81 & new_n80;
  assign new_n141 = new_n81 & new_n84;
  assign new_n142 = new_n141 ^ new_n140;
  assign new_n143 = ~new_n85 & new_n82;
  assign new_n144 = new_n85 & new_n92;
  assign new_n145 = new_n144 ^ new_n143;
  assign new_n146 = new_n145 ^ new_n142;
  assign new_n147 = ~new_n78 & new_n93;
  assign new_n148 = ~new_n86 & ~new_n93;
  assign new_n149 = new_n148 ^ new_n147;
  assign new_n150 = new_n149 ^ new_n146;
  assign new_n151 = ~new_n88 & new_n91;
  assign new_n152 = ~new_n90 & ~new_n91;
  assign new_n153 = new_n152 ^ new_n151;
  assign new_n154 = new_n153 ^ new_n150;
  assign new_n155 = new_n154 ^ new_n139;
  assign new_n156 = new_n155 ^ new_n128;
  assign new_n157 = ~new_n98 & new_n97;
  assign new_n158 = new_n98 & new_n101;
  assign new_n159 = new_n158 ^ new_n157;
  assign new_n160 = ~new_n102 & new_n99;
  assign new_n161 = new_n102 & new_n109;
  assign new_n162 = new_n161 ^ new_n160;
  assign new_n163 = new_n162 ^ new_n159;
  assign new_n164 = new_n110 & new_n125;
  assign new_n165 = ~new_n110 & new_n103;
  assign new_n166 = new_n165 ^ new_n164;
  assign new_n167 = new_n166 ^ new_n163;
  assign new_n168 = ~new_n105 & new_n108;
  assign new_n169 = ~new_n107 & ~new_n108;
  assign new_n170 = new_n169 ^ new_n168;
  assign new_n171 = new_n170 ^ new_n167;
  assign new_n172 = new_n94 & new_n126;
  assign new_n173 = ~new_n126 & new_n111;
  assign new_n174 = new_n173 ^ new_n172;
  assign new_n175 = new_n174 ^ new_n171;
  assign new_n176 = ~new_n120 & new_n119;
  assign new_n177 = new_n120 & new_n123;
  assign new_n178 = new_n177 ^ new_n176;
  assign new_n179 = ~new_n117 & new_n124;
  assign new_n180 = ~new_n121 & ~new_n124;
  assign new_n181 = new_n180 ^ new_n179;
  assign new_n182 = new_n181 ^ new_n178;
  assign new_n183 = new_n113 & new_n116;
  assign new_n184 = ~new_n116 & new_n115;
  assign new_n185 = new_n184 ^ new_n183;
  assign new_n186 = new_n185 ^ new_n182;
  assign new_n187 = new_n186 ^ new_n175;
  assign new_n188 = new_n187 ^ new_n156;
  assign new_n189 = new_n128 & new_n188;
  assign new_n190 = new_n153 ^ new_n145;
  assign new_n191 = new_n146 & new_n190;
  assign new_n192 = new_n191 ^ new_n142;
  assign new_n193 = ~new_n139 & new_n154;
  assign new_n194 = ~new_n149 & ~new_n154;
  assign new_n195 = new_n194 ^ new_n193;
  assign new_n196 = new_n195 ^ new_n192;
  assign new_n197 = new_n138 ^ new_n131;
  assign new_n198 = ~new_n135 & ~new_n197;
  assign new_n199 = new_n198 ^ new_n134;
  assign new_n200 = new_n199 ^ new_n196;
  assign new_n201 = new_n200 ^ new_n189;
  assign new_n202 = new_n170 ^ new_n162;
  assign new_n203 = new_n163 & new_n202;
  assign new_n204 = new_n203 ^ new_n159;
  assign new_n205 = ~new_n171 & ~new_n186;
  assign new_n206 = new_n166 & new_n171;
  assign new_n207 = new_n206 ^ new_n205;
  assign new_n208 = new_n207 ^ new_n204;
  assign new_n209 = new_n155 & new_n187;
  assign new_n210 = ~new_n174 & ~new_n187;
  assign new_n211 = new_n210 ^ new_n209;
  assign new_n212 = new_n211 ^ new_n208;
  assign new_n213 = new_n185 ^ new_n181;
  assign new_n214 = ~new_n182 & new_n213;
  assign new_n215 = new_n214 ^ new_n178;
  assign new_n216 = new_n215 ^ new_n212;
  assign new_n217 = new_n216 ^ new_n201;
  assign new_n218 = ~new_n217 & new_n189;
  assign new_n219 = new_n199 ^ new_n192;
  assign new_n220 = new_n196 & new_n219;
  assign new_n221 = new_n220 ^ new_n195;
  assign new_n222 = new_n221 ^ new_n218;
  assign new_n223 = new_n215 ^ new_n204;
  assign new_n224 = ~new_n223 & new_n208;
  assign new_n225 = new_n224 ^ new_n207;
  assign new_n226 = ~new_n216 & new_n200;
  assign new_n227 = new_n211 & new_n216;
  assign new_n228 = new_n227 ^ new_n226;
  assign new_n229 = new_n228 ^ new_n225;
  assign new_n230 = new_n229 ^ new_n222;
  assign new_n231 = new_n218 & new_n230;
  assign new_n232 = new_n228 ^ new_n221;
  assign new_n233 = ~new_n229 & new_n232;
  assign new_n234 = new_n233 ^ new_n225;
  assign new_n235 = new_n231 & new_n234;
  assign new_n236 = new_n234 ^ new_n231;
  assign po0 = new_n235;
  assign po1 = new_n236;
  assign po2 = ~new_n230;
  assign po3 = new_n217;
  assign po4 = ~new_n188;
  assign po5 = new_n127;
endmodule


