// Benchmark "hd07" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    i0, i1, i2, i3, i4, i5, i6, i7,
    om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7  );
  input  i0, i1, i2, i3, i4, i5, i6, i7;
  output om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7;
  wire new_n19_, new_n20_, new_n22_, new_n23_, new_n24_, new_n25_, new_n26_,
    new_n27_, new_n29_, new_n30_, new_n31_, new_n32_, new_n33_, new_n34_,
    new_n35_, new_n36_, new_n37_, new_n40_, new_n41_, new_n42_, new_n43_,
    new_n44_, new_n45_, new_n46_, new_n47_, new_n48_;
  assign new_n19_ = ~i1;
  assign new_n20_ = new_n19_ & i0;
  assign om_2 = new_n20_ & i2;
  assign new_n22_ = ~new_n20_;
  assign new_n23_ = ~i2;
  assign new_n24_ = new_n23_ & i1;
  assign new_n25_ = ~new_n24_;
  assign new_n26_ = new_n25_ & new_n22_;
  assign new_n27_ = ~new_n26_;
  assign om_3 = new_n27_ & i3;
  assign new_n29_ = i4 & i3;
  assign new_n30_ = ~new_n29_;
  assign new_n31_ = ~i3;
  assign new_n32_ = new_n31_ & new_n23_;
  assign new_n33_ = ~new_n32_;
  assign new_n34_ = new_n33_ & new_n30_;
  assign new_n35_ = ~new_n34_;
  assign new_n36_ = new_n35_ & new_n26_;
  assign new_n37_ = ~new_n36_;
  assign om_4 = new_n37_ & i4;
  assign om_5 = new_n37_ & i5;
  assign new_n40_ = ~i6;
  assign new_n41_ = new_n40_ & i5;
  assign new_n42_ = ~new_n41_;
  assign new_n43_ = ~i5;
  assign new_n44_ = new_n43_ & i4;
  assign new_n45_ = ~new_n44_;
  assign new_n46_ = new_n45_ & new_n42_;
  assign new_n47_ = new_n46_ & new_n36_;
  assign new_n48_ = ~new_n47_;
  assign om_6 = new_n48_ & i6;
  assign om_7 = new_n48_ & i7;
  assign om_0 = 1'b0;
  assign om_1 = 1'b0;
endmodule


