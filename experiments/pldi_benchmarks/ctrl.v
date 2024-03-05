// Benchmark "ctrl" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    opcode0, opcode1, opcode2, opcode3, opcode4, opext0, opext1,
    oseloregdst0, oseloregdst1, oselopB0, oselopB1, oaluop0, oaluop1,
    oaluop2, oaluopext0, oaluopext1, oaluopext2, oaluopext3, ohalt,
    oregwrite, oselpcopA, oselpcopB, obeqz, obnez, obgez, obltz, ojump,
    oCin, oinvA, oinvB, osign, omemwrite, oselwb  );
  input  opcode0, opcode1, opcode2, opcode3, opcode4, opext0, opext1;
  output oseloregdst0, oseloregdst1, oselopB0, oselopB1, oaluop0, oaluop1,
    oaluop2, oaluopext0, oaluopext1, oaluopext2, oaluopext3, ohalt,
    oregwrite, oselpcopA, oselpcopB, obeqz, obnez, obgez, obltz, ojump,
    oCin, oinvA, oinvB, osign, omemwrite, oselwb;
  wire new_n34_, new_n35_, new_n36_, new_n37_, new_n38_, new_n39_, new_n40_,
    new_n41_, new_n42_, new_n43_, new_n44_, new_n45_, new_n46_, new_n47_,
    new_n48_, new_n49_, new_n50_, new_n51_, new_n53_, new_n54_, new_n55_,
    new_n56_, new_n57_, new_n58_, new_n59_, new_n60_, new_n61_, new_n62_,
    new_n63_, new_n64_, new_n65_, new_n66_, new_n67_, new_n68_, new_n70_,
    new_n71_, new_n72_, new_n73_, new_n74_, new_n75_, new_n76_, new_n77_,
    new_n79_, new_n80_, new_n81_, new_n82_, new_n83_, new_n84_, new_n85_,
    new_n86_, new_n88_, new_n89_, new_n90_, new_n91_, new_n92_, new_n93_,
    new_n94_, new_n95_, new_n96_, new_n97_, new_n98_, new_n99_, new_n100_,
    new_n101_, new_n102_, new_n103_, new_n104_, new_n105_, new_n106_,
    new_n107_, new_n108_, new_n109_, new_n110_, new_n111_, new_n113_,
    new_n114_, new_n115_, new_n116_, new_n118_, new_n119_, new_n120_,
    new_n121_, new_n122_, new_n123_, new_n124_, new_n125_, new_n127_,
    new_n128_, new_n129_, new_n130_, new_n131_, new_n132_, new_n133_,
    new_n135_, new_n136_, new_n137_, new_n138_, new_n139_, new_n141_,
    new_n142_, new_n143_, new_n145_, new_n146_, new_n147_, new_n148_,
    new_n149_, new_n150_, new_n151_, new_n152_, new_n153_, new_n154_,
    new_n155_, new_n157_, new_n159_, new_n160_, new_n161_, new_n162_,
    new_n163_, new_n164_, new_n165_, new_n166_, new_n167_, new_n168_,
    new_n169_, new_n170_, new_n171_, new_n176_, new_n178_, new_n180_,
    new_n183_, new_n184_, new_n185_, new_n186_, new_n187_, new_n188_,
    new_n189_, new_n190_, new_n191_, new_n192_, new_n193_, new_n195_,
    new_n196_, new_n197_, new_n198_, new_n199_, new_n200_, new_n201_,
    new_n202_, new_n204_, new_n205_, new_n206_, new_n207_, new_n210_,
    new_n211_, new_n213_;
  assign new_n34_ = opcode4 & opcode3;
  assign new_n35_ = ~opcode0;
  assign new_n36_ = ~opcode1;
  assign new_n37_ = new_n36_ & new_n35_;
  assign new_n38_ = ~new_n37_;
  assign new_n39_ = new_n38_ & new_n34_;
  assign new_n40_ = ~new_n39_;
  assign new_n41_ = ~new_n34_;
  assign new_n42_ = ~opcode3;
  assign new_n43_ = ~opcode4;
  assign new_n44_ = new_n43_ & new_n42_;
  assign new_n45_ = new_n44_ & opcode1;
  assign new_n46_ = ~new_n45_;
  assign new_n47_ = new_n46_ & new_n41_;
  assign new_n48_ = ~new_n47_;
  assign new_n49_ = new_n48_ & opcode2;
  assign new_n50_ = ~new_n49_;
  assign new_n51_ = new_n50_ & new_n40_;
  assign oseloregdst0 = ~new_n51_;
  assign new_n53_ = ~opcode2;
  assign new_n54_ = opcode4 & new_n42_;
  assign new_n55_ = ~new_n54_;
  assign new_n56_ = new_n55_ & opcode1;
  assign new_n57_ = ~new_n56_;
  assign new_n58_ = opcode3 & new_n35_;
  assign new_n59_ = new_n58_ & opcode4;
  assign new_n60_ = ~new_n59_;
  assign new_n61_ = new_n60_ & new_n36_;
  assign new_n62_ = ~new_n61_;
  assign new_n63_ = new_n62_ & new_n57_;
  assign new_n64_ = ~new_n63_;
  assign new_n65_ = new_n64_ & new_n53_;
  assign new_n66_ = ~new_n65_;
  assign new_n67_ = new_n46_ & opcode2;
  assign new_n68_ = ~new_n67_;
  assign oseloregdst1 = new_n68_ & new_n66_;
  assign new_n70_ = new_n54_ & new_n35_;
  assign new_n71_ = ~new_n70_;
  assign new_n72_ = new_n71_ & opcode1;
  assign new_n73_ = new_n43_ & opcode3;
  assign new_n74_ = ~new_n73_;
  assign new_n75_ = new_n74_ & new_n72_;
  assign new_n76_ = ~new_n75_;
  assign new_n77_ = new_n62_ & new_n53_;
  assign oselopB0 = new_n77_ & new_n76_;
  assign new_n79_ = new_n34_ & opcode0;
  assign new_n80_ = ~new_n79_;
  assign new_n81_ = ~new_n44_;
  assign new_n82_ = new_n53_ & new_n36_;
  assign new_n83_ = ~new_n82_;
  assign new_n84_ = new_n83_ & opcode3;
  assign new_n85_ = ~new_n84_;
  assign new_n86_ = new_n85_ & new_n81_;
  assign oselopB1 = new_n86_ & new_n80_;
  assign new_n88_ = ~opext1;
  assign new_n89_ = new_n88_ & opcode4;
  assign new_n90_ = ~new_n89_;
  assign new_n91_ = new_n90_ & opcode3;
  assign new_n92_ = ~opext0;
  assign new_n93_ = new_n92_ & opcode4;
  assign new_n94_ = new_n93_ & opext1;
  assign new_n95_ = ~new_n94_;
  assign new_n96_ = new_n95_ & new_n91_;
  assign new_n97_ = new_n96_ & opcode0;
  assign new_n98_ = ~new_n97_;
  assign new_n99_ = new_n59_ & opext0;
  assign new_n100_ = ~new_n99_;
  assign new_n101_ = new_n100_ & new_n98_;
  assign new_n102_ = ~new_n101_;
  assign new_n103_ = new_n102_ & opcode1;
  assign new_n104_ = ~new_n103_;
  assign new_n105_ = new_n104_ & new_n53_;
  assign new_n106_ = ~new_n105_;
  assign new_n107_ = opcode2 & new_n35_;
  assign new_n108_ = ~new_n107_;
  assign new_n109_ = new_n55_ & opcode2;
  assign new_n110_ = ~new_n109_;
  assign new_n111_ = new_n110_ & new_n108_;
  assign oaluop0 = new_n111_ & new_n106_;
  assign new_n113_ = ~new_n91_;
  assign new_n114_ = new_n113_ & new_n53_;
  assign new_n115_ = ~new_n114_;
  assign new_n116_ = new_n110_ & opcode1;
  assign oaluop1 = new_n116_ & new_n115_;
  assign new_n118_ = opcode1 & opcode0;
  assign new_n119_ = ~new_n118_;
  assign new_n120_ = new_n119_ & new_n34_;
  assign new_n121_ = new_n120_ & new_n53_;
  assign new_n122_ = ~new_n121_;
  assign new_n123_ = new_n41_ & opcode2;
  assign new_n124_ = ~new_n123_;
  assign new_n125_ = new_n124_ & new_n81_;
  assign oaluop2 = new_n125_ & new_n122_;
  assign new_n127_ = new_n80_ & new_n46_;
  assign new_n128_ = ~new_n127_;
  assign new_n129_ = new_n128_ & opcode2;
  assign new_n130_ = ~new_n129_;
  assign new_n131_ = new_n82_ & new_n59_;
  assign new_n132_ = ~new_n131_;
  assign new_n133_ = new_n132_ & new_n130_;
  assign oaluopext0 = ~new_n133_;
  assign new_n135_ = new_n72_ & new_n53_;
  assign new_n136_ = ~new_n135_;
  assign new_n137_ = new_n123_ & new_n81_;
  assign new_n138_ = ~new_n137_;
  assign new_n139_ = new_n138_ & opcode1;
  assign oaluopext1 = new_n139_ & new_n136_;
  assign new_n141_ = new_n82_ & new_n41_;
  assign new_n142_ = ~new_n141_;
  assign new_n143_ = new_n142_ & new_n68_;
  assign oaluopext2 = new_n143_ & new_n136_;
  assign new_n145_ = opcode4 & new_n36_;
  assign new_n146_ = ~new_n145_;
  assign new_n147_ = new_n146_ & opcode3;
  assign new_n148_ = ~new_n147_;
  assign new_n149_ = opcode1 & new_n35_;
  assign new_n150_ = ~new_n149_;
  assign new_n151_ = new_n150_ & new_n54_;
  assign new_n152_ = ~new_n151_;
  assign new_n153_ = new_n152_ & new_n53_;
  assign new_n154_ = new_n153_ & new_n148_;
  assign new_n155_ = ~new_n154_;
  assign oaluopext3 = new_n155_ & new_n110_;
  assign new_n157_ = new_n44_ & new_n35_;
  assign ohalt = new_n157_ & new_n82_;
  assign new_n159_ = new_n38_ & opcode4;
  assign new_n160_ = ~new_n159_;
  assign new_n161_ = new_n160_ & new_n42_;
  assign new_n162_ = ~new_n161_;
  assign new_n163_ = new_n162_ & new_n53_;
  assign new_n164_ = ~new_n163_;
  assign new_n165_ = new_n42_ & opcode1;
  assign new_n166_ = ~new_n165_;
  assign new_n167_ = new_n166_ & new_n43_;
  assign new_n168_ = ~new_n167_;
  assign new_n169_ = new_n168_ & opcode2;
  assign new_n170_ = ~new_n169_;
  assign new_n171_ = new_n170_ & new_n164_;
  assign oregwrite = ~new_n171_;
  assign ojump = new_n44_ & opcode2;
  assign oselpcopA = ojump & opcode0;
  assign oselpcopB = new_n107_ & new_n44_;
  assign new_n176_ = new_n73_ & new_n36_;
  assign obeqz = new_n176_ & new_n107_;
  assign new_n178_ = new_n176_ & opcode0;
  assign obnez = new_n178_ & opcode2;
  assign new_n180_ = new_n73_ & opcode2;
  assign obgez = new_n180_ & new_n118_;
  assign obltz = new_n180_ & new_n149_;
  assign new_n183_ = ~new_n120_;
  assign new_n184_ = new_n183_ & opcode2;
  assign new_n185_ = ~new_n184_;
  assign new_n186_ = ~new_n93_;
  assign new_n187_ = new_n186_ & opcode3;
  assign new_n188_ = new_n187_ & new_n118_;
  assign new_n189_ = ~new_n188_;
  assign new_n190_ = ~new_n178_;
  assign new_n191_ = new_n190_ & new_n53_;
  assign new_n192_ = new_n191_ & new_n189_;
  assign new_n193_ = ~new_n192_;
  assign oCin = new_n193_ & new_n185_;
  assign new_n195_ = ~new_n176_;
  assign new_n196_ = opext0 & opcode1;
  assign new_n197_ = new_n196_ & new_n88_;
  assign new_n198_ = new_n197_ & new_n34_;
  assign new_n199_ = ~new_n198_;
  assign new_n200_ = new_n199_ & new_n195_;
  assign new_n201_ = ~new_n200_;
  assign new_n202_ = new_n53_ & opcode0;
  assign oinvA = new_n202_ & new_n201_;
  assign new_n204_ = new_n118_ & new_n96_;
  assign new_n205_ = ~new_n204_;
  assign new_n206_ = new_n205_ & new_n53_;
  assign new_n207_ = ~new_n206_;
  assign oinvB = new_n207_ & new_n185_;
  assign new_n210_ = ~opcode1 ^ ~new_n35_;
  assign new_n211_ = new_n54_ & new_n53_;
  assign omemwrite = new_n211_ & new_n210_;
  assign new_n213_ = new_n54_ & new_n36_;
  assign oselwb = new_n213_ & new_n202_;
  assign osign = 1'b1;
endmodule


