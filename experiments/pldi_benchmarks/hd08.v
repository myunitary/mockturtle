// Benchmark "hd08" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    i0, i1, i2, i3, i4, i5, i6, i7,
    om_0  );
  input  i0, i1, i2, i3, i4, i5, i6, i7;
  output om_0;
  wire new_n10_, new_n11_, new_n12_, new_n13_, new_n14_, new_n15_, new_n16_,
    new_n17_, new_n18_, new_n19_, new_n20_, new_n21_, new_n22_, new_n23_,
    new_n24_, new_n25_, new_n26_, new_n27_, new_n28_, new_n29_, new_n30_,
    new_n31_, new_n32_, new_n33_, new_n34_, new_n35_, new_n36_, new_n37_,
    new_n38_, new_n39_, new_n40_, new_n41_, new_n42_, new_n43_, new_n44_,
    new_n45_;
  assign new_n10_ = ~i0;
  assign new_n11_ = ~i1;
  assign new_n12_ = new_n11_ & new_n10_;
  assign new_n13_ = ~i2;
  assign new_n14_ = ~i3;
  assign new_n15_ = new_n14_ & new_n13_;
  assign new_n16_ = new_n15_ & new_n12_;
  assign new_n17_ = ~i4;
  assign new_n18_ = ~i5;
  assign new_n19_ = new_n18_ & new_n17_;
  assign new_n20_ = new_n19_ & new_n16_;
  assign new_n21_ = ~i6;
  assign new_n22_ = ~i7;
  assign new_n23_ = new_n22_ & new_n21_;
  assign new_n24_ = ~new_n23_ ^ ~new_n20_;
  assign new_n25_ = ~new_n16_;
  assign new_n26_ = ~new_n19_;
  assign new_n27_ = new_n26_ & new_n25_;
  assign new_n28_ = ~new_n27_;
  assign new_n29_ = ~new_n12_;
  assign new_n30_ = ~new_n15_;
  assign new_n31_ = new_n30_ & new_n29_;
  assign new_n32_ = ~new_n31_;
  assign new_n33_ = i7 & i6;
  assign new_n34_ = ~new_n33_;
  assign new_n35_ = i5 & i4;
  assign new_n36_ = ~new_n35_;
  assign new_n37_ = new_n36_ & new_n34_;
  assign new_n38_ = i1 & i0;
  assign new_n39_ = ~new_n38_;
  assign new_n40_ = i3 & i2;
  assign new_n41_ = ~new_n40_;
  assign new_n42_ = new_n41_ & new_n39_;
  assign new_n43_ = new_n42_ & new_n37_;
  assign new_n44_ = new_n43_ & new_n32_;
  assign new_n45_ = new_n44_ & new_n28_;
  assign om_0 = new_n45_ & new_n24_;
endmodule


