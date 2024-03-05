// Benchmark "router" written by ABC on Wed Dec 27 23:19:08 2023

module top ( 
    destx0, destx1, destx2, destx3, destx4, destx5, destx6, destx7, destx8,
    destx9, destx10, destx11, destx12, destx13, destx14, destx15, destx16,
    destx17, destx18, destx19, destx20, destx21, destx22, destx23, destx24,
    destx25, destx26, destx27, destx28, destx29, desty0, desty1, desty2,
    desty3, desty4, desty5, desty6, desty7, desty8, desty9, desty10,
    desty11, desty12, desty13, desty14, desty15, desty16, desty17, desty18,
    desty19, desty20, desty21, desty22, desty23, desty24, desty25, desty26,
    desty27, desty28, desty29,
    outport0, outport1, outport2, outport3, outport4, outport5, outport6,
    outport7, outport8, outport9, outport10, outport11, outport12,
    outport13, outport14, outport15, outport16, outport17, outport18,
    outport19, outport20, outport21, outport22, outport23, outport24,
    outport25, outport26, outport27, outport28, outport29  );
  input  destx0, destx1, destx2, destx3, destx4, destx5, destx6, destx7,
    destx8, destx9, destx10, destx11, destx12, destx13, destx14, destx15,
    destx16, destx17, destx18, destx19, destx20, destx21, destx22, destx23,
    destx24, destx25, destx26, destx27, destx28, destx29, desty0, desty1,
    desty2, desty3, desty4, desty5, desty6, desty7, desty8, desty9,
    desty10, desty11, desty12, desty13, desty14, desty15, desty16, desty17,
    desty18, desty19, desty20, desty21, desty22, desty23, desty24, desty25,
    desty26, desty27, desty28, desty29;
  output outport0, outport1, outport2, outport3, outport4, outport5, outport6,
    outport7, outport8, outport9, outport10, outport11, outport12,
    outport13, outport14, outport15, outport16, outport17, outport18,
    outport19, outport20, outport21, outport22, outport23, outport24,
    outport25, outport26, outport27, outport28, outport29;
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
    new_n200_, new_n201_, new_n202_, new_n203_, new_n204_, new_n205_,
    new_n206_, new_n207_, new_n208_, new_n209_, new_n210_, new_n211_,
    new_n212_, new_n213_, new_n214_, new_n215_, new_n216_, new_n218_,
    new_n219_, new_n220_, new_n221_, new_n222_, new_n223_, new_n224_,
    new_n225_, new_n226_, new_n227_, new_n228_, new_n229_, new_n230_,
    new_n231_, new_n232_, new_n233_, new_n234_, new_n235_, new_n236_,
    new_n237_, new_n238_, new_n239_, new_n240_, new_n241_, new_n242_,
    new_n243_, new_n244_, new_n245_, new_n246_, new_n247_, new_n248_,
    new_n249_, new_n250_, new_n251_, new_n252_, new_n253_, new_n254_,
    new_n255_, new_n256_, new_n257_, new_n258_, new_n259_, new_n260_,
    new_n261_, new_n262_, new_n263_, new_n264_, new_n265_, new_n266_,
    new_n267_, new_n268_, new_n269_, new_n270_, new_n271_, new_n272_,
    new_n273_, new_n274_, new_n275_, new_n276_, new_n277_, new_n278_,
    new_n279_, new_n280_, new_n281_, new_n282_, new_n283_, new_n284_,
    new_n285_, new_n286_, new_n287_, new_n288_, new_n289_, new_n290_,
    new_n291_, new_n292_, new_n293_, new_n294_, new_n295_, new_n296_,
    new_n297_, new_n298_, new_n299_, new_n300_, new_n301_, new_n302_,
    new_n303_, new_n304_, new_n305_, new_n306_, new_n307_, new_n308_,
    new_n309_, new_n310_, new_n311_, new_n312_, new_n313_, new_n314_,
    new_n315_, new_n316_, new_n317_, new_n318_, new_n319_, new_n320_,
    new_n321_, new_n322_, new_n323_, new_n324_, new_n325_, new_n326_,
    new_n327_, new_n328_, new_n329_, new_n330_, new_n331_, new_n332_,
    new_n333_, new_n334_, new_n335_, new_n336_, new_n337_, new_n338_,
    new_n339_, new_n340_, new_n341_, new_n342_, new_n343_, new_n344_,
    new_n345_, new_n346_, new_n347_, new_n348_, new_n349_, new_n350_,
    new_n351_, new_n352_, new_n353_, new_n354_, new_n355_, new_n356_,
    new_n357_, new_n358_, new_n359_, new_n360_, new_n362_, new_n363_,
    new_n364_, new_n365_, new_n366_;
  assign new_n91_ = destx28 & destx27;
  assign new_n92_ = new_n91_ & destx29;
  assign new_n93_ = ~destx26;
  assign new_n94_ = ~destx18;
  assign new_n95_ = ~destx16;
  assign new_n96_ = ~destx9;
  assign new_n97_ = ~destx10;
  assign new_n98_ = new_n97_ & new_n96_;
  assign new_n99_ = ~new_n98_;
  assign new_n100_ = new_n99_ & destx11;
  assign new_n101_ = ~new_n100_;
  assign new_n102_ = ~destx12;
  assign new_n103_ = ~destx13;
  assign new_n104_ = new_n103_ & new_n102_;
  assign new_n105_ = new_n104_ & new_n101_;
  assign new_n106_ = ~new_n105_;
  assign new_n107_ = new_n106_ & destx14;
  assign new_n108_ = new_n107_ & destx15;
  assign new_n109_ = ~new_n108_;
  assign new_n110_ = new_n109_ & new_n95_;
  assign new_n111_ = ~new_n110_;
  assign new_n112_ = new_n111_ & destx17;
  assign new_n113_ = ~new_n112_;
  assign new_n114_ = new_n113_ & new_n94_;
  assign new_n115_ = ~new_n114_;
  assign new_n116_ = destx20 & destx19;
  assign new_n117_ = new_n116_ & new_n115_;
  assign new_n118_ = ~new_n117_;
  assign new_n119_ = ~destx21;
  assign new_n120_ = ~destx22;
  assign new_n121_ = new_n120_ & new_n119_;
  assign new_n122_ = new_n121_ & new_n118_;
  assign new_n123_ = ~new_n122_;
  assign new_n124_ = new_n123_ & destx23;
  assign new_n125_ = new_n124_ & destx24;
  assign new_n126_ = new_n125_ & destx25;
  assign new_n127_ = ~new_n126_;
  assign new_n128_ = new_n127_ & new_n93_;
  assign new_n129_ = ~new_n128_;
  assign new_n130_ = new_n129_ & new_n92_;
  assign new_n131_ = ~destx15;
  assign new_n132_ = ~new_n107_ ^ ~new_n131_;
  assign new_n133_ = new_n108_ & destx16;
  assign new_n134_ = ~new_n133_;
  assign new_n135_ = new_n100_ & new_n102_;
  assign new_n136_ = ~new_n135_;
  assign new_n137_ = ~destx11;
  assign new_n138_ = destx12 & new_n137_;
  assign new_n139_ = new_n138_ & new_n98_;
  assign new_n140_ = ~new_n139_;
  assign new_n141_ = new_n140_ & new_n136_;
  assign new_n142_ = ~new_n141_;
  assign new_n143_ = new_n119_ & destx20;
  assign new_n144_ = destx23 & new_n120_;
  assign new_n145_ = ~destx1;
  assign new_n146_ = ~destx2;
  assign new_n147_ = new_n146_ & new_n145_;
  assign new_n148_ = new_n147_ & new_n144_;
  assign new_n149_ = new_n148_ & new_n143_;
  assign new_n150_ = ~destx7;
  assign new_n151_ = ~destx8;
  assign new_n152_ = new_n151_ & new_n150_;
  assign new_n153_ = new_n97_ & destx9;
  assign new_n154_ = new_n153_ & new_n152_;
  assign new_n155_ = ~destx3;
  assign new_n156_ = ~destx4;
  assign new_n157_ = new_n156_ & new_n155_;
  assign new_n158_ = ~destx5;
  assign new_n159_ = ~destx6;
  assign new_n160_ = new_n159_ & new_n158_;
  assign new_n161_ = new_n160_ & new_n157_;
  assign new_n162_ = new_n161_ & new_n154_;
  assign new_n163_ = destx25 & destx24;
  assign new_n164_ = new_n163_ & new_n93_;
  assign new_n165_ = destx14 & new_n103_;
  assign new_n166_ = destx19 & new_n94_;
  assign new_n167_ = new_n166_ & new_n165_;
  assign new_n168_ = new_n167_ & new_n164_;
  assign new_n169_ = new_n168_ & new_n162_;
  assign new_n170_ = new_n169_ & new_n149_;
  assign new_n171_ = new_n170_ & new_n142_;
  assign new_n172_ = new_n171_ & new_n134_;
  assign new_n173_ = new_n172_ & new_n132_;
  assign new_n174_ = new_n173_ & new_n112_;
  assign new_n175_ = ~new_n174_;
  assign new_n176_ = new_n175_ & new_n130_;
  assign new_n177_ = ~new_n176_;
  assign new_n178_ = ~new_n130_;
  assign new_n179_ = ~destx24;
  assign new_n180_ = ~new_n124_;
  assign new_n181_ = new_n180_ & new_n179_;
  assign new_n182_ = ~new_n181_;
  assign new_n183_ = ~new_n125_;
  assign new_n184_ = ~destx17;
  assign new_n185_ = new_n110_ & new_n184_;
  assign new_n186_ = ~new_n185_;
  assign new_n187_ = destx25 & destx19;
  assign new_n188_ = new_n187_ & new_n93_;
  assign new_n189_ = new_n95_ & destx15;
  assign new_n190_ = new_n189_ & new_n104_;
  assign new_n191_ = new_n190_ & new_n188_;
  assign new_n192_ = new_n191_ & new_n92_;
  assign new_n193_ = destx1 & destx0;
  assign new_n194_ = new_n193_ & new_n98_;
  assign new_n195_ = new_n144_ & new_n143_;
  assign new_n196_ = new_n195_ & new_n194_;
  assign new_n197_ = destx7 & destx6;
  assign new_n198_ = destx11 & destx8;
  assign new_n199_ = new_n198_ & new_n197_;
  assign new_n200_ = destx3 & destx2;
  assign new_n201_ = destx5 & destx4;
  assign new_n202_ = new_n201_ & new_n200_;
  assign new_n203_ = new_n202_ & new_n199_;
  assign new_n204_ = new_n203_ & new_n196_;
  assign new_n205_ = new_n204_ & new_n192_;
  assign new_n206_ = ~destx14;
  assign new_n207_ = ~new_n105_ ^ ~new_n206_;
  assign new_n208_ = new_n207_ & new_n205_;
  assign new_n209_ = new_n208_ & new_n186_;
  assign new_n210_ = new_n209_ & new_n114_;
  assign new_n211_ = new_n210_ & new_n183_;
  assign new_n212_ = new_n211_ & new_n182_;
  assign new_n213_ = ~new_n212_;
  assign new_n214_ = new_n213_ & new_n178_;
  assign new_n215_ = ~new_n214_;
  assign new_n216_ = new_n215_ & new_n177_;
  assign outport0 = ~new_n216_;
  assign new_n218_ = desty28 & desty27;
  assign new_n219_ = new_n218_ & desty29;
  assign new_n220_ = ~desty26;
  assign new_n221_ = ~desty18;
  assign new_n222_ = ~desty16;
  assign new_n223_ = ~desty13;
  assign new_n224_ = ~desty12;
  assign new_n225_ = ~desty9;
  assign new_n226_ = ~desty10;
  assign new_n227_ = new_n226_ & new_n225_;
  assign new_n228_ = ~new_n227_;
  assign new_n229_ = new_n228_ & desty11;
  assign new_n230_ = ~new_n229_;
  assign new_n231_ = new_n230_ & new_n224_;
  assign new_n232_ = new_n231_ & new_n223_;
  assign new_n233_ = ~new_n232_;
  assign new_n234_ = new_n233_ & desty14;
  assign new_n235_ = new_n234_ & desty15;
  assign new_n236_ = ~new_n235_;
  assign new_n237_ = new_n236_ & new_n222_;
  assign new_n238_ = ~new_n237_;
  assign new_n239_ = new_n238_ & desty17;
  assign new_n240_ = ~new_n239_;
  assign new_n241_ = new_n240_ & new_n221_;
  assign new_n242_ = ~new_n241_;
  assign new_n243_ = desty20 & desty19;
  assign new_n244_ = new_n243_ & new_n242_;
  assign new_n245_ = ~new_n244_;
  assign new_n246_ = ~desty21;
  assign new_n247_ = ~desty22;
  assign new_n248_ = new_n247_ & new_n246_;
  assign new_n249_ = new_n248_ & new_n245_;
  assign new_n250_ = ~new_n249_;
  assign new_n251_ = desty25 & desty24;
  assign new_n252_ = new_n251_ & desty23;
  assign new_n253_ = new_n252_ & new_n250_;
  assign new_n254_ = ~new_n253_;
  assign new_n255_ = new_n254_ & new_n220_;
  assign new_n256_ = ~new_n255_;
  assign new_n257_ = new_n256_ & new_n219_;
  assign new_n258_ = ~new_n257_;
  assign new_n259_ = ~destx0;
  assign new_n260_ = new_n246_ & new_n259_;
  assign new_n261_ = ~new_n260_;
  assign new_n262_ = new_n261_ & new_n258_;
  assign new_n263_ = ~new_n262_;
  assign new_n264_ = ~desty23;
  assign new_n265_ = new_n264_ & desty22;
  assign new_n266_ = ~new_n265_;
  assign new_n267_ = new_n266_ & new_n244_;
  assign new_n268_ = ~new_n267_;
  assign new_n269_ = desty23 & new_n247_;
  assign new_n270_ = ~new_n269_;
  assign new_n271_ = new_n270_ & new_n245_;
  assign new_n272_ = ~new_n271_;
  assign new_n273_ = ~desty19;
  assign new_n274_ = new_n239_ & new_n273_;
  assign new_n275_ = ~new_n274_;
  assign new_n276_ = new_n275_ & new_n242_;
  assign new_n277_ = ~new_n276_;
  assign new_n278_ = ~desty14;
  assign new_n279_ = new_n229_ & desty12;
  assign new_n280_ = new_n279_ & desty13;
  assign new_n281_ = ~new_n280_;
  assign new_n282_ = new_n281_ & new_n278_;
  assign new_n283_ = ~new_n282_;
  assign new_n284_ = ~new_n234_;
  assign new_n285_ = desty1 & desty0;
  assign new_n286_ = new_n285_ & new_n251_;
  assign new_n287_ = new_n273_ & new_n221_;
  assign new_n288_ = ~new_n287_;
  assign new_n289_ = new_n288_ & new_n227_;
  assign new_n290_ = new_n289_ & new_n286_;
  assign new_n291_ = desty7 & desty6;
  assign new_n292_ = desty11 & desty8;
  assign new_n293_ = new_n292_ & new_n291_;
  assign new_n294_ = desty3 & desty2;
  assign new_n295_ = desty5 & desty4;
  assign new_n296_ = new_n295_ & new_n294_;
  assign new_n297_ = new_n296_ & new_n293_;
  assign new_n298_ = desty17 & new_n222_;
  assign new_n299_ = new_n220_ & desty20;
  assign new_n300_ = new_n299_ & new_n298_;
  assign new_n301_ = new_n300_ & new_n219_;
  assign new_n302_ = new_n301_ & new_n297_;
  assign new_n303_ = new_n302_ & new_n290_;
  assign new_n304_ = new_n303_ & new_n284_;
  assign new_n305_ = new_n304_ & new_n283_;
  assign new_n306_ = ~new_n234_ ^ ~desty15;
  assign new_n307_ = new_n306_ & new_n305_;
  assign new_n308_ = new_n307_ & new_n277_;
  assign new_n309_ = new_n308_ & new_n272_;
  assign new_n310_ = new_n309_ & new_n268_;
  assign new_n311_ = new_n310_ & new_n263_;
  assign new_n312_ = ~new_n311_;
  assign new_n313_ = new_n237_ & desty18;
  assign new_n314_ = ~new_n313_;
  assign new_n315_ = new_n314_ & new_n240_;
  assign new_n316_ = ~new_n315_;
  assign new_n317_ = new_n246_ & new_n222_;
  assign new_n318_ = desty27 & new_n220_;
  assign new_n319_ = new_n318_ & new_n317_;
  assign new_n320_ = new_n223_ & desty11;
  assign new_n321_ = desty15 & desty14;
  assign new_n322_ = new_n321_ & new_n320_;
  assign new_n323_ = ~desty7;
  assign new_n324_ = ~desty8;
  assign new_n325_ = new_n324_ & new_n323_;
  assign new_n326_ = new_n226_ & desty9;
  assign new_n327_ = new_n326_ & new_n325_;
  assign new_n328_ = new_n327_ & new_n322_;
  assign new_n329_ = new_n328_ & new_n319_;
  assign new_n330_ = desty18 & desty17;
  assign new_n331_ = ~new_n330_;
  assign new_n332_ = new_n269_ & new_n243_;
  assign new_n333_ = new_n332_ & new_n331_;
  assign new_n334_ = ~desty3;
  assign new_n335_ = ~desty4;
  assign new_n336_ = new_n335_ & new_n334_;
  assign new_n337_ = ~desty5;
  assign new_n338_ = ~desty6;
  assign new_n339_ = new_n338_ & new_n337_;
  assign new_n340_ = new_n339_ & new_n336_;
  assign new_n341_ = ~desty1;
  assign new_n342_ = ~desty2;
  assign new_n343_ = new_n342_ & new_n341_;
  assign new_n344_ = new_n343_ & new_n251_;
  assign new_n345_ = new_n344_ & new_n340_;
  assign new_n346_ = new_n345_ & new_n333_;
  assign new_n347_ = ~new_n229_ ^ ~desty12;
  assign new_n348_ = new_n347_ & new_n346_;
  assign new_n349_ = new_n348_ & new_n329_;
  assign new_n350_ = new_n349_ & new_n316_;
  assign new_n351_ = ~desty0;
  assign new_n352_ = new_n351_ & new_n259_;
  assign new_n353_ = new_n352_ & new_n350_;
  assign new_n354_ = ~new_n353_;
  assign new_n355_ = new_n354_ & new_n257_;
  assign new_n356_ = ~new_n355_;
  assign new_n357_ = new_n356_ & new_n312_;
  assign new_n358_ = ~new_n357_;
  assign new_n359_ = new_n358_ & new_n215_;
  assign new_n360_ = ~new_n359_;
  assign outport1 = new_n360_ & new_n177_;
  assign new_n362_ = desty0 & destx0;
  assign new_n363_ = ~new_n362_;
  assign new_n364_ = new_n363_ & new_n350_;
  assign new_n365_ = ~new_n364_;
  assign new_n366_ = new_n365_ & new_n257_;
  assign outport2 = new_n366_ & new_n216_;
  assign outport3 = 1'b0;
  assign outport4 = 1'b0;
  assign outport5 = 1'b0;
  assign outport6 = 1'b0;
  assign outport7 = 1'b0;
  assign outport8 = 1'b0;
  assign outport9 = 1'b0;
  assign outport10 = 1'b0;
  assign outport11 = 1'b0;
  assign outport12 = 1'b0;
  assign outport13 = 1'b0;
  assign outport14 = 1'b0;
  assign outport15 = 1'b0;
  assign outport16 = 1'b0;
  assign outport17 = 1'b0;
  assign outport18 = 1'b0;
  assign outport19 = 1'b0;
  assign outport20 = 1'b0;
  assign outport21 = 1'b0;
  assign outport22 = 1'b0;
  assign outport23 = 1'b0;
  assign outport24 = 1'b0;
  assign outport25 = 1'b0;
  assign outport26 = 1'b0;
  assign outport27 = 1'b0;
  assign outport28 = 1'b0;
  assign outport29 = 1'b0;
endmodule


