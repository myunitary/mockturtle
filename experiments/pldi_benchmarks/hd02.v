// Benchmark "hd02" written by ABC on Wed Dec 27 23:19:07 2023

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
  wire new_n65_, new_n67_, new_n68_, new_n69_, new_n70_, new_n71_, new_n73_,
    new_n74_, new_n75_, new_n76_, new_n77_, new_n79_, new_n80_, new_n81_,
    new_n82_, new_n83_, new_n85_, new_n86_, new_n87_, new_n88_, new_n89_,
    new_n90_, new_n92_, new_n93_, new_n94_, new_n95_, new_n96_, new_n98_,
    new_n99_, new_n100_, new_n101_, new_n102_, new_n103_, new_n105_,
    new_n106_, new_n107_, new_n108_, new_n109_, new_n110_, new_n112_,
    new_n113_, new_n114_, new_n115_, new_n116_, new_n117_, new_n118_,
    new_n120_, new_n121_, new_n122_, new_n123_, new_n124_, new_n126_,
    new_n127_, new_n128_, new_n129_, new_n130_, new_n131_, new_n133_,
    new_n134_, new_n135_, new_n136_, new_n137_, new_n138_, new_n140_,
    new_n141_, new_n142_, new_n143_, new_n144_, new_n145_, new_n146_,
    new_n148_, new_n149_, new_n150_, new_n151_, new_n152_, new_n153_,
    new_n155_, new_n156_, new_n157_, new_n158_, new_n159_, new_n160_,
    new_n161_, new_n163_, new_n164_, new_n165_, new_n166_, new_n167_,
    new_n168_, new_n169_, new_n171_, new_n172_, new_n173_, new_n174_,
    new_n175_, new_n176_, new_n177_, new_n178_, new_n180_, new_n181_,
    new_n182_, new_n183_, new_n184_, new_n186_, new_n187_, new_n188_,
    new_n189_, new_n190_, new_n191_, new_n193_, new_n194_, new_n195_,
    new_n196_, new_n197_, new_n198_, new_n200_, new_n201_, new_n202_,
    new_n203_, new_n204_, new_n205_, new_n206_, new_n208_, new_n209_,
    new_n210_, new_n211_, new_n212_, new_n213_, new_n215_, new_n216_,
    new_n217_, new_n218_, new_n219_, new_n220_, new_n221_, new_n223_,
    new_n224_, new_n225_, new_n226_, new_n227_, new_n228_, new_n229_,
    new_n231_, new_n232_, new_n233_, new_n234_, new_n235_, new_n236_,
    new_n237_, new_n238_, new_n240_, new_n241_, new_n242_, new_n243_,
    new_n244_, new_n245_, new_n247_, new_n248_, new_n249_, new_n250_,
    new_n251_, new_n252_, new_n253_, new_n255_, new_n256_, new_n257_,
    new_n258_, new_n259_, new_n260_, new_n261_, new_n263_, new_n264_,
    new_n265_, new_n266_, new_n267_, new_n268_, new_n269_, new_n270_,
    new_n272_, new_n273_, new_n274_, new_n275_, new_n276_, new_n277_,
    new_n278_, new_n280_, new_n281_, new_n282_, new_n283_, new_n284_,
    new_n285_, new_n286_, new_n287_, new_n289_, new_n290_, new_n291_,
    new_n292_;
  assign new_n65_ = ~i31 ^ ~i0;
  assign om_0 = ~new_n65_ ^ ~i31;
  assign new_n67_ = ~i0;
  assign new_n68_ = i31 & new_n67_;
  assign new_n69_ = ~new_n68_;
  assign new_n70_ = ~i1;
  assign new_n71_ = ~i31 ^ ~new_n70_;
  assign om_1 = ~new_n71_ ^ ~new_n69_;
  assign new_n73_ = ~new_n71_;
  assign new_n74_ = new_n73_ & new_n68_;
  assign new_n75_ = ~new_n74_;
  assign new_n76_ = ~i2;
  assign new_n77_ = ~i31 ^ ~new_n76_;
  assign om_2 = ~new_n77_ ^ ~new_n75_;
  assign new_n79_ = ~new_n77_;
  assign new_n80_ = new_n79_ & new_n74_;
  assign new_n81_ = ~new_n80_;
  assign new_n82_ = ~i3;
  assign new_n83_ = ~i31 ^ ~new_n82_;
  assign om_3 = ~new_n83_ ^ ~new_n81_;
  assign new_n85_ = ~new_n83_;
  assign new_n86_ = new_n85_ & new_n79_;
  assign new_n87_ = new_n86_ & new_n74_;
  assign new_n88_ = ~new_n87_;
  assign new_n89_ = ~i4;
  assign new_n90_ = ~i31 ^ ~new_n89_;
  assign om_4 = ~new_n90_ ^ ~new_n88_;
  assign new_n92_ = ~new_n90_;
  assign new_n93_ = new_n92_ & new_n87_;
  assign new_n94_ = ~new_n93_;
  assign new_n95_ = ~i5;
  assign new_n96_ = ~i31 ^ ~new_n95_;
  assign om_5 = ~new_n96_ ^ ~new_n94_;
  assign new_n98_ = ~new_n96_;
  assign new_n99_ = new_n98_ & new_n92_;
  assign new_n100_ = new_n99_ & new_n87_;
  assign new_n101_ = ~new_n100_;
  assign new_n102_ = ~i6;
  assign new_n103_ = ~i31 ^ ~new_n102_;
  assign om_6 = ~new_n103_ ^ ~new_n101_;
  assign new_n105_ = ~new_n103_;
  assign new_n106_ = new_n105_ & new_n99_;
  assign new_n107_ = new_n106_ & new_n87_;
  assign new_n108_ = ~new_n107_;
  assign new_n109_ = ~i7;
  assign new_n110_ = ~i31 ^ ~new_n109_;
  assign om_7 = ~new_n110_ ^ ~new_n108_;
  assign new_n112_ = ~new_n110_;
  assign new_n113_ = new_n112_ & new_n105_;
  assign new_n114_ = new_n113_ & new_n99_;
  assign new_n115_ = new_n114_ & new_n87_;
  assign new_n116_ = ~new_n115_;
  assign new_n117_ = ~i8;
  assign new_n118_ = ~i31 ^ ~new_n117_;
  assign om_8 = ~new_n118_ ^ ~new_n116_;
  assign new_n120_ = ~new_n118_;
  assign new_n121_ = new_n120_ & new_n115_;
  assign new_n122_ = ~new_n121_;
  assign new_n123_ = ~i9;
  assign new_n124_ = ~i31 ^ ~new_n123_;
  assign om_9 = ~new_n124_ ^ ~new_n122_;
  assign new_n126_ = ~new_n124_;
  assign new_n127_ = new_n126_ & new_n120_;
  assign new_n128_ = new_n127_ & new_n115_;
  assign new_n129_ = ~new_n128_;
  assign new_n130_ = ~i10;
  assign new_n131_ = ~i31 ^ ~new_n130_;
  assign om_10 = ~new_n131_ ^ ~new_n129_;
  assign new_n133_ = ~new_n131_;
  assign new_n134_ = new_n133_ & new_n127_;
  assign new_n135_ = new_n134_ & new_n115_;
  assign new_n136_ = ~new_n135_;
  assign new_n137_ = ~i11;
  assign new_n138_ = ~i31 ^ ~new_n137_;
  assign om_11 = ~new_n138_ ^ ~new_n136_;
  assign new_n140_ = ~new_n138_;
  assign new_n141_ = new_n140_ & new_n133_;
  assign new_n142_ = new_n141_ & new_n127_;
  assign new_n143_ = new_n142_ & new_n115_;
  assign new_n144_ = ~new_n143_;
  assign new_n145_ = ~i12;
  assign new_n146_ = ~i31 ^ ~new_n145_;
  assign om_12 = ~new_n146_ ^ ~new_n144_;
  assign new_n148_ = ~new_n146_;
  assign new_n149_ = new_n148_ & new_n142_;
  assign new_n150_ = new_n149_ & new_n115_;
  assign new_n151_ = ~new_n150_;
  assign new_n152_ = ~i13;
  assign new_n153_ = ~i31 ^ ~new_n152_;
  assign om_13 = ~new_n153_ ^ ~new_n151_;
  assign new_n155_ = ~new_n153_;
  assign new_n156_ = new_n155_ & new_n148_;
  assign new_n157_ = new_n156_ & new_n142_;
  assign new_n158_ = new_n157_ & new_n115_;
  assign new_n159_ = ~new_n158_;
  assign new_n160_ = ~i14;
  assign new_n161_ = ~i31 ^ ~new_n160_;
  assign om_14 = ~new_n161_ ^ ~new_n159_;
  assign new_n163_ = ~new_n161_;
  assign new_n164_ = new_n163_ & new_n156_;
  assign new_n165_ = new_n164_ & new_n142_;
  assign new_n166_ = new_n165_ & new_n115_;
  assign new_n167_ = ~new_n166_;
  assign new_n168_ = ~i15;
  assign new_n169_ = ~i31 ^ ~new_n168_;
  assign om_15 = ~new_n169_ ^ ~new_n167_;
  assign new_n171_ = ~new_n169_;
  assign new_n172_ = new_n171_ & new_n163_;
  assign new_n173_ = new_n172_ & new_n156_;
  assign new_n174_ = new_n173_ & new_n142_;
  assign new_n175_ = new_n174_ & new_n115_;
  assign new_n176_ = ~new_n175_;
  assign new_n177_ = ~i16;
  assign new_n178_ = ~i31 ^ ~new_n177_;
  assign om_16 = ~new_n178_ ^ ~new_n176_;
  assign new_n180_ = ~new_n178_;
  assign new_n181_ = new_n180_ & new_n175_;
  assign new_n182_ = ~new_n181_;
  assign new_n183_ = ~i17;
  assign new_n184_ = ~i31 ^ ~new_n183_;
  assign om_17 = ~new_n184_ ^ ~new_n182_;
  assign new_n186_ = ~new_n184_;
  assign new_n187_ = new_n186_ & new_n180_;
  assign new_n188_ = new_n187_ & new_n175_;
  assign new_n189_ = ~new_n188_;
  assign new_n190_ = ~i18;
  assign new_n191_ = ~i31 ^ ~new_n190_;
  assign om_18 = ~new_n191_ ^ ~new_n189_;
  assign new_n193_ = ~new_n191_;
  assign new_n194_ = new_n193_ & new_n187_;
  assign new_n195_ = new_n194_ & new_n175_;
  assign new_n196_ = ~new_n195_;
  assign new_n197_ = ~i19;
  assign new_n198_ = ~i31 ^ ~new_n197_;
  assign om_19 = ~new_n198_ ^ ~new_n196_;
  assign new_n200_ = ~new_n198_;
  assign new_n201_ = new_n200_ & new_n193_;
  assign new_n202_ = new_n201_ & new_n187_;
  assign new_n203_ = new_n202_ & new_n175_;
  assign new_n204_ = ~new_n203_;
  assign new_n205_ = ~i20;
  assign new_n206_ = ~i31 ^ ~new_n205_;
  assign om_20 = ~new_n206_ ^ ~new_n204_;
  assign new_n208_ = ~new_n206_;
  assign new_n209_ = new_n208_ & new_n202_;
  assign new_n210_ = new_n209_ & new_n175_;
  assign new_n211_ = ~new_n210_;
  assign new_n212_ = ~i21;
  assign new_n213_ = ~i31 ^ ~new_n212_;
  assign om_21 = ~new_n213_ ^ ~new_n211_;
  assign new_n215_ = ~new_n213_;
  assign new_n216_ = new_n215_ & new_n208_;
  assign new_n217_ = new_n216_ & new_n202_;
  assign new_n218_ = new_n217_ & new_n175_;
  assign new_n219_ = ~new_n218_;
  assign new_n220_ = ~i22;
  assign new_n221_ = ~i31 ^ ~new_n220_;
  assign om_22 = ~new_n221_ ^ ~new_n219_;
  assign new_n223_ = ~new_n221_;
  assign new_n224_ = new_n223_ & new_n216_;
  assign new_n225_ = new_n224_ & new_n202_;
  assign new_n226_ = new_n225_ & new_n175_;
  assign new_n227_ = ~new_n226_;
  assign new_n228_ = ~i23;
  assign new_n229_ = ~i31 ^ ~new_n228_;
  assign om_23 = ~new_n229_ ^ ~new_n227_;
  assign new_n231_ = ~new_n229_;
  assign new_n232_ = new_n231_ & new_n223_;
  assign new_n233_ = new_n232_ & new_n216_;
  assign new_n234_ = new_n233_ & new_n202_;
  assign new_n235_ = new_n234_ & new_n175_;
  assign new_n236_ = ~new_n235_;
  assign new_n237_ = ~i24;
  assign new_n238_ = ~i31 ^ ~new_n237_;
  assign om_24 = ~new_n238_ ^ ~new_n236_;
  assign new_n240_ = ~new_n238_;
  assign new_n241_ = new_n240_ & new_n234_;
  assign new_n242_ = new_n241_ & new_n175_;
  assign new_n243_ = ~new_n242_;
  assign new_n244_ = ~i25;
  assign new_n245_ = ~i31 ^ ~new_n244_;
  assign om_25 = ~new_n245_ ^ ~new_n243_;
  assign new_n247_ = ~new_n245_;
  assign new_n248_ = new_n247_ & new_n240_;
  assign new_n249_ = new_n248_ & new_n234_;
  assign new_n250_ = new_n249_ & new_n175_;
  assign new_n251_ = ~new_n250_;
  assign new_n252_ = ~i26;
  assign new_n253_ = ~i31 ^ ~new_n252_;
  assign om_26 = ~new_n253_ ^ ~new_n251_;
  assign new_n255_ = ~new_n253_;
  assign new_n256_ = new_n255_ & new_n248_;
  assign new_n257_ = new_n256_ & new_n234_;
  assign new_n258_ = new_n257_ & new_n175_;
  assign new_n259_ = ~new_n258_;
  assign new_n260_ = ~i27;
  assign new_n261_ = ~i31 ^ ~new_n260_;
  assign om_27 = ~new_n261_ ^ ~new_n259_;
  assign new_n263_ = ~new_n261_;
  assign new_n264_ = new_n263_ & new_n255_;
  assign new_n265_ = new_n264_ & new_n248_;
  assign new_n266_ = new_n265_ & new_n234_;
  assign new_n267_ = new_n266_ & new_n175_;
  assign new_n268_ = ~new_n267_;
  assign new_n269_ = ~i28;
  assign new_n270_ = ~i31 ^ ~new_n269_;
  assign om_28 = ~new_n270_ ^ ~new_n268_;
  assign new_n272_ = ~new_n270_;
  assign new_n273_ = new_n272_ & new_n265_;
  assign new_n274_ = new_n273_ & new_n234_;
  assign new_n275_ = new_n274_ & new_n175_;
  assign new_n276_ = ~new_n275_;
  assign new_n277_ = ~i29;
  assign new_n278_ = ~i31 ^ ~new_n277_;
  assign om_29 = ~new_n278_ ^ ~new_n276_;
  assign new_n280_ = ~new_n278_;
  assign new_n281_ = new_n280_ & new_n272_;
  assign new_n282_ = new_n281_ & new_n265_;
  assign new_n283_ = new_n282_ & new_n234_;
  assign new_n284_ = new_n283_ & new_n175_;
  assign new_n285_ = ~new_n284_;
  assign new_n286_ = ~i30;
  assign new_n287_ = ~i31 ^ ~new_n286_;
  assign om_30 = ~new_n287_ ^ ~new_n285_;
  assign new_n289_ = ~new_n287_;
  assign new_n290_ = new_n289_ & new_n281_;
  assign new_n291_ = new_n290_ & new_n265_;
  assign new_n292_ = new_n291_ & new_n234_;
  assign om_31 = new_n292_ & new_n175_;
endmodule


