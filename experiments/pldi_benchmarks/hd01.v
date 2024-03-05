// Benchmark "hd01" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15,
    i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26, i27, i28, i29,
    i30, i31,
    om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7, om_8, om_9, om_10,
    om_11, om_12, om_13, om_14, om_15, om_16, om_17, om_18, om_19, om_20,
    om_21, om_22, om_23, om_24, om_25, om_26, om_27, om_28, om_29, om_30,
    om_31  );
  input  i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14,
    i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26, i27, i28,
    i29, i30, i31;
  output om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7, om_8, om_9, om_10,
    om_11, om_12, om_13, om_14, om_15, om_16, om_17, om_18, om_19, om_20,
    om_21, om_22, om_23, om_24, om_25, om_26, om_27, om_28, om_29, om_30,
    om_31;
  wire new_n65_, new_n67_, new_n68_, new_n70_, new_n71_, new_n73_, new_n74_,
    new_n75_, new_n77_, new_n78_, new_n80_, new_n81_, new_n82_, new_n84_,
    new_n85_, new_n87_, new_n88_, new_n89_, new_n90_, new_n92_, new_n93_,
    new_n95_, new_n96_, new_n97_, new_n99_, new_n100_, new_n102_,
    new_n103_, new_n104_, new_n105_, new_n107_, new_n108_, new_n110_,
    new_n111_, new_n112_, new_n114_, new_n115_, new_n116_, new_n118_,
    new_n119_, new_n120_, new_n121_, new_n122_, new_n124_, new_n125_,
    new_n127_, new_n128_, new_n129_, new_n131_, new_n132_, new_n133_,
    new_n135_, new_n136_, new_n137_, new_n139_, new_n140_, new_n142_,
    new_n143_, new_n144_, new_n146_, new_n147_, new_n148_, new_n150_,
    new_n151_, new_n152_, new_n153_, new_n155_, new_n156_, new_n158_,
    new_n159_, new_n160_, new_n162_, new_n163_, new_n164_, new_n166_,
    new_n167_, new_n168_, new_n170_, new_n171_, new_n172_, new_n174_,
    new_n175_, new_n176_, new_n178_, new_n179_, new_n180_, new_n181_;
  assign new_n65_ = ~i0;
  assign om_1 = i1 & new_n65_;
  assign new_n67_ = ~i1;
  assign new_n68_ = new_n67_ & new_n65_;
  assign om_2 = new_n68_ & i2;
  assign new_n70_ = ~i2;
  assign new_n71_ = i3 & new_n70_;
  assign om_3 = new_n71_ & new_n68_;
  assign new_n73_ = ~i3;
  assign new_n74_ = new_n73_ & new_n70_;
  assign new_n75_ = new_n74_ & new_n68_;
  assign om_4 = new_n75_ & i4;
  assign new_n77_ = ~i4;
  assign new_n78_ = i5 & new_n77_;
  assign om_5 = new_n78_ & new_n75_;
  assign new_n80_ = ~i5;
  assign new_n81_ = new_n80_ & new_n77_;
  assign new_n82_ = new_n81_ & new_n75_;
  assign om_6 = new_n82_ & i6;
  assign new_n84_ = ~i6;
  assign new_n85_ = i7 & new_n84_;
  assign om_7 = new_n85_ & new_n82_;
  assign new_n87_ = ~i7;
  assign new_n88_ = new_n87_ & new_n84_;
  assign new_n89_ = new_n88_ & new_n81_;
  assign new_n90_ = new_n89_ & new_n75_;
  assign om_8 = new_n90_ & i8;
  assign new_n92_ = ~i8;
  assign new_n93_ = i9 & new_n92_;
  assign om_9 = new_n93_ & new_n90_;
  assign new_n95_ = ~i9;
  assign new_n96_ = new_n95_ & new_n92_;
  assign new_n97_ = new_n96_ & new_n90_;
  assign om_10 = new_n97_ & i10;
  assign new_n99_ = ~i10;
  assign new_n100_ = i11 & new_n99_;
  assign om_11 = new_n100_ & new_n97_;
  assign new_n102_ = ~i11;
  assign new_n103_ = new_n102_ & new_n99_;
  assign new_n104_ = new_n103_ & new_n96_;
  assign new_n105_ = new_n104_ & new_n90_;
  assign om_12 = new_n105_ & i12;
  assign new_n107_ = ~i12;
  assign new_n108_ = i13 & new_n107_;
  assign om_13 = new_n108_ & new_n105_;
  assign new_n110_ = ~i13;
  assign new_n111_ = new_n110_ & new_n107_;
  assign new_n112_ = new_n111_ & i14;
  assign om_14 = new_n112_ & new_n105_;
  assign new_n114_ = ~i14;
  assign new_n115_ = i15 & new_n114_;
  assign new_n116_ = new_n115_ & new_n111_;
  assign om_15 = new_n116_ & new_n105_;
  assign new_n118_ = ~i15;
  assign new_n119_ = new_n118_ & new_n114_;
  assign new_n120_ = new_n119_ & new_n111_;
  assign new_n121_ = new_n120_ & new_n104_;
  assign new_n122_ = new_n121_ & new_n90_;
  assign om_16 = new_n122_ & i16;
  assign new_n124_ = ~i16;
  assign new_n125_ = i17 & new_n124_;
  assign om_17 = new_n125_ & new_n122_;
  assign new_n127_ = ~i17;
  assign new_n128_ = new_n127_ & new_n124_;
  assign new_n129_ = new_n128_ & i18;
  assign om_18 = new_n129_ & new_n122_;
  assign new_n131_ = ~i18;
  assign new_n132_ = new_n128_ & new_n131_;
  assign new_n133_ = new_n132_ & i19;
  assign om_19 = new_n133_ & new_n122_;
  assign new_n135_ = ~i19;
  assign new_n136_ = new_n132_ & new_n135_;
  assign new_n137_ = new_n136_ & new_n122_;
  assign om_20 = new_n137_ & i20;
  assign new_n139_ = ~i20;
  assign new_n140_ = i21 & new_n139_;
  assign om_21 = new_n140_ & new_n137_;
  assign new_n142_ = ~i21;
  assign new_n143_ = new_n142_ & new_n139_;
  assign new_n144_ = new_n143_ & i22;
  assign om_22 = new_n144_ & new_n137_;
  assign new_n146_ = ~i22;
  assign new_n147_ = new_n143_ & new_n146_;
  assign new_n148_ = new_n147_ & i23;
  assign om_23 = new_n148_ & new_n137_;
  assign new_n150_ = ~i23;
  assign new_n151_ = new_n147_ & new_n150_;
  assign new_n152_ = new_n151_ & new_n136_;
  assign new_n153_ = new_n152_ & new_n122_;
  assign om_24 = new_n153_ & i24;
  assign new_n155_ = ~i24;
  assign new_n156_ = i25 & new_n155_;
  assign om_25 = new_n156_ & new_n153_;
  assign new_n158_ = ~i25;
  assign new_n159_ = new_n158_ & new_n155_;
  assign new_n160_ = new_n159_ & i26;
  assign om_26 = new_n160_ & new_n153_;
  assign new_n162_ = ~i26;
  assign new_n163_ = new_n159_ & new_n162_;
  assign new_n164_ = new_n163_ & i27;
  assign om_27 = new_n164_ & new_n153_;
  assign new_n166_ = ~i27;
  assign new_n167_ = new_n163_ & new_n166_;
  assign new_n168_ = new_n167_ & i28;
  assign om_28 = new_n168_ & new_n153_;
  assign new_n170_ = ~i28;
  assign new_n171_ = new_n167_ & new_n170_;
  assign new_n172_ = new_n171_ & i29;
  assign om_29 = new_n172_ & new_n153_;
  assign new_n174_ = ~i29;
  assign new_n175_ = i30 & new_n174_;
  assign new_n176_ = new_n175_ & new_n171_;
  assign om_30 = new_n176_ & new_n153_;
  assign new_n178_ = ~i30;
  assign new_n179_ = new_n178_ & new_n174_;
  assign new_n180_ = new_n179_ & i31;
  assign new_n181_ = new_n180_ & new_n171_;
  assign om_31 = new_n181_ & new_n153_;
  assign om_0 = i0;
endmodule


