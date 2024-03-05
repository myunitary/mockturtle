// Benchmark "hd03" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15,
    om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7  );
  input  i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14,
    i15;
  output om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7;
  wire new_n25_, new_n26_, new_n27_, new_n28_, new_n30_, new_n31_, new_n32_,
    new_n33_, new_n34_, new_n35_, new_n36_, new_n38_, new_n39_, new_n40_,
    new_n41_, new_n42_, new_n43_, new_n44_, new_n46_, new_n47_, new_n48_,
    new_n49_, new_n50_, new_n51_, new_n53_, new_n54_, new_n55_, new_n56_,
    new_n57_, new_n58_, new_n59_, new_n60_, new_n61_, new_n63_, new_n64_,
    new_n65_, new_n66_, new_n67_, new_n68_, new_n70_, new_n71_, new_n72_,
    new_n73_, new_n74_, new_n75_, new_n76_, new_n77_, new_n78_, new_n80_,
    new_n81_, new_n82_, new_n83_, new_n84_, new_n85_, new_n86_, new_n87_;
  assign new_n25_ = i8 & i0;
  assign new_n26_ = ~new_n25_;
  assign new_n27_ = ~i1;
  assign new_n28_ = ~i9 ^ ~new_n27_;
  assign om_0 = ~new_n28_ ^ ~new_n26_;
  assign new_n30_ = ~new_n28_;
  assign new_n31_ = new_n30_ & new_n25_;
  assign new_n32_ = ~new_n31_;
  assign new_n33_ = i9 & i1;
  assign new_n34_ = ~new_n33_;
  assign new_n35_ = ~i10 ^ ~i2;
  assign new_n36_ = ~new_n35_ ^ ~new_n34_;
  assign om_1 = ~new_n36_ ^ ~new_n32_;
  assign new_n38_ = ~new_n36_;
  assign new_n39_ = new_n38_ & new_n31_;
  assign new_n40_ = new_n35_ & new_n33_;
  assign new_n41_ = ~new_n40_ ^ ~new_n39_;
  assign new_n42_ = i10 & i2;
  assign new_n43_ = ~i11 ^ ~i3;
  assign new_n44_ = ~new_n43_ ^ ~new_n42_;
  assign om_2 = ~new_n44_ ^ ~new_n41_;
  assign new_n46_ = new_n43_ & new_n42_;
  assign new_n47_ = new_n44_ & new_n41_;
  assign new_n48_ = ~new_n47_ ^ ~new_n46_;
  assign new_n49_ = i11 & i3;
  assign new_n50_ = ~i12 ^ ~i4;
  assign new_n51_ = ~new_n50_ ^ ~new_n49_;
  assign om_3 = ~new_n51_ ^ ~new_n48_;
  assign new_n53_ = new_n51_ & new_n44_;
  assign new_n54_ = new_n53_ & new_n41_;
  assign new_n55_ = new_n50_ & new_n49_;
  assign new_n56_ = new_n51_ & new_n46_;
  assign new_n57_ = ~new_n56_ ^ ~new_n55_;
  assign new_n58_ = ~new_n57_ ^ ~new_n54_;
  assign new_n59_ = i12 & i4;
  assign new_n60_ = ~i13 ^ ~i5;
  assign new_n61_ = ~new_n60_ ^ ~new_n59_;
  assign om_4 = ~new_n61_ ^ ~new_n58_;
  assign new_n63_ = new_n60_ & new_n59_;
  assign new_n64_ = new_n61_ & new_n58_;
  assign new_n65_ = ~new_n64_ ^ ~new_n63_;
  assign new_n66_ = i13 & i5;
  assign new_n67_ = ~i14 ^ ~i6;
  assign new_n68_ = ~new_n67_ ^ ~new_n66_;
  assign om_5 = ~new_n68_ ^ ~new_n65_;
  assign new_n70_ = new_n68_ & new_n61_;
  assign new_n71_ = new_n70_ & new_n58_;
  assign new_n72_ = new_n67_ & new_n66_;
  assign new_n73_ = new_n68_ & new_n63_;
  assign new_n74_ = ~new_n73_ ^ ~new_n72_;
  assign new_n75_ = ~new_n74_ ^ ~new_n71_;
  assign new_n76_ = i14 & i6;
  assign new_n77_ = ~i15 ^ ~i7;
  assign new_n78_ = ~new_n77_ ^ ~new_n76_;
  assign om_6 = ~new_n78_ ^ ~new_n75_;
  assign new_n80_ = new_n78_ & new_n70_;
  assign new_n81_ = new_n80_ & new_n58_;
  assign new_n82_ = new_n77_ & new_n76_;
  assign new_n83_ = new_n78_ & new_n74_;
  assign new_n84_ = ~new_n83_ ^ ~new_n82_;
  assign new_n85_ = ~new_n84_ ^ ~new_n81_;
  assign new_n86_ = i15 & i7;
  assign new_n87_ = ~new_n77_ ^ ~new_n86_;
  assign om_7 = ~new_n87_ ^ ~new_n85_;
endmodule


