// Benchmark "dec" written by ABC on Wed Dec 27 23:19:07 2023

module top ( 
    count0, count1, count2, count3, count4, count5, count6, count7,
    osel10, osel11, osel12, osel13, osel14, osel15, osel16, osel17, osel18,
    osel19, osel110, osel111, osel112, osel113, osel114, osel115, osel116,
    osel117, osel118, osel119, osel120, osel121, osel122, osel123, osel124,
    osel125, osel126, osel127, osel128, osel129, osel130, osel131, osel132,
    osel133, osel134, osel135, osel136, osel137, osel138, osel139, osel140,
    osel141, osel142, osel143, osel144, osel145, osel146, osel147, osel148,
    osel149, osel150, osel151, osel152, osel153, osel154, osel155, osel156,
    osel157, osel158, osel159, osel160, osel161, osel162, osel163, osel164,
    osel165, osel166, osel167, osel168, osel169, osel170, osel171, osel172,
    osel173, osel174, osel175, osel176, osel177, osel178, osel179, osel180,
    osel181, osel182, osel183, osel184, osel185, osel186, osel187, osel188,
    osel189, osel190, osel191, osel192, osel193, osel194, osel195, osel196,
    osel197, osel198, osel199, osel1100, osel1101, osel1102, osel1103,
    osel1104, osel1105, osel1106, osel1107, osel1108, osel1109, osel1110,
    osel1111, osel1112, osel1113, osel1114, osel1115, osel1116, osel1117,
    osel1118, osel1119, osel1120, osel1121, osel1122, osel1123, osel1124,
    osel1125, osel1126, osel1127, osel20, osel21, osel22, osel23, osel24,
    osel25, osel26, osel27, osel28, osel29, osel210, osel211, osel212,
    osel213, osel214, osel215, osel216, osel217, osel218, osel219, osel220,
    osel221, osel222, osel223, osel224, osel225, osel226, osel227, osel228,
    osel229, osel230, osel231, osel232, osel233, osel234, osel235, osel236,
    osel237, osel238, osel239, osel240, osel241, osel242, osel243, osel244,
    osel245, osel246, osel247, osel248, osel249, osel250, osel251, osel252,
    osel253, osel254, osel255, osel256, osel257, osel258, osel259, osel260,
    osel261, osel262, osel263, osel264, osel265, osel266, osel267, osel268,
    osel269, osel270, osel271, osel272, osel273, osel274, osel275, osel276,
    osel277, osel278, osel279, osel280, osel281, osel282, osel283, osel284,
    osel285, osel286, osel287, osel288, osel289, osel290, osel291, osel292,
    osel293, osel294, osel295, osel296, osel297, osel298, osel299,
    osel2100, osel2101, osel2102, osel2103, osel2104, osel2105, osel2106,
    osel2107, osel2108, osel2109, osel2110, osel2111, osel2112, osel2113,
    osel2114, osel2115, osel2116, osel2117, osel2118, osel2119, osel2120,
    osel2121, osel2122, osel2123, osel2124, osel2125, osel2126, osel2127  );
  input  count0, count1, count2, count3, count4, count5, count6, count7;
  output osel10, osel11, osel12, osel13, osel14, osel15, osel16, osel17,
    osel18, osel19, osel110, osel111, osel112, osel113, osel114, osel115,
    osel116, osel117, osel118, osel119, osel120, osel121, osel122, osel123,
    osel124, osel125, osel126, osel127, osel128, osel129, osel130, osel131,
    osel132, osel133, osel134, osel135, osel136, osel137, osel138, osel139,
    osel140, osel141, osel142, osel143, osel144, osel145, osel146, osel147,
    osel148, osel149, osel150, osel151, osel152, osel153, osel154, osel155,
    osel156, osel157, osel158, osel159, osel160, osel161, osel162, osel163,
    osel164, osel165, osel166, osel167, osel168, osel169, osel170, osel171,
    osel172, osel173, osel174, osel175, osel176, osel177, osel178, osel179,
    osel180, osel181, osel182, osel183, osel184, osel185, osel186, osel187,
    osel188, osel189, osel190, osel191, osel192, osel193, osel194, osel195,
    osel196, osel197, osel198, osel199, osel1100, osel1101, osel1102,
    osel1103, osel1104, osel1105, osel1106, osel1107, osel1108, osel1109,
    osel1110, osel1111, osel1112, osel1113, osel1114, osel1115, osel1116,
    osel1117, osel1118, osel1119, osel1120, osel1121, osel1122, osel1123,
    osel1124, osel1125, osel1126, osel1127, osel20, osel21, osel22, osel23,
    osel24, osel25, osel26, osel27, osel28, osel29, osel210, osel211,
    osel212, osel213, osel214, osel215, osel216, osel217, osel218, osel219,
    osel220, osel221, osel222, osel223, osel224, osel225, osel226, osel227,
    osel228, osel229, osel230, osel231, osel232, osel233, osel234, osel235,
    osel236, osel237, osel238, osel239, osel240, osel241, osel242, osel243,
    osel244, osel245, osel246, osel247, osel248, osel249, osel250, osel251,
    osel252, osel253, osel254, osel255, osel256, osel257, osel258, osel259,
    osel260, osel261, osel262, osel263, osel264, osel265, osel266, osel267,
    osel268, osel269, osel270, osel271, osel272, osel273, osel274, osel275,
    osel276, osel277, osel278, osel279, osel280, osel281, osel282, osel283,
    osel284, osel285, osel286, osel287, osel288, osel289, osel290, osel291,
    osel292, osel293, osel294, osel295, osel296, osel297, osel298, osel299,
    osel2100, osel2101, osel2102, osel2103, osel2104, osel2105, osel2106,
    osel2107, osel2108, osel2109, osel2110, osel2111, osel2112, osel2113,
    osel2114, osel2115, osel2116, osel2117, osel2118, osel2119, osel2120,
    osel2121, osel2122, osel2123, osel2124, osel2125, osel2126, osel2127;
  wire new_n265_, new_n266_, new_n267_, new_n268_, new_n269_, new_n270_,
    new_n271_, new_n272_, new_n273_, new_n274_, new_n275_, new_n276_,
    new_n277_, new_n279_, new_n280_, new_n282_, new_n283_, new_n285_,
    new_n287_, new_n288_, new_n290_, new_n291_, new_n293_, new_n295_,
    new_n297_, new_n298_, new_n300_, new_n302_, new_n303_, new_n305_,
    new_n307_, new_n309_, new_n311_, new_n313_, new_n315_, new_n316_,
    new_n333_, new_n334_, new_n351_, new_n352_, new_n369_, new_n370_,
    new_n387_, new_n404_, new_n421_, new_n438_, new_n439_, new_n440_,
    new_n457_, new_n474_, new_n491_, new_n508_, new_n509_, new_n526_,
    new_n543_, new_n560_;
  assign new_n265_ = ~count4;
  assign new_n266_ = ~count5;
  assign new_n267_ = new_n266_ & new_n265_;
  assign new_n268_ = ~count6;
  assign new_n269_ = count7 & new_n268_;
  assign new_n270_ = new_n269_ & new_n267_;
  assign new_n271_ = ~count0;
  assign new_n272_ = ~count2;
  assign new_n273_ = new_n272_ & new_n271_;
  assign new_n274_ = ~count1;
  assign new_n275_ = ~count3;
  assign new_n276_ = new_n275_ & new_n274_;
  assign new_n277_ = new_n276_ & new_n273_;
  assign osel10 = new_n277_ & new_n270_;
  assign new_n279_ = new_n272_ & count0;
  assign new_n280_ = new_n279_ & new_n276_;
  assign osel11 = new_n280_ & new_n270_;
  assign new_n282_ = new_n275_ & count1;
  assign new_n283_ = new_n282_ & new_n273_;
  assign osel12 = new_n283_ & new_n270_;
  assign new_n285_ = new_n282_ & new_n279_;
  assign osel13 = new_n285_ & new_n270_;
  assign new_n287_ = count2 & new_n271_;
  assign new_n288_ = new_n287_ & new_n276_;
  assign osel14 = new_n288_ & new_n270_;
  assign new_n290_ = count2 & count0;
  assign new_n291_ = new_n290_ & new_n276_;
  assign osel15 = new_n291_ & new_n270_;
  assign new_n293_ = new_n287_ & new_n282_;
  assign osel16 = new_n293_ & new_n270_;
  assign new_n295_ = new_n290_ & new_n282_;
  assign osel17 = new_n295_ & new_n270_;
  assign new_n297_ = count3 & new_n274_;
  assign new_n298_ = new_n297_ & new_n273_;
  assign osel18 = new_n298_ & new_n270_;
  assign new_n300_ = new_n297_ & new_n279_;
  assign osel19 = new_n300_ & new_n270_;
  assign new_n302_ = count3 & count1;
  assign new_n303_ = new_n302_ & new_n273_;
  assign osel110 = new_n303_ & new_n270_;
  assign new_n305_ = new_n302_ & new_n279_;
  assign osel111 = new_n305_ & new_n270_;
  assign new_n307_ = new_n297_ & new_n287_;
  assign osel112 = new_n307_ & new_n270_;
  assign new_n309_ = new_n297_ & new_n290_;
  assign osel113 = new_n309_ & new_n270_;
  assign new_n311_ = new_n302_ & new_n287_;
  assign osel114 = new_n311_ & new_n270_;
  assign new_n313_ = new_n302_ & new_n290_;
  assign osel115 = new_n313_ & new_n270_;
  assign new_n315_ = new_n266_ & count4;
  assign new_n316_ = new_n315_ & new_n269_;
  assign osel116 = new_n316_ & new_n277_;
  assign osel117 = new_n316_ & new_n280_;
  assign osel118 = new_n316_ & new_n283_;
  assign osel119 = new_n316_ & new_n285_;
  assign osel120 = new_n316_ & new_n288_;
  assign osel121 = new_n316_ & new_n291_;
  assign osel122 = new_n316_ & new_n293_;
  assign osel123 = new_n316_ & new_n295_;
  assign osel124 = new_n316_ & new_n298_;
  assign osel125 = new_n316_ & new_n300_;
  assign osel126 = new_n316_ & new_n303_;
  assign osel127 = new_n316_ & new_n305_;
  assign osel128 = new_n316_ & new_n307_;
  assign osel129 = new_n316_ & new_n309_;
  assign osel130 = new_n316_ & new_n311_;
  assign osel131 = new_n316_ & new_n313_;
  assign new_n333_ = count5 & new_n265_;
  assign new_n334_ = new_n333_ & new_n269_;
  assign osel132 = new_n334_ & new_n277_;
  assign osel133 = new_n334_ & new_n280_;
  assign osel134 = new_n334_ & new_n283_;
  assign osel135 = new_n334_ & new_n285_;
  assign osel136 = new_n334_ & new_n288_;
  assign osel137 = new_n334_ & new_n291_;
  assign osel138 = new_n334_ & new_n293_;
  assign osel139 = new_n334_ & new_n295_;
  assign osel140 = new_n334_ & new_n298_;
  assign osel141 = new_n334_ & new_n300_;
  assign osel142 = new_n334_ & new_n303_;
  assign osel143 = new_n334_ & new_n305_;
  assign osel144 = new_n334_ & new_n307_;
  assign osel145 = new_n334_ & new_n309_;
  assign osel146 = new_n334_ & new_n311_;
  assign osel147 = new_n334_ & new_n313_;
  assign new_n351_ = count5 & count4;
  assign new_n352_ = new_n351_ & new_n269_;
  assign osel148 = new_n352_ & new_n277_;
  assign osel149 = new_n352_ & new_n280_;
  assign osel150 = new_n352_ & new_n283_;
  assign osel151 = new_n352_ & new_n285_;
  assign osel152 = new_n352_ & new_n288_;
  assign osel153 = new_n352_ & new_n291_;
  assign osel154 = new_n352_ & new_n293_;
  assign osel155 = new_n352_ & new_n295_;
  assign osel156 = new_n352_ & new_n298_;
  assign osel157 = new_n352_ & new_n300_;
  assign osel158 = new_n352_ & new_n303_;
  assign osel159 = new_n352_ & new_n305_;
  assign osel160 = new_n352_ & new_n307_;
  assign osel161 = new_n352_ & new_n309_;
  assign osel162 = new_n352_ & new_n311_;
  assign osel163 = new_n352_ & new_n313_;
  assign new_n369_ = count7 & count6;
  assign new_n370_ = new_n369_ & new_n267_;
  assign osel164 = new_n370_ & new_n277_;
  assign osel165 = new_n370_ & new_n280_;
  assign osel166 = new_n370_ & new_n283_;
  assign osel167 = new_n370_ & new_n285_;
  assign osel168 = new_n370_ & new_n288_;
  assign osel169 = new_n370_ & new_n291_;
  assign osel170 = new_n370_ & new_n293_;
  assign osel171 = new_n370_ & new_n295_;
  assign osel172 = new_n370_ & new_n298_;
  assign osel173 = new_n370_ & new_n300_;
  assign osel174 = new_n370_ & new_n303_;
  assign osel175 = new_n370_ & new_n305_;
  assign osel176 = new_n370_ & new_n307_;
  assign osel177 = new_n370_ & new_n309_;
  assign osel178 = new_n370_ & new_n311_;
  assign osel179 = new_n370_ & new_n313_;
  assign new_n387_ = new_n369_ & new_n315_;
  assign osel180 = new_n387_ & new_n277_;
  assign osel181 = new_n387_ & new_n280_;
  assign osel182 = new_n387_ & new_n283_;
  assign osel183 = new_n387_ & new_n285_;
  assign osel184 = new_n387_ & new_n288_;
  assign osel185 = new_n387_ & new_n291_;
  assign osel186 = new_n387_ & new_n293_;
  assign osel187 = new_n387_ & new_n295_;
  assign osel188 = new_n387_ & new_n298_;
  assign osel189 = new_n387_ & new_n300_;
  assign osel190 = new_n387_ & new_n303_;
  assign osel191 = new_n387_ & new_n305_;
  assign osel192 = new_n387_ & new_n307_;
  assign osel193 = new_n387_ & new_n309_;
  assign osel194 = new_n387_ & new_n311_;
  assign osel195 = new_n387_ & new_n313_;
  assign new_n404_ = new_n369_ & new_n333_;
  assign osel196 = new_n404_ & new_n277_;
  assign osel197 = new_n404_ & new_n280_;
  assign osel198 = new_n404_ & new_n283_;
  assign osel199 = new_n404_ & new_n285_;
  assign osel1100 = new_n404_ & new_n288_;
  assign osel1101 = new_n404_ & new_n291_;
  assign osel1102 = new_n404_ & new_n293_;
  assign osel1103 = new_n404_ & new_n295_;
  assign osel1104 = new_n404_ & new_n298_;
  assign osel1105 = new_n404_ & new_n300_;
  assign osel1106 = new_n404_ & new_n303_;
  assign osel1107 = new_n404_ & new_n305_;
  assign osel1108 = new_n404_ & new_n307_;
  assign osel1109 = new_n404_ & new_n309_;
  assign osel1110 = new_n404_ & new_n311_;
  assign osel1111 = new_n404_ & new_n313_;
  assign new_n421_ = new_n369_ & new_n351_;
  assign osel1112 = new_n421_ & new_n277_;
  assign osel1113 = new_n421_ & new_n280_;
  assign osel1114 = new_n421_ & new_n283_;
  assign osel1115 = new_n421_ & new_n285_;
  assign osel1116 = new_n421_ & new_n288_;
  assign osel1117 = new_n421_ & new_n291_;
  assign osel1118 = new_n421_ & new_n293_;
  assign osel1119 = new_n421_ & new_n295_;
  assign osel1120 = new_n421_ & new_n298_;
  assign osel1121 = new_n421_ & new_n300_;
  assign osel1122 = new_n421_ & new_n303_;
  assign osel1123 = new_n421_ & new_n305_;
  assign osel1124 = new_n421_ & new_n307_;
  assign osel1125 = new_n421_ & new_n309_;
  assign osel1126 = new_n421_ & new_n311_;
  assign osel1127 = new_n421_ & new_n313_;
  assign new_n438_ = ~count7;
  assign new_n439_ = new_n438_ & new_n268_;
  assign new_n440_ = new_n439_ & new_n267_;
  assign osel20 = new_n440_ & new_n277_;
  assign osel21 = new_n440_ & new_n280_;
  assign osel22 = new_n440_ & new_n283_;
  assign osel23 = new_n440_ & new_n285_;
  assign osel24 = new_n440_ & new_n288_;
  assign osel25 = new_n440_ & new_n291_;
  assign osel26 = new_n440_ & new_n293_;
  assign osel27 = new_n440_ & new_n295_;
  assign osel28 = new_n440_ & new_n298_;
  assign osel29 = new_n440_ & new_n300_;
  assign osel210 = new_n440_ & new_n303_;
  assign osel211 = new_n440_ & new_n305_;
  assign osel212 = new_n440_ & new_n307_;
  assign osel213 = new_n440_ & new_n309_;
  assign osel214 = new_n440_ & new_n311_;
  assign osel215 = new_n440_ & new_n313_;
  assign new_n457_ = new_n439_ & new_n315_;
  assign osel216 = new_n457_ & new_n277_;
  assign osel217 = new_n457_ & new_n280_;
  assign osel218 = new_n457_ & new_n283_;
  assign osel219 = new_n457_ & new_n285_;
  assign osel220 = new_n457_ & new_n288_;
  assign osel221 = new_n457_ & new_n291_;
  assign osel222 = new_n457_ & new_n293_;
  assign osel223 = new_n457_ & new_n295_;
  assign osel224 = new_n457_ & new_n298_;
  assign osel225 = new_n457_ & new_n300_;
  assign osel226 = new_n457_ & new_n303_;
  assign osel227 = new_n457_ & new_n305_;
  assign osel228 = new_n457_ & new_n307_;
  assign osel229 = new_n457_ & new_n309_;
  assign osel230 = new_n457_ & new_n311_;
  assign osel231 = new_n457_ & new_n313_;
  assign new_n474_ = new_n439_ & new_n333_;
  assign osel232 = new_n474_ & new_n277_;
  assign osel233 = new_n474_ & new_n280_;
  assign osel234 = new_n474_ & new_n283_;
  assign osel235 = new_n474_ & new_n285_;
  assign osel236 = new_n474_ & new_n288_;
  assign osel237 = new_n474_ & new_n291_;
  assign osel238 = new_n474_ & new_n293_;
  assign osel239 = new_n474_ & new_n295_;
  assign osel240 = new_n474_ & new_n298_;
  assign osel241 = new_n474_ & new_n300_;
  assign osel242 = new_n474_ & new_n303_;
  assign osel243 = new_n474_ & new_n305_;
  assign osel244 = new_n474_ & new_n307_;
  assign osel245 = new_n474_ & new_n309_;
  assign osel246 = new_n474_ & new_n311_;
  assign osel247 = new_n474_ & new_n313_;
  assign new_n491_ = new_n439_ & new_n351_;
  assign osel248 = new_n491_ & new_n277_;
  assign osel249 = new_n491_ & new_n280_;
  assign osel250 = new_n491_ & new_n283_;
  assign osel251 = new_n491_ & new_n285_;
  assign osel252 = new_n491_ & new_n288_;
  assign osel253 = new_n491_ & new_n291_;
  assign osel254 = new_n491_ & new_n293_;
  assign osel255 = new_n491_ & new_n295_;
  assign osel256 = new_n491_ & new_n298_;
  assign osel257 = new_n491_ & new_n300_;
  assign osel258 = new_n491_ & new_n303_;
  assign osel259 = new_n491_ & new_n305_;
  assign osel260 = new_n491_ & new_n307_;
  assign osel261 = new_n491_ & new_n309_;
  assign osel262 = new_n491_ & new_n311_;
  assign osel263 = new_n491_ & new_n313_;
  assign new_n508_ = new_n438_ & count6;
  assign new_n509_ = new_n508_ & new_n267_;
  assign osel264 = new_n509_ & new_n277_;
  assign osel265 = new_n509_ & new_n280_;
  assign osel266 = new_n509_ & new_n283_;
  assign osel267 = new_n509_ & new_n285_;
  assign osel268 = new_n509_ & new_n288_;
  assign osel269 = new_n509_ & new_n291_;
  assign osel270 = new_n509_ & new_n293_;
  assign osel271 = new_n509_ & new_n295_;
  assign osel272 = new_n509_ & new_n298_;
  assign osel273 = new_n509_ & new_n300_;
  assign osel274 = new_n509_ & new_n303_;
  assign osel275 = new_n509_ & new_n305_;
  assign osel276 = new_n509_ & new_n307_;
  assign osel277 = new_n509_ & new_n309_;
  assign osel278 = new_n509_ & new_n311_;
  assign osel279 = new_n509_ & new_n313_;
  assign new_n526_ = new_n508_ & new_n315_;
  assign osel280 = new_n526_ & new_n277_;
  assign osel281 = new_n526_ & new_n280_;
  assign osel282 = new_n526_ & new_n283_;
  assign osel283 = new_n526_ & new_n285_;
  assign osel284 = new_n526_ & new_n288_;
  assign osel285 = new_n526_ & new_n291_;
  assign osel286 = new_n526_ & new_n293_;
  assign osel287 = new_n526_ & new_n295_;
  assign osel288 = new_n526_ & new_n298_;
  assign osel289 = new_n526_ & new_n300_;
  assign osel290 = new_n526_ & new_n303_;
  assign osel291 = new_n526_ & new_n305_;
  assign osel292 = new_n526_ & new_n307_;
  assign osel293 = new_n526_ & new_n309_;
  assign osel294 = new_n526_ & new_n311_;
  assign osel295 = new_n526_ & new_n313_;
  assign new_n543_ = new_n508_ & new_n333_;
  assign osel296 = new_n543_ & new_n277_;
  assign osel297 = new_n543_ & new_n280_;
  assign osel298 = new_n543_ & new_n283_;
  assign osel299 = new_n543_ & new_n285_;
  assign osel2100 = new_n543_ & new_n288_;
  assign osel2101 = new_n543_ & new_n291_;
  assign osel2102 = new_n543_ & new_n293_;
  assign osel2103 = new_n543_ & new_n295_;
  assign osel2104 = new_n543_ & new_n298_;
  assign osel2105 = new_n543_ & new_n300_;
  assign osel2106 = new_n543_ & new_n303_;
  assign osel2107 = new_n543_ & new_n305_;
  assign osel2108 = new_n543_ & new_n307_;
  assign osel2109 = new_n543_ & new_n309_;
  assign osel2110 = new_n543_ & new_n311_;
  assign osel2111 = new_n543_ & new_n313_;
  assign new_n560_ = new_n508_ & new_n351_;
  assign osel2112 = new_n560_ & new_n277_;
  assign osel2113 = new_n560_ & new_n280_;
  assign osel2114 = new_n560_ & new_n283_;
  assign osel2115 = new_n560_ & new_n285_;
  assign osel2116 = new_n560_ & new_n288_;
  assign osel2117 = new_n560_ & new_n291_;
  assign osel2118 = new_n560_ & new_n293_;
  assign osel2119 = new_n560_ & new_n295_;
  assign osel2120 = new_n560_ & new_n298_;
  assign osel2121 = new_n560_ & new_n300_;
  assign osel2122 = new_n560_ & new_n303_;
  assign osel2123 = new_n560_ & new_n305_;
  assign osel2124 = new_n560_ & new_n307_;
  assign osel2125 = new_n560_ & new_n309_;
  assign osel2126 = new_n560_ & new_n311_;
  assign osel2127 = new_n560_ & new_n313_;
endmodule


