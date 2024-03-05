// Benchmark "hd10" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    i_2, i_3, i_4, i_5, i_6, i_7, i_8, i_9, i_10, i_11, i_12, i_13, i_14,
    i_15, i_16, i_17, i_18, i_19, i_20, i_21, i_22, i_23, i_24, i_25, i_26,
    i_27, i_28, i_29, i_30, i_31, i_32, i_33,
    m_0, m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12,
    m_13, m_14, m_15, m_16, m_17, m_18, m_19, m_20, m_21, m_22, m_23, m_24,
    m_25, m_26, m_27, m_28, m_29, m_30, m_31  );
  input  i_2, i_3, i_4, i_5, i_6, i_7, i_8, i_9, i_10, i_11, i_12, i_13,
    i_14, i_15, i_16, i_17, i_18, i_19, i_20, i_21, i_22, i_23, i_24, i_25,
    i_26, i_27, i_28, i_29, i_30, i_31, i_32, i_33;
  output m_0, m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12,
    m_13, m_14, m_15, m_16, m_17, m_18, m_19, m_20, m_21, m_22, m_23, m_24,
    m_25, m_26, m_27, m_28, m_29, m_30, m_31;
  wire new_n94_, new_n95_, new_n96_, new_n97_, new_n98_, new_n99_, new_n100_,
    new_n101_, new_n102_, new_n103_, new_n104_, new_n105_, new_n106_,
    new_n107_, new_n108_, new_n109_, new_n110_, new_n111_, new_n112_,
    new_n113_, new_n114_, new_n115_, new_n116_, new_n117_, new_n118_,
    new_n119_, new_n120_, new_n121_, new_n122_, new_n123_, new_n124_,
    new_n125_, new_n126_, new_n127_, new_n128_, new_n129_, new_n130_,
    new_n131_, new_n132_, new_n133_, new_n134_, new_n135_, new_n136_,
    new_n137_, new_n138_, new_n139_, new_n140_, new_n141_, new_n142_,
    new_n143_, new_n144_, new_n145_, new_n146_, new_n147_, new_n148_,
    new_n149_, new_n150_, new_n151_, new_n152_, new_n153_, new_n154_,
    new_n155_, new_n156_, new_n157_, new_n158_, new_n159_, new_n161_,
    new_n162_, new_n164_, new_n165_;
  assign new_n94_ = ~i_14;
  assign new_n95_ = ~i_15;
  assign new_n96_ = new_n95_ & new_n94_;
  assign new_n97_ = ~i_16;
  assign new_n98_ = ~i_17;
  assign new_n99_ = new_n98_ & new_n97_;
  assign new_n100_ = new_n99_ & new_n96_;
  assign new_n101_ = ~i_10;
  assign new_n102_ = ~i_11;
  assign new_n103_ = new_n102_ & new_n101_;
  assign new_n104_ = ~i_12;
  assign new_n105_ = ~i_13;
  assign new_n106_ = new_n105_ & new_n104_;
  assign new_n107_ = new_n106_ & new_n103_;
  assign new_n108_ = new_n107_ & new_n100_;
  assign new_n109_ = ~new_n108_;
  assign new_n110_ = ~i_6;
  assign new_n111_ = ~i_7;
  assign new_n112_ = new_n111_ & new_n110_;
  assign new_n113_ = ~i_8;
  assign new_n114_ = ~i_9;
  assign new_n115_ = new_n114_ & new_n113_;
  assign new_n116_ = new_n115_ & new_n112_;
  assign new_n117_ = ~i_2;
  assign new_n118_ = ~i_3;
  assign new_n119_ = new_n118_ & new_n117_;
  assign new_n120_ = ~i_4;
  assign new_n121_ = ~i_5;
  assign new_n122_ = new_n121_ & new_n120_;
  assign new_n123_ = new_n122_ & new_n119_;
  assign new_n124_ = new_n123_ & new_n116_;
  assign new_n125_ = ~new_n124_;
  assign new_n126_ = new_n125_ & new_n109_;
  assign new_n127_ = ~i_30;
  assign new_n128_ = ~i_31;
  assign new_n129_ = new_n128_ & new_n127_;
  assign new_n130_ = ~i_32;
  assign new_n131_ = ~i_33;
  assign new_n132_ = new_n131_ & new_n130_;
  assign new_n133_ = new_n132_ & new_n129_;
  assign new_n134_ = ~i_26;
  assign new_n135_ = ~i_27;
  assign new_n136_ = new_n135_ & new_n134_;
  assign new_n137_ = ~i_28;
  assign new_n138_ = ~i_29;
  assign new_n139_ = new_n138_ & new_n137_;
  assign new_n140_ = new_n139_ & new_n136_;
  assign new_n141_ = new_n140_ & new_n133_;
  assign new_n142_ = ~new_n141_;
  assign new_n143_ = ~i_22;
  assign new_n144_ = ~i_23;
  assign new_n145_ = new_n144_ & new_n143_;
  assign new_n146_ = ~i_24;
  assign new_n147_ = ~i_25;
  assign new_n148_ = new_n147_ & new_n146_;
  assign new_n149_ = new_n148_ & new_n145_;
  assign new_n150_ = ~i_18;
  assign new_n151_ = ~i_19;
  assign new_n152_ = new_n151_ & new_n150_;
  assign new_n153_ = ~i_20;
  assign new_n154_ = ~i_21;
  assign new_n155_ = new_n154_ & new_n153_;
  assign new_n156_ = new_n155_ & new_n152_;
  assign new_n157_ = new_n156_ & new_n149_;
  assign new_n158_ = ~new_n157_;
  assign new_n159_ = new_n158_ & new_n142_;
  assign m_29 = new_n159_ & new_n126_;
  assign new_n161_ = new_n158_ & new_n141_;
  assign new_n162_ = ~new_n161_ ^ ~new_n157_;
  assign m_30 = new_n162_ & new_n126_;
  assign new_n164_ = new_n161_ & new_n109_;
  assign new_n165_ = ~new_n164_ ^ ~new_n108_;
  assign m_31 = new_n165_ & new_n125_;
  assign m_0 = 1'b0;
  assign m_1 = 1'b0;
  assign m_2 = 1'b0;
  assign m_3 = 1'b0;
  assign m_4 = 1'b0;
  assign m_5 = 1'b0;
  assign m_6 = 1'b0;
  assign m_7 = 1'b0;
  assign m_8 = 1'b0;
  assign m_9 = 1'b0;
  assign m_10 = 1'b0;
  assign m_11 = 1'b0;
  assign m_12 = 1'b0;
  assign m_13 = 1'b0;
  assign m_14 = 1'b0;
  assign m_15 = 1'b0;
  assign m_16 = 1'b0;
  assign m_17 = 1'b0;
  assign m_18 = 1'b0;
  assign m_19 = 1'b0;
  assign m_20 = 1'b0;
  assign m_21 = 1'b0;
  assign m_22 = 1'b0;
  assign m_23 = 1'b0;
  assign m_24 = 1'b0;
  assign m_25 = 1'b0;
  assign m_26 = 1'b0;
  assign m_27 = 1'b0;
  assign m_28 = 1'b0;
endmodule


