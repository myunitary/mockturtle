// Benchmark "hd04" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    i_2, i_3, i_4, i_5, i_6, i_7, i_8, i_9, i_10, i_11, i_12, i_13, i_14,
    i_15, i_16, i_17,
    m_0, m_1, m_2, m_3, m_4, m_5, m_6, m_7  );
  input  i_2, i_3, i_4, i_5, i_6, i_7, i_8, i_9, i_10, i_11, i_12, i_13,
    i_14, i_15, i_16, i_17;
  output m_0, m_1, m_2, m_3, m_4, m_5, m_6, m_7;
  wire new_n25_, new_n26_, new_n27_, new_n28_, new_n29_, new_n30_, new_n31_,
    new_n32_, new_n33_, new_n34_, new_n35_, new_n36_, new_n37_, new_n38_,
    new_n39_, new_n40_, new_n41_, new_n42_, new_n43_, new_n44_, new_n45_,
    new_n46_, new_n47_, new_n48_, new_n49_, new_n50_, new_n51_, new_n52_,
    new_n53_, new_n54_, new_n55_, new_n56_, new_n57_, new_n58_, new_n59_,
    new_n60_, new_n61_, new_n62_, new_n63_, new_n64_, new_n65_, new_n66_,
    new_n67_, new_n68_, new_n69_, new_n70_, new_n71_, new_n72_, new_n73_,
    new_n74_, new_n75_, new_n76_, new_n77_, new_n78_, new_n79_, new_n80_,
    new_n81_, new_n82_, new_n83_, new_n84_, new_n85_, new_n86_, new_n87_,
    new_n88_, new_n89_, new_n90_, new_n91_, new_n92_, new_n93_, new_n94_,
    new_n95_, new_n96_, new_n97_, new_n98_, new_n99_, new_n100_, new_n101_,
    new_n102_, new_n103_, new_n104_, new_n105_, new_n106_, new_n107_,
    new_n108_, new_n109_, new_n110_, new_n111_, new_n112_, new_n113_,
    new_n114_, new_n115_, new_n116_, new_n117_, new_n118_, new_n119_,
    new_n120_, new_n121_, new_n122_, new_n123_, new_n125_, new_n126_,
    new_n127_, new_n128_, new_n129_, new_n130_, new_n131_, new_n132_,
    new_n133_, new_n134_, new_n135_, new_n136_, new_n137_, new_n138_,
    new_n139_, new_n140_, new_n141_, new_n142_, new_n143_, new_n144_,
    new_n145_, new_n146_, new_n147_, new_n148_, new_n149_, new_n150_,
    new_n151_, new_n153_, new_n154_, new_n155_, new_n156_, new_n157_,
    new_n158_, new_n159_, new_n160_, new_n161_, new_n162_, new_n165_,
    new_n166_, new_n167_, new_n168_, new_n169_, new_n170_, new_n171_,
    new_n172_, new_n174_, new_n175_, new_n176_, new_n177_, new_n178_,
    new_n179_, new_n181_;
  assign new_n25_ = i_10 & i_2;
  assign new_n26_ = i_11 & i_3;
  assign new_n27_ = ~new_n26_;
  assign new_n28_ = new_n27_ & new_n25_;
  assign new_n29_ = ~new_n28_;
  assign new_n30_ = ~i_10;
  assign new_n31_ = new_n26_ & new_n30_;
  assign new_n32_ = ~new_n31_;
  assign new_n33_ = new_n32_ & new_n29_;
  assign new_n34_ = ~new_n33_;
  assign new_n35_ = ~i_16 ^ ~i_8;
  assign new_n36_ = i_17 & i_9;
  assign new_n37_ = new_n36_ & new_n35_;
  assign new_n38_ = ~i_7;
  assign new_n39_ = ~i_15 ^ ~new_n38_;
  assign new_n40_ = ~new_n39_;
  assign new_n41_ = new_n40_ & new_n37_;
  assign new_n42_ = ~i_6;
  assign new_n43_ = ~i_14 ^ ~new_n42_;
  assign new_n44_ = ~new_n43_;
  assign new_n45_ = ~i_13 ^ ~i_5;
  assign new_n46_ = new_n45_ & new_n44_;
  assign new_n47_ = new_n46_ & new_n41_;
  assign new_n48_ = new_n47_ & new_n34_;
  assign new_n49_ = ~new_n48_;
  assign new_n50_ = ~i_3;
  assign new_n51_ = ~i_11 ^ ~new_n50_;
  assign new_n52_ = i_12 & i_4;
  assign new_n53_ = ~new_n52_;
  assign new_n54_ = new_n53_ & new_n51_;
  assign new_n55_ = ~new_n54_;
  assign new_n56_ = new_n55_ & new_n49_;
  assign new_n57_ = ~i_4;
  assign new_n58_ = ~i_12 ^ ~new_n57_;
  assign new_n59_ = i_13 & i_5;
  assign new_n60_ = ~new_n59_;
  assign new_n61_ = i_14 & i_6;
  assign new_n62_ = ~new_n61_;
  assign new_n63_ = new_n62_ & new_n60_;
  assign new_n64_ = i_15 & i_7;
  assign new_n65_ = ~new_n64_;
  assign new_n66_ = i_16 & i_8;
  assign new_n67_ = ~new_n66_;
  assign new_n68_ = new_n67_ & new_n65_;
  assign new_n69_ = ~new_n68_;
  assign new_n70_ = ~i_15;
  assign new_n71_ = new_n70_ & new_n38_;
  assign new_n72_ = ~new_n71_;
  assign new_n73_ = ~i_14;
  assign new_n74_ = new_n73_ & new_n42_;
  assign new_n75_ = ~new_n74_;
  assign new_n76_ = new_n75_ & new_n72_;
  assign new_n77_ = new_n76_ & new_n69_;
  assign new_n78_ = ~new_n77_;
  assign new_n79_ = new_n78_ & new_n63_;
  assign new_n80_ = ~new_n79_;
  assign new_n81_ = ~i_5;
  assign new_n82_ = ~i_13;
  assign new_n83_ = new_n82_ & new_n81_;
  assign new_n84_ = ~new_n83_;
  assign new_n85_ = new_n84_ & new_n80_;
  assign new_n86_ = ~new_n85_;
  assign new_n87_ = new_n86_ & new_n58_;
  assign new_n88_ = ~new_n87_;
  assign new_n89_ = ~new_n47_;
  assign new_n90_ = new_n89_ & new_n32_;
  assign new_n91_ = ~new_n90_;
  assign new_n92_ = new_n91_ & new_n88_;
  assign new_n93_ = new_n92_ & new_n56_;
  assign new_n94_ = ~new_n93_;
  assign new_n95_ = ~i_12;
  assign new_n96_ = new_n95_ & new_n57_;
  assign new_n97_ = ~new_n96_;
  assign new_n98_ = new_n97_ & new_n85_;
  assign new_n99_ = ~new_n98_;
  assign new_n100_ = ~i_2;
  assign new_n101_ = new_n30_ & new_n100_;
  assign new_n102_ = ~new_n101_;
  assign new_n103_ = new_n53_ & new_n27_;
  assign new_n104_ = new_n103_ & new_n102_;
  assign new_n105_ = new_n104_ & new_n99_;
  assign new_n106_ = ~new_n105_;
  assign new_n107_ = new_n84_ & i_4;
  assign new_n108_ = new_n107_ & new_n80_;
  assign new_n109_ = i_12 & new_n100_;
  assign new_n110_ = new_n109_ & new_n31_;
  assign new_n111_ = new_n110_ & new_n108_;
  assign new_n112_ = ~new_n111_;
  assign new_n113_ = ~new_n25_;
  assign new_n114_ = ~i_11;
  assign new_n115_ = new_n114_ & new_n50_;
  assign new_n116_ = ~new_n115_;
  assign new_n117_ = new_n116_ & new_n113_;
  assign new_n118_ = ~new_n117_;
  assign new_n119_ = new_n118_ & new_n102_;
  assign new_n120_ = ~new_n119_;
  assign new_n121_ = new_n120_ & new_n112_;
  assign new_n122_ = new_n121_ & new_n106_;
  assign new_n123_ = ~new_n122_;
  assign m_0 = new_n123_ & new_n94_;
  assign new_n125_ = ~new_n51_;
  assign new_n126_ = ~new_n108_ ^ ~new_n125_;
  assign new_n127_ = new_n84_ & new_n58_;
  assign new_n128_ = new_n127_ & new_n80_;
  assign new_n129_ = ~new_n128_;
  assign new_n130_ = ~new_n58_;
  assign new_n131_ = new_n63_ & new_n130_;
  assign new_n132_ = new_n131_ & new_n78_;
  assign new_n133_ = ~new_n132_;
  assign new_n134_ = new_n83_ & new_n130_;
  assign new_n135_ = ~new_n134_;
  assign new_n136_ = new_n135_ & new_n133_;
  assign new_n137_ = new_n136_ & new_n129_;
  assign new_n138_ = ~new_n137_;
  assign new_n139_ = new_n138_ & new_n126_;
  assign new_n140_ = ~i_10 ^ ~new_n100_;
  assign new_n141_ = ~new_n140_ ^ ~new_n27_;
  assign new_n142_ = ~new_n141_ ^ ~new_n89_;
  assign new_n143_ = new_n142_ & new_n139_;
  assign new_n144_ = ~new_n143_;
  assign new_n145_ = ~new_n139_;
  assign new_n146_ = new_n96_ & new_n125_;
  assign new_n147_ = ~new_n146_;
  assign new_n148_ = new_n147_ & new_n116_;
  assign new_n149_ = ~new_n148_ ^ ~new_n140_;
  assign new_n150_ = new_n149_ & new_n145_;
  assign new_n151_ = ~new_n150_;
  assign m_1 = new_n151_ & new_n144_;
  assign new_n153_ = new_n139_ & new_n47_;
  assign new_n154_ = ~new_n153_;
  assign new_n155_ = ~new_n96_ ^ ~new_n125_;
  assign new_n156_ = new_n155_ & new_n137_;
  assign new_n157_ = ~new_n156_;
  assign new_n158_ = ~new_n126_;
  assign new_n159_ = new_n138_ & new_n89_;
  assign new_n160_ = new_n159_ & new_n158_;
  assign new_n161_ = ~new_n160_;
  assign new_n162_ = new_n161_ & new_n157_;
  assign m_2 = new_n162_ & new_n154_;
  assign m_3 = ~new_n137_ ^ ~new_n89_;
  assign new_n165_ = ~new_n37_;
  assign new_n166_ = new_n68_ & new_n165_;
  assign new_n167_ = ~new_n166_;
  assign new_n168_ = new_n167_ & new_n72_;
  assign new_n169_ = ~new_n168_;
  assign new_n170_ = new_n169_ & new_n62_;
  assign new_n171_ = ~new_n170_;
  assign new_n172_ = new_n171_ & new_n75_;
  assign m_4 = ~new_n172_ ^ ~new_n45_;
  assign new_n174_ = ~new_n41_;
  assign new_n175_ = new_n67_ & new_n174_;
  assign new_n176_ = ~new_n175_;
  assign new_n177_ = new_n176_ & new_n40_;
  assign new_n178_ = ~new_n177_;
  assign new_n179_ = new_n178_ & new_n65_;
  assign m_5 = ~new_n179_ ^ ~new_n43_;
  assign new_n181_ = new_n67_ & new_n165_;
  assign m_6 = ~new_n181_ ^ ~new_n39_;
  assign m_7 = ~new_n36_ ^ ~new_n35_;
endmodule


