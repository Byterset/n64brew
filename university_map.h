
#ifndef UNIVERSITY_MAP_H
#define UNIVERSITY_MAP_H 1
#include "constants.h"
#include "gameobject.h"


AABB university_map_bounds[] = {
{{-45.071983, -12.775320, -47.049881}, {48.456688, 53.892517, 47.044338}}, // (Bush.inst) 
{{-45.071983, -12.775320, -47.049881}, {48.456745, 53.892513, 47.044338}}, // (Bush.inst.001) 
{{-45.071983, -12.775320, -47.049881}, {48.456745, 53.892517, 47.044338}}, // (Bush.inst.002) 
{{-16.400528, 0.000000, -44.529362}, {20.007133, 159.193802, 44.529362}}, // (GardenerCharacter) 
{{-236.518585, -54.389256, -15.605621}, {236.519394, 18.310425, 15.605621}}, // (Wall.inst) 
{{-236.518585, -54.389256, -15.605621}, {236.519394, 18.310425, 15.605621}}, // (Wall.inst.002) 
{{-75.067978, -32.971409, -74.530678}, {75.116386, 31.478992, 74.758820}}, // (Planter.inst) 
{{-75.067978, -32.971409, -74.530678}, {75.116386, 31.478992, 74.758820}}, // (Planter.inst.001) 
{{-45.071869, -12.775320, -47.049866}, {48.456688, 53.892517, 47.044342}}, // (Bush.inst.007) 
{{-32.452812, 0.000000, -15.666389}, {28.539619, 69.391663, 15.363922}}, // (Goose) 
{{-6.503963, -2.904237, -12.591991}, {10.518951, 2.923790, 12.591991}}, // (BookItem) 
{{-275.618408, -2.655000, -1016.100769}, {275.507019, 0.885000, 1016.099060}}, // (Ground.001) 
{{-551.125366, 0.000000, -551.125488}, {551.125366, 0.000000, 551.125488}}, // (Ground.002) 
{{-755.245422, -0.419021, -551.125488}, {755.245483, 1.257062, 551.125488}}, // (Ground.003) 
{{-421.130676, -16.538755, -291.649231}, {413.913971, 4.724967, 277.476379}}, // (Ground.004) 
{{-657.454834, -32.263943, -297.538025}, {580.171753, 26.695923, 281.871521}}, // (Ground.005) 
{{-384.693787, -121.215118, -827.152649}, {458.608582, 46.774334, 654.430298}}, // (Ground.006) 
{{-658.548889, -62.432297, -792.997742}, {586.079529, 87.568802, 686.565063}}, // (Ground) 
{{0.000000, 0.000000, -943.522095}, {615.350525, 0.000000, 0.000000}}, // (Water) 
{{-29.933395, -13.390024, -25.482903}, {30.003204, 5.335236, 31.683998}}, // (Rock.inst) 
{{-29.933395, -13.390024, -25.482903}, {30.003204, 6.759514, 31.683998}}, // (Rock.001.inst) 
{{-19.174805, -13.390017, -25.482903}, {30.003204, 13.261921, 30.914383}}, // (Rock.002.inst) 
{{-15.867348, -21.999542, -5.716667}, {3.573647, 53.502243, 10.156860}}, // (Watergrass.002.inst) 
{{-1.107788, -9.607552, -10.389175}, {13.390160, 49.918880, 0.000000}}, // (Watergrass.003.inst) 
{{-29.933395, -13.390017, -25.482903}, {30.003204, 5.335236, 31.683998}}, // (Rock.inst.001) 
{{0.000000, -66.621208, 0.000000}, {2.139816, 49.910568, 1.821327}}, // (Reed.001.inst.001) 
{{-29.933395, -13.390024, -25.482903}, {30.003204, 6.759514, 31.683998}}, // (Rock.001.inst.001) 
{{0.000000, -8.391581, -7.322273}, {18.022156, 63.362251, 0.000000}}, // (Watergrass.001.inst.001) 
{{-19.668732, -34.420197, -5.539627}, {6.624069, 56.590496, 9.426155}}, // (Watergrass.inst.001) 
{{0.000000, -66.621223, 0.000000}, {2.139816, 49.910568, 1.821327}}, // (Reed.001.inst.002) 
{{-11.701584, 0.000000, -12.729034}, {15.457535, 0.000000, 15.827866}}, // (Lilypad.inst) 
{{-14.035149, 0.000000, -16.148987}, {14.504128, 0.000000, 11.302414}}, // (Lilypad.inst.001) 
{{-13.432961, 0.000000, -11.844177}, {14.300766, 0.000000, 16.635590}}, // (Lilypad.inst.002) 
{{0.000000, -8.391581, -7.322273}, {18.022156, 63.362251, 0.000000}}, // (Watergrass.001.inst.002) 
{{-15.867348, -21.999542, -5.716667}, {3.573647, 53.502243, 10.156860}}, // (Watergrass.002.inst.001) 
{{-1.107788, -9.607558, -10.389175}, {13.390160, 49.918880, 0.000000}}, // (Watergrass.003.inst.001) 
{{-19.668732, -34.420197, -5.539627}, {6.624069, 56.590496, 9.426155}}, // (Watergrass.inst.002) 
{{-905.231201, 0.000000, -381.856384}, {0.000000, 0.000000, 252.461243}}, // (Water.001) 
{{-562.398254, -1.747820, -448.869568}, {0.000000, 0.000000, 251.303680}}, // (Water.002) 
{{-112.304764, -3.202901, -520.496094}, {409.274353, 0.000000, 170.604523}}, // (Water.003) 
{{0.000000, 0.000000, -520.496094}, {649.044434, 0.000000, 60.936470}}, // (Water.004) 

};

