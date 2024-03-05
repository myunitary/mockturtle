// Benchmark "hd12" written by ABC on Wed Dec 27 23:19:07 2023

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
  wire new_n91_, new_n92_, new_n93_, new_n94_, new_n95_, new_n96_, new_n97_,
    new_n98_, new_n99_, new_n100_, new_n101_, new_n102_, new_n103_,
    new_n104_, new_n105_, new_n106_, new_n107_, new_n108_, new_n109_,
    new_n110_, new_n111_, new_n112_, new_n113_, new_n114_, new_n115_,
    new_n116_, new_n117_, new_n118_, new_n119_, new_n120_, new_n121_,
    new_n122_, new_n123_, new_n124_, new_n125_, new_n126_, new_n127_,
    new_n128_, new_n129_, new_n130_, new_n131_, new_n132_, new_n133_,
    new_n134_, new_n135_, new_n136_, new_n137_, new_n138_, new_n139_,
    new_n140_, new_n141_, new_n142_, new_n143_, new_n144_, new_n145_,
    new_n146_, new_n147_, new_n148_, new_n149_, new_n150_, new_n151_,
    new_n152_, new_n153_, new_n154_, new_n155_, new_n156_, new_n157_,
    new_n158_, new_n159_, new_n160_, new_n161_, new_n162_, new_n163_,
    new_n164_, new_n165_, new_n166_, new_n167_, new_n168_, new_n169_,
    new_n170_, new_n171_, new_n172_, new_n173_, new_n174_, new_n175_,
    new_n176_, new_n177_, new_n178_, new_n179_, new_n180_, new_n181_,
    new_n182_, new_n183_, new_n184_, new_n185_, new_n186_, new_n187_,
    new_n188_, new_n189_, new_n190_, new_n191_, new_n192_, new_n193_,
    new_n194_, new_n195_, new_n196_, new_n197_, new_n198_, new_n199_,
    new_n200_, new_n202_, new_n203_, new_n204_, new_n205_, new_n206_,
    new_n207_, new_n208_, new_n209_, new_n210_, new_n211_, new_n212_,
    new_n213_, new_n214_, new_n215_, new_n216_, new_n217_, new_n218_,
    new_n219_, new_n220_, new_n221_, new_n222_, new_n223_, new_n224_,
    new_n225_, new_n226_, new_n227_, new_n228_, new_n229_, new_n230_,
    new_n231_, new_n232_, new_n233_, new_n234_, new_n235_, new_n236_,
    new_n237_, new_n238_, new_n239_, new_n240_, new_n241_, new_n242_,
    new_n243_, new_n244_, new_n245_, new_n247_, new_n248_, new_n249_,
    new_n250_, new_n251_, new_n252_, new_n253_, new_n254_, new_n255_,
    new_n256_, new_n257_, new_n258_, new_n259_, new_n260_, new_n261_,
    new_n262_, new_n263_, new_n264_, new_n265_, new_n266_, new_n268_,
    new_n269_, new_n270_, new_n271_, new_n272_, new_n273_, new_n274_,
    new_n275_, new_n276_, new_n277_, new_n278_, new_n279_, new_n280_,
    new_n281_, new_n282_, new_n283_, new_n284_, new_n285_, new_n286_,
    new_n287_, new_n288_, new_n289_, new_n290_, new_n291_, new_n292_,
    new_n293_, new_n294_, new_n295_, new_n296_, new_n297_, new_n298_,
    new_n299_, new_n300_, new_n301_, new_n302_, new_n303_, new_n304_,
    new_n305_, new_n306_, new_n307_, new_n309_, new_n310_, new_n311_,
    new_n312_, new_n313_, new_n314_, new_n315_, new_n316_, new_n317_,
    new_n318_, new_n319_, new_n320_;
  assign new_n91_ = ~i_16;
  assign new_n92_ = i_17 & new_n91_;
  assign new_n93_ = ~new_n92_ ^ ~i_16;
  assign new_n94_ = ~i_14;
  assign new_n95_ = ~i_15;
  assign new_n96_ = new_n95_ & new_n94_;
  assign new_n97_ = new_n96_ & new_n93_;
  assign new_n98_ = ~new_n97_;
  assign new_n99_ = i_15 & new_n94_;
  assign new_n100_ = ~new_n99_ ^ ~new_n94_;
  assign new_n101_ = ~new_n100_ ^ ~new_n98_;
  assign new_n102_ = ~i_10;
  assign new_n103_ = ~i_11;
  assign new_n104_ = new_n103_ & new_n102_;
  assign new_n105_ = ~i_12;
  assign new_n106_ = ~i_13;
  assign new_n107_ = new_n106_ & new_n105_;
  assign new_n108_ = new_n107_ & new_n104_;
  assign new_n109_ = new_n108_ & new_n101_;
  assign new_n110_ = ~new_n109_;
  assign new_n111_ = i_13 & new_n105_;
  assign new_n112_ = ~new_n111_ ^ ~i_12;
  assign new_n113_ = new_n112_ & new_n104_;
  assign new_n114_ = i_11 & new_n102_;
  assign new_n115_ = ~new_n114_ ^ ~new_n102_;
  assign new_n116_ = ~new_n115_ ^ ~new_n113_;
  assign new_n117_ = ~new_n116_ ^ ~new_n110_;
  assign new_n118_ = ~i_2;
  assign new_n119_ = ~i_3;
  assign new_n120_ = new_n119_ & new_n118_;
  assign new_n121_ = ~i_4;
  assign new_n122_ = ~i_5;
  assign new_n123_ = new_n122_ & new_n121_;
  assign new_n124_ = new_n123_ & new_n120_;
  assign new_n125_ = ~i_6;
  assign new_n126_ = ~i_7;
  assign new_n127_ = new_n126_ & new_n125_;
  assign new_n128_ = ~i_8;
  assign new_n129_ = ~i_9;
  assign new_n130_ = new_n129_ & new_n128_;
  assign new_n131_ = new_n130_ & new_n127_;
  assign new_n132_ = new_n131_ & new_n124_;
  assign new_n133_ = new_n132_ & new_n117_;
  assign new_n134_ = i_9 & new_n128_;
  assign new_n135_ = ~new_n134_ ^ ~i_8;
  assign new_n136_ = new_n135_ & new_n127_;
  assign new_n137_ = ~new_n136_;
  assign new_n138_ = i_7 & new_n125_;
  assign new_n139_ = ~new_n138_ ^ ~new_n125_;
  assign new_n140_ = ~new_n139_ ^ ~new_n137_;
  assign new_n141_ = new_n140_ & new_n124_;
  assign new_n142_ = i_5 & new_n121_;
  assign new_n143_ = ~new_n142_ ^ ~i_4;
  assign new_n144_ = new_n143_ & new_n120_;
  assign new_n145_ = i_3 & new_n118_;
  assign new_n146_ = ~new_n145_ ^ ~i_2;
  assign new_n147_ = ~new_n146_ ^ ~new_n144_;
  assign new_n148_ = ~new_n147_ ^ ~new_n141_;
  assign new_n149_ = ~new_n148_ ^ ~new_n133_;
  assign new_n150_ = new_n149_ & i_2;
  assign new_n151_ = ~new_n149_;
  assign new_n152_ = new_n151_ & i_18;
  assign new_n153_ = ~new_n152_ ^ ~new_n150_;
  assign new_n154_ = new_n149_ & i_8;
  assign new_n155_ = new_n151_ & i_24;
  assign new_n156_ = ~new_n155_ ^ ~new_n154_;
  assign new_n157_ = ~new_n156_;
  assign new_n158_ = new_n149_ & i_9;
  assign new_n159_ = new_n151_ & i_25;
  assign new_n160_ = ~new_n159_ ^ ~new_n158_;
  assign new_n161_ = new_n160_ & new_n157_;
  assign new_n162_ = ~new_n161_ ^ ~new_n156_;
  assign new_n163_ = new_n149_ & i_7;
  assign new_n164_ = new_n151_ & i_23;
  assign new_n165_ = ~new_n164_ ^ ~new_n163_;
  assign new_n166_ = ~new_n165_;
  assign new_n167_ = new_n149_ & i_6;
  assign new_n168_ = new_n151_ & i_22;
  assign new_n169_ = ~new_n168_ ^ ~new_n167_;
  assign new_n170_ = ~new_n169_;
  assign new_n171_ = new_n170_ & new_n166_;
  assign new_n172_ = new_n171_ & new_n162_;
  assign new_n173_ = ~new_n172_;
  assign new_n174_ = new_n170_ & new_n165_;
  assign new_n175_ = ~new_n174_ ^ ~new_n170_;
  assign new_n176_ = ~new_n175_ ^ ~new_n173_;
  assign new_n177_ = ~new_n153_;
  assign new_n178_ = new_n149_ & i_3;
  assign new_n179_ = new_n151_ & i_19;
  assign new_n180_ = ~new_n179_ ^ ~new_n178_;
  assign new_n181_ = ~new_n180_;
  assign new_n182_ = new_n181_ & new_n177_;
  assign new_n183_ = new_n149_ & i_5;
  assign new_n184_ = new_n151_ & i_21;
  assign new_n185_ = ~new_n184_ ^ ~new_n183_;
  assign new_n186_ = ~new_n185_;
  assign new_n187_ = new_n149_ & i_4;
  assign new_n188_ = new_n151_ & i_20;
  assign new_n189_ = ~new_n188_ ^ ~new_n187_;
  assign new_n190_ = ~new_n189_;
  assign new_n191_ = new_n190_ & new_n186_;
  assign new_n192_ = new_n191_ & new_n182_;
  assign new_n193_ = new_n192_ & new_n176_;
  assign new_n194_ = ~new_n193_;
  assign new_n195_ = new_n190_ & new_n185_;
  assign new_n196_ = ~new_n195_ ^ ~new_n189_;
  assign new_n197_ = new_n196_ & new_n182_;
  assign new_n198_ = new_n180_ & new_n177_;
  assign new_n199_ = ~new_n198_ ^ ~new_n153_;
  assign new_n200_ = ~new_n199_ ^ ~new_n197_;
  assign m_28 = ~new_n200_ ^ ~new_n194_;
  assign new_n202_ = ~m_28;
  assign new_n203_ = new_n202_ & new_n153_;
  assign new_n204_ = ~new_n203_;
  assign new_n205_ = new_n149_ & i_10;
  assign new_n206_ = new_n151_ & i_26;
  assign new_n207_ = ~new_n206_ ^ ~new_n205_;
  assign new_n208_ = new_n207_ & m_28;
  assign new_n209_ = ~new_n208_ ^ ~new_n204_;
  assign new_n210_ = ~new_n209_;
  assign new_n211_ = new_n202_ & new_n189_;
  assign new_n212_ = new_n149_ & i_12;
  assign new_n213_ = new_n151_ & i_28;
  assign new_n214_ = ~new_n213_ ^ ~new_n212_;
  assign new_n215_ = new_n214_ & m_28;
  assign new_n216_ = ~new_n215_ ^ ~new_n211_;
  assign new_n217_ = ~new_n216_;
  assign new_n218_ = new_n202_ & new_n185_;
  assign new_n219_ = ~new_n218_;
  assign new_n220_ = new_n149_ & i_13;
  assign new_n221_ = new_n151_ & i_29;
  assign new_n222_ = ~new_n221_ ^ ~new_n220_;
  assign new_n223_ = new_n222_ & m_28;
  assign new_n224_ = ~new_n223_ ^ ~new_n219_;
  assign new_n225_ = new_n224_ & new_n217_;
  assign new_n226_ = ~new_n225_;
  assign new_n227_ = ~new_n224_;
  assign new_n228_ = new_n227_ & new_n217_;
  assign new_n229_ = new_n228_ & new_n216_;
  assign new_n230_ = ~new_n229_;
  assign new_n231_ = new_n202_ & new_n180_;
  assign new_n232_ = new_n149_ & i_11;
  assign new_n233_ = new_n151_ & i_27;
  assign new_n234_ = ~new_n233_ ^ ~new_n232_;
  assign new_n235_ = new_n234_ & m_28;
  assign new_n236_ = ~new_n235_ ^ ~new_n231_;
  assign new_n237_ = ~new_n236_;
  assign new_n238_ = ~new_n208_ ^ ~new_n203_;
  assign new_n239_ = ~new_n238_;
  assign new_n240_ = new_n239_ & new_n237_;
  assign new_n241_ = new_n240_ & new_n230_;
  assign new_n242_ = new_n241_ & new_n226_;
  assign new_n243_ = ~new_n242_;
  assign new_n244_ = new_n239_ & new_n236_;
  assign new_n245_ = ~new_n244_ ^ ~new_n238_;
  assign m_29 = ~new_n245_ ^ ~new_n243_;
  assign new_n247_ = new_n202_ & new_n169_;
  assign new_n248_ = new_n149_ & i_14;
  assign new_n249_ = new_n151_ & i_30;
  assign new_n250_ = ~new_n249_ ^ ~new_n248_;
  assign new_n251_ = new_n250_ & m_28;
  assign new_n252_ = ~new_n251_ ^ ~new_n247_;
  assign new_n253_ = new_n252_ & m_29;
  assign new_n254_ = ~new_n253_ ^ ~new_n210_;
  assign new_n255_ = ~new_n254_;
  assign new_n256_ = ~m_29;
  assign new_n257_ = new_n256_ & new_n236_;
  assign new_n258_ = new_n202_ & new_n165_;
  assign new_n259_ = new_n149_ & i_15;
  assign new_n260_ = new_n151_ & i_31;
  assign new_n261_ = ~new_n260_ ^ ~new_n259_;
  assign new_n262_ = new_n261_ & m_28;
  assign new_n263_ = ~new_n262_ ^ ~new_n258_;
  assign new_n264_ = new_n263_ & m_29;
  assign new_n265_ = ~new_n264_ ^ ~new_n257_;
  assign new_n266_ = new_n265_ & new_n255_;
  assign m_30 = ~new_n266_ ^ ~new_n255_;
  assign new_n268_ = ~m_30;
  assign new_n269_ = ~i_26;
  assign new_n270_ = ~i_27;
  assign new_n271_ = new_n270_ & new_n269_;
  assign new_n272_ = ~i_28;
  assign new_n273_ = ~i_29;
  assign new_n274_ = new_n273_ & new_n272_;
  assign new_n275_ = new_n274_ & new_n271_;
  assign new_n276_ = ~i_22;
  assign new_n277_ = ~i_23;
  assign new_n278_ = new_n277_ & new_n276_;
  assign new_n279_ = ~i_24;
  assign new_n280_ = ~i_25;
  assign new_n281_ = new_n280_ & new_n279_;
  assign new_n282_ = new_n281_ & new_n278_;
  assign new_n283_ = new_n282_ & new_n275_;
  assign new_n284_ = ~i_30;
  assign new_n285_ = ~i_31;
  assign new_n286_ = new_n285_ & new_n284_;
  assign new_n287_ = ~i_32;
  assign new_n288_ = ~i_33;
  assign new_n289_ = new_n288_ & new_n287_;
  assign new_n290_ = new_n289_ & new_n286_;
  assign new_n291_ = new_n290_ & new_n108_;
  assign new_n292_ = new_n291_ & new_n283_;
  assign new_n293_ = ~i_18;
  assign new_n294_ = ~i_19;
  assign new_n295_ = new_n294_ & new_n293_;
  assign new_n296_ = ~i_20;
  assign new_n297_ = ~i_21;
  assign new_n298_ = new_n297_ & new_n296_;
  assign new_n299_ = new_n298_ & new_n295_;
  assign new_n300_ = ~i_17;
  assign new_n301_ = new_n300_ & new_n91_;
  assign new_n302_ = new_n301_ & new_n96_;
  assign new_n303_ = new_n302_ & new_n299_;
  assign new_n304_ = new_n303_ & new_n132_;
  assign new_n305_ = new_n304_ & new_n292_;
  assign new_n306_ = new_n305_ & new_n268_;
  assign new_n307_ = new_n305_ & m_30;
  assign m_26 = ~new_n307_ ^ ~new_n306_;
  assign new_n309_ = ~new_n133_;
  assign new_n310_ = ~new_n253_ ^ ~new_n209_;
  assign new_n311_ = ~new_n215_ ^ ~new_n211_;
  assign new_n312_ = new_n202_ & new_n156_;
  assign new_n313_ = new_n149_ & i_16;
  assign new_n314_ = new_n151_ & i_32;
  assign new_n315_ = ~new_n314_ ^ ~new_n313_;
  assign new_n316_ = new_n315_ & m_28;
  assign new_n317_ = ~new_n316_ ^ ~new_n312_;
  assign new_n318_ = new_n317_ & m_29;
  assign new_n319_ = ~new_n318_ ^ ~new_n311_;
  assign new_n320_ = new_n319_ & m_30;
  assign m_31 = ~new_n320_ ^ ~new_n310_;
  assign m_27 = ~new_n148_ ^ ~new_n309_;
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
endmodule


