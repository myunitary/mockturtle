// Benchmark "/tmp/tmp" written by ABC on Wed Nov 12 17:31:41 2025

module comparator_32bit_unsigned_lt ( 
    n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16,
    n17, n18, n19, n20, n21, n22, n23, n24, n25, n26, n27, n28, n29, n30,
    n31, n32, n33, n34, n35, n36, n37, n38, n39, n40, n41, n42, n43, n44,
    n45, n46, n47, n48, n49, n50, n51, n52, n53, n54, n55, n56, n57, n58,
    n59, n60, n61, n62, n63, n64,
    po0  );
  input  n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14,
    n15, n16, n17, n18, n19, n20, n21, n22, n23, n24, n25, n26, n27, n28,
    n29, n30, n31, n32, n33, n34, n35, n36, n37, n38, n39, n40, n41, n42,
    n43, n44, n45, n46, n47, n48, n49, n50, n51, n52, n53, n54, n55, n56,
    n57, n58, n59, n60, n61, n62, n63, n64;
  output po0;
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
    new_n166, new_n167, new_n177, new_n178, new_n179, new_n180, new_n181,
    new_n182, new_n183, new_n184, new_n185, new_n186, new_n187, new_n188,
    new_n189, new_n190, new_n191, new_n192, new_n193, new_n194, new_n195,
    new_n196, new_n197, new_n198, new_n199, new_n202, new_n203, new_n204,
    new_n205, new_n206, new_n207, new_n208, new_n209, new_n210, new_n211,
    new_n212, new_n213, new_n214, new_n215, new_n216, new_n217, new_n218,
    new_n219, new_n220, new_n221, new_n222, new_n223, new_n224, new_n225,
    new_n226, new_n227, new_n228, new_n229, new_n230, new_n231, new_n232,
    new_n233, new_n234, new_n235, new_n236, new_n237, new_n238, new_n239,
    new_n240, new_n241, new_n242, new_n243, new_n244, new_n245, new_n246,
    new_n247, new_n248, new_n249, new_n250, new_n251, new_n252, new_n253,
    new_n254, new_n255, new_n256, new_n257, new_n258, new_n259, new_n260,
    new_n261;
  assign new_n65 = ~n30 & n62;
  assign new_n66 = ~n61 & n29;
  assign new_n67 = n61 ^ n29;
  assign new_n68 = new_n67 ^ new_n66;
  assign new_n69 = ~new_n65 & ~new_n68;
  assign new_n70 = ~n31 & n63;
  assign new_n71 = ~n32 & n64;
  assign new_n72 = ~new_n70 & ~new_n71;
  assign new_n73 = new_n69 & new_n72;
  assign new_n74 = n60 ^ n28;
  assign new_n75 = n59 ^ n27;
  assign new_n76 = ~n28 & n60;
  assign new_n77 = new_n76 ^ new_n75;
  assign new_n78 = ~n26 & ~n58;
  assign new_n79 = new_n78 ^ n58;
  assign new_n80 = new_n79 ^ n27;
  assign new_n81 = ~new_n77 & ~new_n80;
  assign new_n82 = new_n81 ^ n27;
  assign new_n83 = new_n82 ^ n60;
  assign new_n84 = ~new_n74 & ~new_n83;
  assign new_n85 = new_n84 ^ n60;
  assign new_n86 = ~n27 & n59;
  assign new_n87 = ~new_n76 & ~new_n86;
  assign new_n88 = new_n78 ^ n26;
  assign new_n89 = new_n87 & new_n88;
  assign new_n90 = ~n57 & new_n89;
  assign new_n91 = n25 & new_n89;
  assign new_n92 = ~new_n90 & ~new_n91;
  assign new_n93 = new_n91 ^ new_n90;
  assign new_n94 = new_n93 ^ new_n92;
  assign new_n95 = new_n85 & new_n94;
  assign new_n96 = ~new_n95 & new_n73;
  assign new_n97 = ~new_n92 & new_n73;
  assign new_n98 = ~n24 & n56;
  assign new_n99 = ~n55 & n23;
  assign new_n100 = n55 ^ n23;
  assign new_n101 = new_n100 ^ new_n99;
  assign new_n102 = ~new_n98 & ~new_n101;
  assign new_n103 = n54 ^ n22;
  assign new_n104 = ~n21 & n53;
  assign new_n105 = n53 ^ n21;
  assign new_n106 = new_n105 ^ new_n104;
  assign new_n107 = new_n106 ^ n54;
  assign new_n108 = ~new_n103 & new_n107;
  assign new_n109 = new_n108 ^ n22;
  assign new_n110 = new_n102 & new_n109;
  assign new_n111 = ~n22 & n54;
  assign new_n112 = ~new_n104 & ~new_n111;
  assign new_n113 = new_n102 & new_n112;
  assign new_n114 = n52 ^ n20;
  assign new_n115 = n51 ^ n19;
  assign new_n116 = n50 ^ n18;
  assign new_n117 = ~n17 & n49;
  assign new_n118 = n49 ^ n17;
  assign new_n119 = new_n118 ^ new_n117;
  assign new_n120 = new_n119 ^ n50;
  assign new_n121 = ~new_n116 & new_n120;
  assign new_n122 = new_n121 ^ n18;
  assign new_n123 = new_n122 ^ n51;
  assign new_n124 = ~new_n115 & ~new_n123;
  assign new_n125 = new_n124 ^ n52;
  assign new_n126 = new_n125 ^ n51;
  assign new_n127 = ~new_n114 & ~new_n126;
  assign new_n128 = new_n127 ^ n20;
  assign new_n129 = new_n113 & new_n128;
  assign new_n130 = ~new_n110 & ~new_n129;
  assign new_n131 = n56 ^ n24;
  assign new_n132 = new_n99 ^ n56;
  assign new_n133 = ~new_n131 & new_n132;
  assign new_n134 = new_n133 ^ n24;
  assign new_n135 = ~new_n134 & new_n130;
  assign new_n136 = ~new_n135 & new_n97;
  assign new_n137 = ~new_n96 & ~new_n136;
  assign new_n138 = ~n18 & n50;
  assign new_n139 = ~n19 & n51;
  assign new_n140 = ~n20 & n52;
  assign new_n141 = ~new_n139 & ~new_n140;
  assign new_n142 = ~n16 & n48;
  assign new_n143 = ~n15 & n47;
  assign new_n144 = ~new_n142 & ~new_n143;
  assign new_n145 = ~n14 & ~n46;
  assign new_n146 = new_n145 ^ n14;
  assign new_n147 = new_n144 & new_n146;
  assign new_n148 = ~n13 & n45;
  assign new_n149 = n44 ^ n12;
  assign new_n150 = n43 ^ n11;
  assign new_n151 = ~n10 & n42;
  assign new_n152 = n42 ^ n10;
  assign new_n153 = new_n152 ^ new_n151;
  assign new_n154 = new_n153 ^ n43;
  assign new_n155 = ~new_n150 & ~new_n154;
  assign new_n156 = new_n155 ^ n43;
  assign new_n157 = new_n156 ^ n44;
  assign new_n158 = ~new_n149 & new_n157;
  assign new_n159 = new_n158 ^ n44;
  assign new_n160 = ~n12 & n44;
  assign new_n161 = ~n11 & n43;
  assign new_n162 = ~new_n160 & ~new_n161;
  assign new_n163 = ~n41 & ~new_n151;
  assign new_n164 = new_n162 & new_n163;
  assign new_n165 = ~new_n151 & n9;
  assign new_n166 = new_n162 & new_n165;
  assign new_n167 = new_n166 ^ new_n164;
  assign new_n177 = n38 ^ n6;
  assign new_n178 = n37 ^ n5;
  assign new_n179 = n36 ^ n4;
  assign new_n180 = n35 ^ n3;
  assign new_n181 = n34 ^ n2;
  assign new_n182 = ~n3 & n35;
  assign new_n183 = new_n182 ^ new_n181;
  assign new_n184 = ~n1 & n33;
  assign new_n185 = new_n184 ^ n2;
  assign new_n186 = ~new_n183 & ~new_n185;
  assign new_n187 = new_n186 ^ n2;
  assign new_n188 = new_n187 ^ n35;
  assign new_n189 = ~new_n180 & ~new_n188;
  assign new_n190 = new_n189 ^ n35;
  assign new_n191 = new_n190 ^ n36;
  assign new_n192 = ~new_n179 & new_n191;
  assign new_n193 = new_n192 ^ n36;
  assign new_n194 = new_n193 ^ n37;
  assign new_n195 = ~new_n178 & new_n194;
  assign new_n196 = new_n195 ^ n37;
  assign new_n197 = new_n196 ^ n38;
  assign new_n198 = ~new_n177 & new_n197;
  assign new_n199 = new_n198 ^ n38;
  assign new_n202 = new_n261 ^ new_n166;
  assign new_n203 = ~new_n202 & new_n167;
  assign new_n204 = new_n203 ^ new_n166;
  assign new_n205 = ~new_n204 & new_n159;
  assign new_n206 = ~new_n148 & ~new_n205;
  assign new_n207 = new_n147 & new_n206;
  assign new_n208 = n13 & new_n147;
  assign new_n209 = ~n45 & new_n208;
  assign new_n210 = n48 ^ n16;
  assign new_n211 = n47 ^ n15;
  assign new_n212 = new_n211 ^ new_n142;
  assign new_n213 = new_n145 ^ n46;
  assign new_n214 = new_n213 ^ n15;
  assign new_n215 = ~new_n212 & ~new_n214;
  assign new_n216 = new_n215 ^ n15;
  assign new_n217 = new_n216 ^ n48;
  assign new_n218 = ~new_n210 & ~new_n217;
  assign new_n219 = new_n218 ^ n48;
  assign new_n220 = ~new_n209 & new_n219;
  assign new_n221 = ~new_n207 & new_n220;
  assign new_n222 = ~new_n117 & ~new_n221;
  assign new_n223 = new_n141 & new_n222;
  assign new_n224 = ~new_n138 & new_n223;
  assign new_n225 = new_n113 & new_n224;
  assign new_n226 = new_n97 & new_n225;
  assign new_n227 = n64 ^ n32;
  assign new_n228 = n63 ^ n31;
  assign new_n229 = n62 ^ n30;
  assign new_n230 = new_n66 ^ n62;
  assign new_n231 = ~new_n229 & new_n230;
  assign new_n232 = new_n231 ^ n30;
  assign new_n233 = new_n232 ^ n63;
  assign new_n234 = ~new_n228 & ~new_n233;
  assign new_n235 = new_n234 ^ n64;
  assign new_n236 = new_n235 ^ n63;
  assign new_n237 = ~new_n227 & ~new_n236;
  assign new_n238 = new_n237 ^ n32;
  assign new_n239 = ~new_n226 & ~new_n238;
  assign new_n240 = new_n137 & new_n239;
  assign new_n241 = ~n39 & ~n40;
  assign new_n242 = n39 & n40;
  assign new_n243 = new_n199 ^ n7;
  assign new_n244 = new_n199 ^ n8;
  assign new_n245 = ~new_n243 & new_n242;
  assign new_n246 = new_n242 ^ n40;
  assign new_n247 = ~new_n243 & new_n246;
  assign new_n248 = new_n247 ^ n40;
  assign new_n249 = ~new_n243 & new_n241;
  assign new_n250 = new_n241 ^ n40;
  assign new_n251 = ~new_n243 & ~new_n250;
  assign new_n252 = new_n251 ^ n40;
  assign new_n253 = new_n248 ^ new_n245;
  assign new_n254 = ~new_n244 & new_n253;
  assign new_n255 = new_n254 ^ new_n245;
  assign new_n256 = new_n252 ^ new_n249;
  assign new_n257 = ~new_n244 & ~new_n256;
  assign new_n258 = new_n257 ^ new_n249;
  assign new_n259 = new_n258 ^ new_n255;
  assign new_n260 = ~n8 & ~new_n259;
  assign new_n261 = new_n260 ^ new_n255;
  assign po0 = new_n240;
endmodule