GameObject university_map_data[] = {
{0, // object id (Bush.inst)
{-1107.730835, 11.748462, -64.801270}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{1, // object id (Bush.inst.001)
{-994.538513, 11.748466, -18.517456}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{2, // object id (Bush.inst.002)
{-1005.235413, 11.748462, -147.547226}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{3, // object id (GardenerCharacter)
{-1042.592285, 0.000000, -553.012817}, // position
{0.000000, 0.000000, -0.000000}, // rotation
GardenerCharacterModel, // modelType
0, // subtype
},
{4, // object id (Wall.inst)
{-940.044006, 49.424400, -691.284058}, // position
{0.000000, 0.000000, -0.000000}, // rotation
WallModel, // modelType
0, // subtype
},
{5, // object id (Wall.inst.002)
{-1533.462036, 49.424400, -691.284058}, // position
{0.000000, 0.000000, -0.000000}, // rotation
WallModel, // modelType
0, // subtype
},
{6, // object id (Planter.inst)
{-1390.277832, 28.706844, -404.714294}, // position
{0.000000, 0.000000, -0.000000}, // rotation
PlanterModel, // modelType
0, // subtype
},
{7, // object id (Planter.inst.001)
{-1390.277832, 28.706844, -127.979004}, // position
{0.000000, 0.000000, -0.000000}, // rotation
PlanterModel, // modelType
0, // subtype
},
{8, // object id (Bush.inst.007)
{-1931.657593, 11.748433, -464.240753}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{9, // object id (Goose)
{-1892.656250, -62.705933, -1714.741943}, // position
{0.000000, 0.000000, -0.000000}, // rotation
GooseModel, // modelType
0, // subtype
},
{10, // object id (BookItem)
{-900.784485, 0.000007, -241.499634}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BookItemModel, // modelType
0, // subtype
},
{11, // object id (Ground.001)
{-194.526672, -0.944913, -1929.590576}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
0, // subtype
},
{12, // object id (Ground.002)
{-470.145050, -0.059913, -362.366211}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
1, // subtype
},
{13, // object id (Ground.003)
{-1776.515869, 0.359107, -362.366180}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
2, // subtype
},
{14, // object id (Ground.004)
{-884.059082, -4.784880, -1190.968140}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
3, // subtype
},
{15, // object id (Ground.005)
{-1877.005005, -3.859154, -1195.363037}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
4, // subtype
},
{16, // object id (Ground.006)
{-928.530762, -46.834248, -2119.046631}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
5, // subtype
},
{17, // object id (Ground)
{-1880.907227, -71.331970, -2153.255371}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
6, // subtype
},
{18, // object id (Water)
{-1623.523560, -65.354485, -1919.260742}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WaterModel, // modelType
0, // subtype
},
{19, // object id (Rock.inst)
{-1763.404297, -65.354439, -1842.198730}, // position
{0.000000, 172.253710, 0.000000}, // rotation
RockModel, // modelType
0, // subtype
},
{20, // object id (Rock.001.inst)
{-1760.923584, -65.354439, -1763.196533}, // position
{0.000000, 172.253710, 0.000000}, // rotation
RockModel, // modelType
1, // subtype
},
{21, // object id (Rock.002.inst)
{-1812.201416, -62.778927, -1753.173828}, // position
{0.000000, 172.253710, 0.000000}, // rotation
RockModel, // modelType
2, // subtype
},
{22, // object id (Watergrass.002.inst)
{-1830.088745, -65.926628, -1768.197144}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WatergrassModel, // modelType
0, // subtype
},
{23, // object id (Watergrass.003.inst)
{-1828.565674, -72.866142, -1763.804688}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WatergrassModel, // modelType
1, // subtype
},
{24, // object id (Rock.inst.001)
{-1302.227539, -65.354431, -1805.489868}, // position
{0.000000, 172.253710, 0.000000}, // rotation
RockModel, // modelType
0, // subtype
},
{25, // object id (Reed.001.inst.001)
{-1825.325562, -43.459881, -1765.484863}, // position
{0.000000, 0.000000, 0.000000}, // rotation
ReedModel, // modelType
0, // subtype
},
{26, // object id (Rock.001.inst.001)
{-1370.753418, -65.354439, -1783.925171}, // position
{0.000000, 172.253710, 0.000000}, // rotation
RockModel, // modelType
1, // subtype
},
{27, // object id (Watergrass.001.inst.001)
{-1852.447754, -74.177345, -1745.773804}, // position
{0.000000, -9.583076, -0.000000}, // rotation
WatergrassModel, // modelType
2, // subtype
},
{28, // object id (Watergrass.inst.001)
{-1851.673340, -81.396332, -1751.330688}, // position
{0.000000, -9.583076, -0.000000}, // rotation
WatergrassModel, // modelType
3, // subtype
},
{29, // object id (Reed.001.inst.002)
{-1390.599121, -43.459885, -1769.178955}, // position
{0.000000, 0.000000, 0.000000}, // rotation
ReedModel, // modelType
0, // subtype
},
{30, // object id (Lilypad.inst)
{-1584.975220, -62.969212, -1989.747559}, // position
{0.000000, 0.000000, 0.000000}, // rotation
LilypadModel, // modelType
0, // subtype
},
{31, // object id (Lilypad.inst.001)
{-1507.698730, -62.969212, -2013.217163}, // position
{0.000000, 123.995268, 0.000000}, // rotation
LilypadModel, // modelType
0, // subtype
},
{32, // object id (Lilypad.inst.002)
{-1592.382568, -62.969212, -2030.158569}, // position
{0.000000, -40.211082, 0.000000}, // rotation
LilypadModel, // modelType
0, // subtype
},
{33, // object id (Watergrass.001.inst.002)
{-1331.828979, -73.228706, -1764.312256}, // position
{0.000000, -9.583076, -0.000000}, // rotation
WatergrassModel, // modelType
2, // subtype
},
{34, // object id (Watergrass.002.inst.001)
{-1395.362183, -65.926636, -1771.891235}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WatergrassModel, // modelType
0, // subtype
},
{35, // object id (Watergrass.003.inst.001)
{-1393.839233, -72.866150, -1767.498779}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WatergrassModel, // modelType
1, // subtype
},
{36, // object id (Watergrass.inst.002)
{-1331.054565, -80.447685, -1769.869141}, // position
{0.000000, -9.583076, -0.000000}, // rotation
WatergrassModel, // modelType
3, // subtype
},
{37, // object id (Water.001)
{-1623.523560, -65.354485, -1919.260742}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WaterModel, // modelType
1, // subtype
},
{38, // object id (Water.002)
{-1623.523560, -65.354485, -1919.260742}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WaterModel, // modelType
2, // subtype
},
{39, // object id (Water.003)
{-1623.523560, -65.354485, -1919.260742}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WaterModel, // modelType
3, // subtype
},
{40, // object id (Water.004)
{-1623.523560, -65.354485, -1919.260742}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WaterModel, // modelType
4, // subtype
},

};

#define UNIVERSITY_MAP_COUNT 41

#endif /* UNIVERSITY_MAP_H */
