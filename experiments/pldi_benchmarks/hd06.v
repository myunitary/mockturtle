// Benchmark "hd06" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15,
    i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26, i27, i28, i29,
    i30, i31, i32, i33, i34, i35, i36, i37, i38, i39, i40, i41, i42, i43,
    i44, i45, i46, i47, i48, i49, i50, i51, i52, i53, i54, i55, i56, i57,
    i58, i59, i60, i61, i62, i63,
    om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7, om_8, om_9, om_10,
    om_11, om_12, om_13, om_14, om_15, om_16, om_17, om_18, om_19, om_20,
    om_21, om_22, om_23, om_24, om_25, om_26, om_27, om_28, om_29, om_30,
    om_31  );
  input  i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14,
    i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26, i27, i28,
    i29, i30, i31, i32, i33, i34, i35, i36, i37, i38, i39, i40, i41, i42,
    i43, i44, i45, i46, i47, i48, i49, i50, i51, i52, i53, i54, i55, i56,
    i57, i58, i59, i60, i61, i62, i63;
  output om_0, om_1, om_2, om_3, om_4, om_5, om_6, om_7, om_8, om_9, om_10,
    om_11, om_12, om_13, om_14, om_15, om_16, om_17, om_18, om_19, om_20,
    om_21, om_22, om_23, om_24, om_25, om_26, om_27, om_28, om_29, om_30,
    om_31;
  wire new_n97_, new_n98_, new_n99_, new_n100_, new_n101_, new_n102_,
    new_n103_, new_n104_, new_n105_, new_n106_, new_n107_, new_n108_,
    new_n109_, new_n110_, new_n111_, new_n112_, new_n113_, new_n114_,
    new_n115_, new_n116_, new_n117_, new_n118_, new_n119_, new_n120_,
    new_n121_, new_n122_, new_n123_, new_n124_, new_n125_, new_n126_,
    new_n127_, new_n128_, new_n129_, new_n130_, new_n131_, new_n132_,
    new_n133_, new_n134_, new_n135_, new_n136_, new_n137_, new_n138_,
    new_n139_, new_n140_, new_n141_, new_n142_, new_n143_, new_n144_,
    new_n145_, new_n146_, new_n147_, new_n148_, new_n149_, new_n150_,
    new_n151_, new_n152_, new_n153_, new_n154_, new_n155_, new_n156_,
    new_n157_, new_n158_, new_n159_, new_n160_, new_n161_, new_n162_,
    new_n163_, new_n164_, new_n165_, new_n166_, new_n167_, new_n168_,
    new_n169_, new_n170_, new_n171_, new_n172_, new_n173_, new_n174_,
    new_n175_, new_n176_, new_n177_, new_n178_, new_n179_, new_n180_,
    new_n181_, new_n182_, new_n183_, new_n184_, new_n185_, new_n186_,
    new_n187_, new_n188_, new_n189_, new_n190_, new_n191_, new_n192_,
    new_n193_, new_n194_, new_n195_, new_n196_, new_n197_, new_n198_,
    new_n199_, new_n200_, new_n201_, new_n202_, new_n203_, new_n204_,
    new_n205_, new_n206_, new_n207_, new_n208_, new_n209_, new_n210_,
    new_n211_, new_n212_, new_n213_, new_n214_, new_n215_, new_n216_,
    new_n217_, new_n218_, new_n219_, new_n220_, new_n221_, new_n222_,
    new_n223_, new_n224_, new_n225_, new_n226_, new_n227_, new_n228_,
    new_n229_, new_n230_, new_n231_, new_n232_, new_n233_, new_n234_,
    new_n235_, new_n236_, new_n237_, new_n238_, new_n239_, new_n240_,
    new_n241_, new_n242_, new_n243_, new_n244_, new_n245_, new_n246_,
    new_n247_, new_n248_, new_n249_, new_n250_, new_n251_, new_n252_,
    new_n253_, new_n254_, new_n255_, new_n256_, new_n257_, new_n258_,
    new_n259_, new_n260_, new_n261_, new_n262_, new_n263_, new_n264_,
    new_n265_, new_n266_, new_n267_, new_n268_, new_n269_, new_n270_,
    new_n271_, new_n272_, new_n273_, new_n274_, new_n275_, new_n276_,
    new_n277_, new_n278_, new_n279_, new_n280_, new_n281_, new_n282_,
    new_n283_, new_n284_, new_n285_, new_n286_, new_n287_, new_n288_,
    new_n289_, new_n290_, new_n291_, new_n292_, new_n293_, new_n294_,
    new_n295_, new_n296_, new_n297_, new_n298_, new_n299_, new_n300_,
    new_n301_, new_n302_, new_n303_, new_n304_, new_n305_, new_n306_,
    new_n307_, new_n308_, new_n309_, new_n310_, new_n311_, new_n312_,
    new_n313_, new_n314_, new_n315_, new_n316_, new_n317_, new_n318_,
    new_n319_, new_n320_, new_n321_, new_n322_, new_n323_, new_n324_,
    new_n325_, new_n326_, new_n327_, new_n328_, new_n330_, new_n332_,
    new_n334_, new_n336_, new_n338_, new_n340_, new_n342_, new_n344_,
    new_n346_, new_n348_, new_n350_, new_n352_, new_n354_, new_n356_,
    new_n358_, new_n360_, new_n362_, new_n364_, new_n366_, new_n368_,
    new_n370_, new_n372_, new_n374_, new_n376_, new_n378_, new_n380_,
    new_n382_, new_n384_, new_n386_, new_n388_, new_n390_;
  assign new_n97_ = ~i33 ^ ~i1;
  assign new_n98_ = ~new_n97_;
  assign new_n99_ = ~i32;
  assign new_n100_ = new_n99_ & i0;
  assign new_n101_ = new_n100_ & new_n98_;
  assign new_n102_ = ~i33;
  assign new_n103_ = new_n102_ & i1;
  assign new_n104_ = ~new_n103_ ^ ~new_n101_;
  assign new_n105_ = ~i34 ^ ~i2;
  assign new_n106_ = ~new_n105_;
  assign new_n107_ = ~i35 ^ ~i3;
  assign new_n108_ = ~new_n107_;
  assign new_n109_ = new_n108_ & new_n106_;
  assign new_n110_ = new_n109_ & new_n104_;
  assign new_n111_ = ~new_n110_;
  assign new_n112_ = ~i34;
  assign new_n113_ = new_n112_ & i2;
  assign new_n114_ = new_n113_ & new_n108_;
  assign new_n115_ = ~new_n114_;
  assign new_n116_ = ~i35;
  assign new_n117_ = new_n116_ & i3;
  assign new_n118_ = ~new_n117_ ^ ~new_n115_;
  assign new_n119_ = ~new_n118_ ^ ~new_n111_;
  assign new_n120_ = ~i38 ^ ~i6;
  assign new_n121_ = ~new_n120_;
  assign new_n122_ = ~i39 ^ ~i7;
  assign new_n123_ = ~new_n122_;
  assign new_n124_ = new_n123_ & new_n121_;
  assign new_n125_ = ~i36 ^ ~i4;
  assign new_n126_ = ~new_n125_;
  assign new_n127_ = ~i37 ^ ~i5;
  assign new_n128_ = ~new_n127_;
  assign new_n129_ = new_n128_ & new_n126_;
  assign new_n130_ = new_n129_ & new_n124_;
  assign new_n131_ = new_n130_ & new_n119_;
  assign new_n132_ = ~new_n131_;
  assign new_n133_ = ~i36;
  assign new_n134_ = new_n133_ & i4;
  assign new_n135_ = new_n134_ & new_n128_;
  assign new_n136_ = ~i37;
  assign new_n137_ = new_n136_ & i5;
  assign new_n138_ = ~new_n137_ ^ ~new_n135_;
  assign new_n139_ = new_n138_ & new_n124_;
  assign new_n140_ = ~i38;
  assign new_n141_ = new_n140_ & i6;
  assign new_n142_ = new_n141_ & new_n123_;
  assign new_n143_ = ~new_n142_;
  assign new_n144_ = ~i39;
  assign new_n145_ = new_n144_ & i7;
  assign new_n146_ = ~new_n145_ ^ ~new_n143_;
  assign new_n147_ = ~new_n146_ ^ ~new_n139_;
  assign new_n148_ = ~new_n147_ ^ ~new_n132_;
  assign new_n149_ = ~i46 ^ ~i14;
  assign new_n150_ = ~new_n149_;
  assign new_n151_ = ~i47 ^ ~i15;
  assign new_n152_ = ~new_n151_;
  assign new_n153_ = new_n152_ & new_n150_;
  assign new_n154_ = ~i44 ^ ~i12;
  assign new_n155_ = ~new_n154_;
  assign new_n156_ = ~i45 ^ ~i13;
  assign new_n157_ = ~new_n156_;
  assign new_n158_ = new_n157_ & new_n155_;
  assign new_n159_ = new_n158_ & new_n153_;
  assign new_n160_ = ~i42 ^ ~i10;
  assign new_n161_ = ~new_n160_;
  assign new_n162_ = ~i43 ^ ~i11;
  assign new_n163_ = ~new_n162_;
  assign new_n164_ = new_n163_ & new_n161_;
  assign new_n165_ = ~i40 ^ ~i8;
  assign new_n166_ = ~new_n165_;
  assign new_n167_ = ~i41 ^ ~i9;
  assign new_n168_ = ~new_n167_;
  assign new_n169_ = new_n168_ & new_n166_;
  assign new_n170_ = new_n169_ & new_n164_;
  assign new_n171_ = new_n170_ & new_n159_;
  assign new_n172_ = new_n171_ & new_n148_;
  assign new_n173_ = ~new_n172_;
  assign new_n174_ = ~i40;
  assign new_n175_ = new_n174_ & i8;
  assign new_n176_ = new_n175_ & new_n168_;
  assign new_n177_ = ~i41;
  assign new_n178_ = new_n177_ & i9;
  assign new_n179_ = ~new_n178_ ^ ~new_n176_;
  assign new_n180_ = new_n179_ & new_n164_;
  assign new_n181_ = ~new_n180_;
  assign new_n182_ = ~i42;
  assign new_n183_ = new_n182_ & i10;
  assign new_n184_ = new_n183_ & new_n163_;
  assign new_n185_ = ~new_n184_;
  assign new_n186_ = ~i43;
  assign new_n187_ = new_n186_ & i11;
  assign new_n188_ = ~new_n187_ ^ ~new_n185_;
  assign new_n189_ = ~new_n188_ ^ ~new_n181_;
  assign new_n190_ = new_n189_ & new_n159_;
  assign new_n191_ = ~i44;
  assign new_n192_ = new_n191_ & i12;
  assign new_n193_ = new_n192_ & new_n157_;
  assign new_n194_ = ~i45;
  assign new_n195_ = new_n194_ & i13;
  assign new_n196_ = ~new_n195_ ^ ~new_n193_;
  assign new_n197_ = new_n196_ & new_n153_;
  assign new_n198_ = ~i46;
  assign new_n199_ = new_n198_ & i14;
  assign new_n200_ = new_n199_ & new_n152_;
  assign new_n201_ = ~new_n200_;
  assign new_n202_ = ~i47;
  assign new_n203_ = new_n202_ & i15;
  assign new_n204_ = ~new_n203_ ^ ~new_n201_;
  assign new_n205_ = ~new_n204_ ^ ~new_n197_;
  assign new_n206_ = ~new_n205_ ^ ~new_n190_;
  assign new_n207_ = ~new_n206_ ^ ~new_n173_;
  assign new_n208_ = ~i62 ^ ~i30;
  assign new_n209_ = ~new_n208_;
  assign new_n210_ = ~i63 ^ ~i31;
  assign new_n211_ = ~new_n210_;
  assign new_n212_ = new_n211_ & new_n209_;
  assign new_n213_ = ~i60 ^ ~i28;
  assign new_n214_ = ~new_n213_;
  assign new_n215_ = ~i61 ^ ~i29;
  assign new_n216_ = ~new_n215_;
  assign new_n217_ = new_n216_ & new_n214_;
  assign new_n218_ = new_n217_ & new_n212_;
  assign new_n219_ = ~i58 ^ ~i26;
  assign new_n220_ = ~new_n219_;
  assign new_n221_ = ~i59 ^ ~i27;
  assign new_n222_ = ~new_n221_;
  assign new_n223_ = new_n222_ & new_n220_;
  assign new_n224_ = ~i56 ^ ~i24;
  assign new_n225_ = ~new_n224_;
  assign new_n226_ = ~i57 ^ ~i25;
  assign new_n227_ = ~new_n226_;
  assign new_n228_ = new_n227_ & new_n225_;
  assign new_n229_ = new_n228_ & new_n223_;
  assign new_n230_ = new_n229_ & new_n218_;
  assign new_n231_ = ~i54 ^ ~i22;
  assign new_n232_ = ~new_n231_;
  assign new_n233_ = ~i55 ^ ~i23;
  assign new_n234_ = ~new_n233_;
  assign new_n235_ = new_n234_ & new_n232_;
  assign new_n236_ = ~i52 ^ ~i20;
  assign new_n237_ = ~new_n236_;
  assign new_n238_ = ~i53 ^ ~i21;
  assign new_n239_ = ~new_n238_;
  assign new_n240_ = new_n239_ & new_n237_;
  assign new_n241_ = new_n240_ & new_n235_;
  assign new_n242_ = ~i50 ^ ~i18;
  assign new_n243_ = ~new_n242_;
  assign new_n244_ = ~i51 ^ ~i19;
  assign new_n245_ = ~new_n244_;
  assign new_n246_ = new_n245_ & new_n243_;
  assign new_n247_ = ~i48 ^ ~i16;
  assign new_n248_ = ~new_n247_;
  assign new_n249_ = ~i49 ^ ~i17;
  assign new_n250_ = ~new_n249_;
  assign new_n251_ = new_n250_ & new_n248_;
  assign new_n252_ = new_n251_ & new_n246_;
  assign new_n253_ = new_n252_ & new_n241_;
  assign new_n254_ = new_n253_ & new_n230_;
  assign new_n255_ = new_n254_ & new_n207_;
  assign new_n256_ = ~new_n255_;
  assign new_n257_ = ~i48;
  assign new_n258_ = new_n257_ & i16;
  assign new_n259_ = new_n258_ & new_n250_;
  assign new_n260_ = ~i49;
  assign new_n261_ = new_n260_ & i17;
  assign new_n262_ = ~new_n261_ ^ ~new_n259_;
  assign new_n263_ = new_n262_ & new_n246_;
  assign new_n264_ = ~new_n263_;
  assign new_n265_ = ~i50;
  assign new_n266_ = new_n265_ & i18;
  assign new_n267_ = new_n266_ & new_n245_;
  assign new_n268_ = ~new_n267_;
  assign new_n269_ = ~i51;
  assign new_n270_ = new_n269_ & i19;
  assign new_n271_ = ~new_n270_ ^ ~new_n268_;
  assign new_n272_ = ~new_n271_ ^ ~new_n264_;
  assign new_n273_ = new_n272_ & new_n241_;
  assign new_n274_ = ~new_n273_;
  assign new_n275_ = ~i52;
  assign new_n276_ = new_n275_ & i20;
  assign new_n277_ = new_n276_ & new_n239_;
  assign new_n278_ = ~i53;
  assign new_n279_ = new_n278_ & i21;
  assign new_n280_ = ~new_n279_ ^ ~new_n277_;
  assign new_n281_ = new_n280_ & new_n235_;
  assign new_n282_ = ~i54;
  assign new_n283_ = new_n282_ & i22;
  assign new_n284_ = new_n283_ & new_n234_;
  assign new_n285_ = ~new_n284_;
  assign new_n286_ = ~i55;
  assign new_n287_ = new_n286_ & i23;
  assign new_n288_ = ~new_n287_ ^ ~new_n285_;
  assign new_n289_ = ~new_n288_ ^ ~new_n281_;
  assign new_n290_ = ~new_n289_ ^ ~new_n274_;
  assign new_n291_ = new_n290_ & new_n230_;
  assign new_n292_ = ~i56;
  assign new_n293_ = new_n292_ & i24;
  assign new_n294_ = new_n293_ & new_n227_;
  assign new_n295_ = ~i57;
  assign new_n296_ = new_n295_ & i25;
  assign new_n297_ = ~new_n296_ ^ ~new_n294_;
  assign new_n298_ = new_n297_ & new_n223_;
  assign new_n299_ = ~new_n298_;
  assign new_n300_ = ~i58;
  assign new_n301_ = new_n300_ & i26;
  assign new_n302_ = new_n301_ & new_n222_;
  assign new_n303_ = ~new_n302_;
  assign new_n304_ = ~i59;
  assign new_n305_ = new_n304_ & i27;
  assign new_n306_ = ~new_n305_ ^ ~new_n303_;
  assign new_n307_ = ~new_n306_ ^ ~new_n299_;
  assign new_n308_ = new_n307_ & new_n218_;
  assign new_n309_ = ~i60;
  assign new_n310_ = new_n309_ & i28;
  assign new_n311_ = new_n310_ & new_n216_;
  assign new_n312_ = ~i61;
  assign new_n313_ = new_n312_ & i29;
  assign new_n314_ = ~new_n313_ ^ ~new_n311_;
  assign new_n315_ = new_n314_ & new_n212_;
  assign new_n316_ = ~i62;
  assign new_n317_ = new_n316_ & i30;
  assign new_n318_ = new_n317_ & new_n211_;
  assign new_n319_ = ~new_n318_;
  assign new_n320_ = ~i63;
  assign new_n321_ = new_n320_ & i31;
  assign new_n322_ = ~new_n321_ ^ ~new_n319_;
  assign new_n323_ = ~new_n322_ ^ ~new_n315_;
  assign new_n324_ = ~new_n323_ ^ ~new_n308_;
  assign new_n325_ = ~new_n324_ ^ ~new_n291_;
  assign new_n326_ = ~new_n325_ ^ ~new_n256_;
  assign new_n327_ = ~i32 ^ ~i0;
  assign new_n328_ = new_n327_ & new_n326_;
  assign om_0 = ~new_n328_ ^ ~i32;
  assign new_n330_ = new_n326_ & new_n97_;
  assign om_1 = ~new_n330_ ^ ~i33;
  assign new_n332_ = new_n326_ & new_n105_;
  assign om_2 = ~new_n332_ ^ ~i34;
  assign new_n334_ = new_n326_ & new_n107_;
  assign om_3 = ~new_n334_ ^ ~i35;
  assign new_n336_ = new_n326_ & new_n125_;
  assign om_4 = ~new_n336_ ^ ~i36;
  assign new_n338_ = new_n326_ & new_n127_;
  assign om_5 = ~new_n338_ ^ ~i37;
  assign new_n340_ = new_n326_ & new_n120_;
  assign om_6 = ~new_n340_ ^ ~i38;
  assign new_n342_ = new_n326_ & new_n122_;
  assign om_7 = ~new_n342_ ^ ~i39;
  assign new_n344_ = new_n326_ & new_n165_;
  assign om_8 = ~new_n344_ ^ ~i40;
  assign new_n346_ = new_n326_ & new_n167_;
  assign om_9 = ~new_n346_ ^ ~i41;
  assign new_n348_ = new_n326_ & new_n160_;
  assign om_10 = ~new_n348_ ^ ~i42;
  assign new_n350_ = new_n326_ & new_n162_;
  assign om_11 = ~new_n350_ ^ ~i43;
  assign new_n352_ = new_n326_ & new_n154_;
  assign om_12 = ~new_n352_ ^ ~i44;
  assign new_n354_ = new_n326_ & new_n156_;
  assign om_13 = ~new_n354_ ^ ~i45;
  assign new_n356_ = new_n326_ & new_n149_;
  assign om_14 = ~new_n356_ ^ ~i46;
  assign new_n358_ = new_n326_ & new_n151_;
  assign om_15 = ~new_n358_ ^ ~i47;
  assign new_n360_ = new_n326_ & new_n247_;
  assign om_16 = ~new_n360_ ^ ~i48;
  assign new_n362_ = new_n326_ & new_n249_;
  assign om_17 = ~new_n362_ ^ ~i49;
  assign new_n364_ = new_n326_ & new_n242_;
  assign om_18 = ~new_n364_ ^ ~i50;
  assign new_n366_ = new_n326_ & new_n244_;
  assign om_19 = ~new_n366_ ^ ~i51;
  assign new_n368_ = new_n326_ & new_n236_;
  assign om_20 = ~new_n368_ ^ ~i52;
  assign new_n370_ = new_n326_ & new_n238_;
  assign om_21 = ~new_n370_ ^ ~i53;
  assign new_n372_ = new_n326_ & new_n231_;
  assign om_22 = ~new_n372_ ^ ~i54;
  assign new_n374_ = new_n326_ & new_n233_;
  assign om_23 = ~new_n374_ ^ ~i55;
  assign new_n376_ = new_n326_ & new_n224_;
  assign om_24 = ~new_n376_ ^ ~i56;
  assign new_n378_ = new_n326_ & new_n226_;
  assign om_25 = ~new_n378_ ^ ~i57;
  assign new_n380_ = new_n326_ & new_n219_;
  assign om_26 = ~new_n380_ ^ ~i58;
  assign new_n382_ = new_n326_ & new_n221_;
  assign om_27 = ~new_n382_ ^ ~i59;
  assign new_n384_ = new_n326_ & new_n213_;
  assign om_28 = ~new_n384_ ^ ~i60;
  assign new_n386_ = new_n326_ & new_n215_;
  assign om_29 = ~new_n386_ ^ ~i61;
  assign new_n388_ = new_n326_ & new_n208_;
  assign om_30 = ~new_n388_ ^ ~i62;
  assign new_n390_ = new_n326_ & new_n210_;
  assign om_31 = ~new_n390_ ^ ~i63;
endmodule


